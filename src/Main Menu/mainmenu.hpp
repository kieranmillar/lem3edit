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

#include "SDL.h"

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

	enum menuBox {
		NONE, NEWLEVEL, LOADLEVEL, COPYLEVEL, DELETELEVEL,
		NEWPACK, LOADPACK, PREVIOUSPACK, OPTIONS, QUIT
	};
	menuBox highlighting;

	SDL_Texture * titleText;
	SDL_Texture * NewLevelText;
	SDL_Texture * LoadLevelText;
	SDL_Texture * CopyLevelText;
	SDL_Texture * DeleteLevelText;
	SDL_Texture * NewPackText;
	SDL_Texture * LoadPackText;
	SDL_Texture * PreviousPackText;
	SDL_Texture * OptionsText;
	SDL_Texture * QuitText;

	void refreshPreviousPackText(void);

	void renderText(SDL_Texture * tex, const int centreX, const int topY);
	void renderButton(SDL_Texture * tex, const int centreX, const int topY, const bool highlight);
};

#endif // MAINMENU_HPP
