/*
* lem3edit
* Copyright (C) 2008-2009 Carl Reinke
* Copyright (C) 2017-2018 Kieran Millar
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef PACKEDITOR_HPP
#define PACKEDITOR_HPP

#include "../lem3edit.hpp"

#include "SDL.h"

#include <string>
#include <vector>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem::v1;

#define CURRENTPACKFILEVERSION 1

class Ini;
class Editor;

class PackEditor
{
public:
	PackEditor(void);

	void setReferences(Ini * i, Editor * e);

	void handlePackEditorEvents(SDL_Event event);

	bool create(void);
	bool load(const fs::path fileName);
	bool save(void);

	//void createLevel(const int n, const tribeName tribe);
	//void loadLevel(const int n, const tribeName tribe);
	//void editLevel(const int n, const tribeName tribe);

	bool levelExists(const int id);//returns if level files exist and all match expected id

private:
	Ini * ini_ptr;
	Editor * editor_ptr;

	bool redraw = false;

	Uint32 lastFrameTick = 0;

	int version = CURRENTPACKFILEVERSION;

	fs::path packPath;

	tribeName tribeTab = CLASSIC;

	SDL_Texture * numbers[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	SDL_Texture * packTitleTex = NULL;
	SDL_Texture * classicTabTex = NULL;
	SDL_Texture * shadowTabTex = NULL;
	SDL_Texture * egyptTabTex = NULL;
	SDL_Texture * totalLemsTex = NULL;
	SDL_Texture * quitTex = NULL;
	SDL_Texture * addNewLevelTex = NULL;
	SDL_Texture * loadLevelTex = NULL;

	class levelData
	{
	public:
		levelData(std::string s, int n);

		void refreshTexture(void);

		std::string name;
		int lems;
		SDL_Texture * tex = NULL;
	};

	void refreshTitleTexture(void);

	std::vector<levelData> levels[TRIBECOUNT];
	void clearLevels(void);

	int totalLems[TRIBECOUNT] = { 20, 20, 20 };
	void refreshLemCounts(void);

	void draw(void);
	enum renderAlign { LEFT, CENTRE };
	//Renders text, pass LEFT or CENTRE as alignment, pass 0 for no width restriction
	void renderText(SDL_Texture * tex, const int x, const int topY, const renderAlign align, const int restrictWidth);
	void renderNumbers(int num, const int rightX, const int y);
};

#endif // PACKEDITOR_HPP
