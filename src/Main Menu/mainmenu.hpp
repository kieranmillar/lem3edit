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

#ifndef MAINMENU_HPP
#define MAINMENU_HPP

#include "packeditor.hpp"
#include "../lem3edit.hpp"

#include "SDL.h"

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem::v1;

class Ini;
class Editor;

class Mainmenu
{
public:

	Mainmenu(Ini * i, Editor * e);

	PackEditor packEditor;

	void handleMainMenuEvents(SDL_Event event);

	void draw(void);

	struct OBSValues {
		Uint16 perm;
		Uint16 temp;
	};

	static bool confirmOverwrite(fs::path filePath, int id);

	void drawLoadingBanner(void);

	//opens a DAT file and returns the values that reference the OBS files;
	static OBSValues loadOBSValues(fs::path DATfilepath);
	static bool updateOBSValues(fs::path DATfilepath, const int id);

private:
	Ini * ini_ptr;
	Editor * editor_ptr;

	Uint32 lastFrameTick;

	enum menuBox {
		NONE, NEWLEVEL, LOADLEVEL, COPYLEVEL, DELETELEVEL,
		NEWPACK, LOADPACK, PREVIOUSPACK, QUIT
	};
	menuBox highlighting;

	enum menuMode { NODIALOG, NEWLEVELDIALOG, COPYLEVELDIALOG };
	menuMode menuDialog;

	bool selectedCopy;

	tribeName selectedTribe;

	// main menu text textures
	SDL_Texture * titleText = NULL;
	SDL_Texture * loadingText = NULL;
	SDL_Texture * NewLevelText = NULL;
	SDL_Texture * LoadLevelText = NULL;
	SDL_Texture * CopyLevelText = NULL;
	SDL_Texture * DeleteLevelText = NULL;
	SDL_Texture * NewPackText = NULL;
	SDL_Texture * LoadPackText = NULL;
	SDL_Texture * PreviousPackText = NULL;
	SDL_Texture * QuitText = NULL;

	//dialog text textures
	SDL_Texture * numbers[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	SDL_Texture * OKText = NULL;
	SDL_Texture * CancelText = NULL;
	SDL_Texture * selectTribeText = NULL;
	SDL_Texture * classicTribeText = NULL;
	SDL_Texture * shadowTribeText = NULL;
	SDL_Texture * egyptTribeText = NULL;
	SDL_Texture * newLevelIDText = NULL;
	SDL_Texture * levelIDClassicText = NULL;
	SDL_Texture * levelIDShadowText = NULL;
	SDL_Texture * levelIDEgyptText = NULL;
	SDL_Texture * levelIDPracticeText = NULL;
	SDL_Texture * levelIDDemoText = NULL;
	SDL_Texture * copyText = NULL;
	SDL_Texture * renumberText = NULL;

	void refreshPreviousPackText(void);

	void typedNumber(const unsigned int value);

	enum renderAlign { LEFT, CENTRE };

	void renderText(SDL_Texture * tex, const int x, const int topY, const renderAlign align);
	void renderButton(SDL_Texture * tex, const int centreX, const int topY, const bool highlight);
	void renderNumbers(int num, const int rightX, const int y);

	int level_id;
	OBSValues fileOBS;//check for {1000, 1000} or higher for invalid result
	fs::path filePath;

	void newLevelDialog(void);
	void newLevel(void);
	void loadLevel(void);
	void copyLevelDialog(void);
	void copyLevel(void);
	void deleteLevel(void);
};

#endif // MAINMENU_HPP
