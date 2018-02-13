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

	void handleMainMenuEvents(SDL_Event event);

	void draw(void);

private:
	Ini * ini_ptr;
	Editor * editor_ptr;

	Uint32 lastFrameTick;

	enum menuBox {
		NONE, NEWLEVEL, LOADLEVEL, COPYLEVEL, DELETELEVEL,
		NEWPACK, LOADPACK, PREVIOUSPACK, OPTIONS, QUIT
	};
	menuBox highlighting;

	enum menuMode { NODIALOG, NEWLEVELDIALOG, COPYLEVELDIALOG, OPTIONSDIALOG };
	menuMode menuDialog;

	bool selectedCopy;

	tribeName selectedTribe;

	// main menu text textures
	SDL_Texture * titleText;
	SDL_Texture * loadingText;
	SDL_Texture * NewLevelText;
	SDL_Texture * LoadLevelText;
	SDL_Texture * CopyLevelText;
	SDL_Texture * DeleteLevelText;
	SDL_Texture * NewPackText;
	SDL_Texture * LoadPackText;
	SDL_Texture * PreviousPackText;
	SDL_Texture * OptionsText;
	SDL_Texture * QuitText;

	//dialog text textures
	SDL_Texture * numbers[10];
	SDL_Texture * OKText;
	SDL_Texture * CancelText;
	SDL_Texture * selectTribeText;
	SDL_Texture * classicTribeText;
	SDL_Texture * shadowTribeText;
	SDL_Texture * egyptTribeText;
	SDL_Texture * newLevelIDText;
	SDL_Texture * levelIDClassicText;
	SDL_Texture * levelIDShadowText;
	SDL_Texture * levelIDEgyptText;
	SDL_Texture * levelIDPracticeText;
	SDL_Texture * levelIDDemoText;
	SDL_Texture * copyText;
	SDL_Texture * renumberText;

	void refreshPreviousPackText(void);

	void Mainmenu::typedNumber(const unsigned int value);

	enum renderAlign { LEFT, CENTRE };

	void renderText(SDL_Texture * tex, const int centreX, const int topY, const renderAlign align);
	void renderButton(SDL_Texture * tex, const int centreX, const int topY, const bool highlight);
	void renderNumbers(int num, const int rightX, const int y);

	int level_id;
	struct OBSValues {
		Uint16 perm;
		Uint16 temp;
	};

	OBSValues fileOBS;//check for {1000, 1000} or higher for invalid result
	fs::path filePath;

	//opens a DAT file and returns the values that reference the OBS files;
	OBSValues loadOBSValues(fs::path DATfilepath);
	bool updateOBSValues(fs::path DATfilepath, const int id);

	bool confirmOverwrite(fs::path filePath, int id);

	void newLevelDialog(void);
	void newLevel(void);
	void loadLevel(void);
	void copyLevelDialog(void);
	void copyLevel(void);
	void deleteLevel(void);

	void drawLoadingBanner(void);
};

#endif // MAINMENU_HPP
