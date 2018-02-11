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

/*
This file handles the main menu
*/

#include "mainmenu.hpp"
#include "../font.hpp"
#include "../ini.hpp"
#include "../lem3edit.hpp"
#include "../window.hpp"

#include "SDL.h"
#include "SDL_ttf.h"

#include <string>

Mainmenu::Mainmenu(Ini * i)
{
	ini_ptr = i;
	redraw = true;

	TTF_Font * titleFont = TTF_OpenFont("./gfx/DejaVuSansMono.ttf", 70);
	titleText = Font::createTextureFromString(titleFont, "Lem3edit");
	TTF_CloseFont(titleFont);

	TTF_Font * buttonFont = TTF_OpenFont("./gfx/DejaVuSansMono.ttf", 30);
	NewLevelText = Font::createTextureFromString(buttonFont, "New Single Level");
	LoadLevelText = Font::createTextureFromString(buttonFont, "Load Single Level");
	CopyLevelText = Font::createTextureFromString(buttonFont, "Copy or Re-number Single Level");
	DeleteLevelText = Font::createTextureFromString(buttonFont, "Delete Single Level");
	NewPackText = Font::createTextureFromString(buttonFont, "New Level Pack");
	LoadPackText = Font::createTextureFromString(buttonFont, "Load Level Pack");
	refreshPreviousPackText();
	OptionsText = Font::createTextureFromString(buttonFont, "Set Testing Paths and Options");
	QuitText = Font::createTextureFromString(buttonFont, "Quit");
	TTF_CloseFont(buttonFont);

	draw();
}

void Mainmenu::refreshPreviousPackText(void)
{
	SDL_DestroyTexture(PreviousPackText);
	PreviousPackText = NULL;
	std::string s = "Load Last Pack: ";
	if (ini_ptr->getLastLoadedPack() == "")
	{
		s += "None!";
	}
	else
	{
		s += ini_ptr->getLastLoadedPack().stem().generic_string();
	}

	TTF_Font * buttonFont = TTF_OpenFont("./gfx/DejaVuSansMono.ttf", 30);
	PreviousPackText = Font::createTextureFromString(buttonFont, s);
	TTF_CloseFont(buttonFont);
}

void Mainmenu::draw(void)
{
	if (!redraw)
		return;

	int centreX = g_window.width / 2;

	SDL_SetRenderDrawBlendMode(g_window.screen_renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(g_window.screen_renderer, g_window.screen_texture);
	SDL_SetRenderDrawColor(g_window.screen_renderer, 240, 240, 240, 255);
	SDL_RenderFillRect(g_window.screen_renderer, NULL);

	//title text
	{
		renderText(titleText, centreX, 4);
	}

	//single level buttons
	{
		renderButton(NewLevelText, centreX, 90);
		renderButton(LoadLevelText, centreX, 140);
		renderButton(CopyLevelText, centreX, 190);
		renderButton(DeleteLevelText, centreX, 240);
	}

	//level pack buttons
	{
		renderButton(NewPackText, centreX, 310);
		renderButton(LoadPackText, centreX, 360);
		renderButton(PreviousPackText, centreX, 410);
	}

	//other buttons
	{
		renderButton(OptionsText, centreX, 480);
		renderButton(QuitText, centreX, 530);
	}

	SDL_SetRenderTarget(g_window.screen_renderer, NULL);
	SDL_RenderCopy(g_window.screen_renderer, g_window.screen_texture, NULL, NULL);
	SDL_RenderPresent(g_window.screen_renderer);

	redraw = false;
}

void Mainmenu::renderText(SDL_Texture * tex, int centreX, int topY)
{
	int textW, textH;
	SDL_Rect textRect;

	SDL_QueryTexture(tex, NULL, NULL, &textW, &textH);
	textRect.x = centreX - (textW / 2);
	textRect.y = topY;
	textRect.w = textW;
	textRect.h = textH;
	SDL_RenderCopy(g_window.screen_renderer, tex, NULL, &textRect);
}

void Mainmenu::renderButton(SDL_Texture * tex, int centreX, int topY)
{
	int textW, textH;
	SDL_Rect buttonRect;

	SDL_QueryTexture(tex, NULL, NULL, &textW, &textH);
	buttonRect.x = centreX - (textW / 2) - 2;
	buttonRect.y = topY;
	buttonRect.w = textW + 4;
	buttonRect.h = textH + 4;

	SDL_SetRenderDrawColor(g_window.screen_renderer, 200, 200, 200, 255);
	SDL_RenderFillRect(g_window.screen_renderer, &buttonRect);
	SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(g_window.screen_renderer, &buttonRect);

	renderText(tex, centreX, topY + 2);
}
