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
#include "../Editor/editor.hpp"
#include "../font.hpp"
#include "../ini.hpp"
#include "../lem3edit.hpp"
#include "../tinyfiledialogs.h"
#include "../window.hpp"

#include "SDL.h"
#include "SDL_ttf.h"

#include <fstream>
#include <iostream>
#include <string>

Mainmenu::Mainmenu(Ini * i, Editor * e)
{
	ini_ptr = i;
	editor_ptr = e;

	packEditor.setReferences(ini_ptr, editor_ptr);

	highlighting = NONE;

	lastFrameTick = 0;

	//main menu text textures
	TTF_Font * bigFont = TTF_OpenFont("./gfx/DejaVuSansMono.ttf", 70);
	titleText = Font::createTextureFromString(bigFont, "Lem3edit");
	loadingText = Font::createTextureFromString(bigFont, "LOADING");
	TTF_CloseFont(bigFont);

	//dialog text textures
	TTF_Font * buttonFont = TTF_OpenFont("./gfx/DejaVuSansMono.ttf", 30);
	NewLevelText = Font::createTextureFromString(buttonFont, "New Single Level");
	LoadLevelText = Font::createTextureFromString(buttonFont, "Load Single Level");
	CopyLevelText = Font::createTextureFromString(buttonFont, "Copy or Re-number Single Level");
	DeleteLevelText = Font::createTextureFromString(buttonFont, "Delete Single Level");
	NewPackText = Font::createTextureFromString(buttonFont, "New Level Pack");
	LoadPackText = Font::createTextureFromString(buttonFont, "Load Level Pack");
	PreviousPackText = NULL;
	refreshPreviousPackText();
	OptionsText = Font::createTextureFromString(buttonFont, "Set Testing Paths and Options");
	QuitText = Font::createTextureFromString(buttonFont, "Quit");
	TTF_CloseFont(buttonFont);

	TTF_Font * smallFont = TTF_OpenFont("./gfx/DejaVuSansMono.ttf", 20);
	for (int i = 0; i < 10; i++)
	{
		char digit[2] = "0";
		digit[0] += i;
		numbers[i] = Font::createTextureFromString(smallFont, digit);
	}
	OKText = Font::createTextureFromString(smallFont, "OK");
	CancelText = Font::createTextureFromString(smallFont, "Cancel");
	selectTribeText = Font::createTextureFromString(smallFont, "Select Tribe:");
	classicTribeText = Font::createTextureFromString(smallFont, "CLASSIC");
	shadowTribeText = Font::createTextureFromString(smallFont, "SHADOW");
	egyptTribeText = Font::createTextureFromString(smallFont, "EGYPT");
	newLevelIDText = Font::createTextureFromString(smallFont, "Level ID: ");
	levelIDClassicText = Font::createTextureFromString(smallFont, "001 - 030: Classic Levels");
	levelIDShadowText = Font::createTextureFromString(smallFont, "101 - 130: Shadow Levels");
	levelIDEgyptText = Font::createTextureFromString(smallFont, "201 - 230: Egypt Levels");
	levelIDPracticeText = Font::createTextureFromString(smallFont, "990      : Practice Level");
	levelIDDemoText = Font::createTextureFromString(smallFont, "995 - 999: Demo Levels");
	copyText = Font::createTextureFromString(smallFont, "Make a copy");
	renumberText = Font::createTextureFromString(smallFont, "Re-number (deletes original files)");
	TTF_CloseFont(smallFont);

	menuDialog = NODIALOG;

	level_id = 1;
	fileOBS = { 1000, 1000 };

	draw();
}

void Mainmenu::refreshPreviousPackText(void)
{
	if (PreviousPackText != NULL)
		SDL_DestroyTexture(PreviousPackText);
	PreviousPackText = NULL;

	std::string s = "Load Last Pack: ";
	if (fs::exists(ini_ptr->lastLoadedPack))
	{
		s += ini_ptr->lastLoadedPack.stem().generic_string();
	}
	else
	{
		s += "None!";
	}

	TTF_Font * buttonFont = TTF_OpenFont("./gfx/DejaVuSansMono.ttf", 30);
	PreviousPackText = Font::createTextureFromString(buttonFont, s);
	TTF_CloseFont(buttonFont);
}

void Mainmenu::handleMainMenuEvents(SDL_Event event)
{
	Sint32 mouse_x_window, mouse_y_window;
	Uint8 mouse_state = SDL_GetMouseState(&mouse_x_window, &mouse_y_window);

	switch (event.type)
	{
	case SDL_WINDOWEVENT:
	{
		SDL_WindowEvent &e = event.window;

		if (e.event == SDL_WINDOWEVENT_RESIZED)
		{
			g_window.resize(e.data1, e.data2);
		}
		break;
	}
	case SDL_MOUSEMOTION:
	{
		SDL_MouseMotionEvent &e = event.motion;

		if (menuDialog == NODIALOG)
		{
			highlighting = NONE;

			if (mouse_y_window > 90
				&& mouse_y_window < 130)
			{
				highlighting = NEWLEVEL;
			}
			if (mouse_y_window > 140
				&& mouse_y_window < 180)
			{
				highlighting = LOADLEVEL;
			}
			if (mouse_y_window > 190
				&& mouse_y_window < 230)
			{
				highlighting = COPYLEVEL;
			}
			if (mouse_y_window > 240
				&& mouse_y_window < 270)
			{
				highlighting = DELETELEVEL;
			}
			if (mouse_y_window > 310
				&& mouse_y_window < 350)
			{
				highlighting = NEWPACK;
			}
			if (mouse_y_window > 360
				&& mouse_y_window < 400)
			{
				//highlighting = LOADPACK;
			}
			if (mouse_y_window > 410
				&& mouse_y_window < 450)
			{
				if (fs::exists(ini_ptr->lastLoadedPack))
					highlighting = PREVIOUSPACK;
			}
			if (mouse_y_window > 480
				&& mouse_y_window < 520)
			{
				//highlighting = OPTIONS;
			}
			if (mouse_y_window > 530
				&& mouse_y_window < 570)
			{
				highlighting = QUIT;
			}
		}
		break;
	}
	case SDL_MOUSEBUTTONDOWN://when pressed
	{
		SDL_MouseButtonEvent &e = event.button;

		if (e.button == SDL_BUTTON_LEFT)
		{
			int centreX = g_window.width / 2;

			switch (menuDialog)
			{
			case NODIALOG:
			{
				switch (highlighting)
				{
				case NEWLEVEL:
					newLevelDialog();
					break;

				case LOADLEVEL:
				{
					loadLevel();
				}
				break;

				case COPYLEVEL:
				{
					copyLevelDialog();
				}
				break;

				case DELETELEVEL:
				{
					deleteLevel();
				}
				break;

				case NEWPACK:
					if (packEditor.create())
					{
						highlighting = NONE;
						refreshPreviousPackText();
					}
					break;

				case LOADPACK:
					//todo
					break;

				case PREVIOUSPACK:
					if (fs::exists(ini_ptr->lastLoadedPack))
					{
						if (packEditor.load(ini_ptr->lastLoadedPack))
						{
							highlighting = NONE;
						}
					}
					break;

				case OPTIONS:
					//todo
					break;

				case QUIT:
					die();
					break;

				default:

					break;
				}
			}
			break;

			case NEWLEVELDIALOG:
			{
				if (mouse_x_window > (centreX - 190)
					&& mouse_x_window < (centreX - 170)
					&& mouse_y_window > 205
					&& mouse_y_window < 225)
				{
					selectedTribe = CLASSIC;
				}
				if (mouse_x_window > (centreX - 190)
					&& mouse_x_window < (centreX - 170)
					&& mouse_y_window > 235
					&& mouse_y_window < 255)
				{
					selectedTribe = SHADOW;
				}
				if (mouse_x_window > (centreX - 190)
					&& mouse_x_window < (centreX - 170)
					&& mouse_y_window > 265
					&& mouse_y_window < 285)
				{
					selectedTribe = EGYPT;
				}
				if (mouse_x_window > (centreX - 150)
					&& mouse_x_window < (centreX - 50)
					&& mouse_y_window > 425
					&& mouse_y_window < 465)
				{
					newLevel();
				}
				if (mouse_x_window > (centreX + 50)
					&& mouse_x_window < (centreX + 150)
					&& mouse_y_window > 425
					&& mouse_y_window < 465)
				{
					menuDialog = NODIALOG;
				}
			}
			break;

			case COPYLEVELDIALOG:
			{
				if (mouse_x_window > (centreX - 290)
					&& mouse_x_window < (centreX - 270)
					&& mouse_y_window > 200
					&& mouse_y_window < 220)
				{
					selectedCopy = true;
				}
				if (mouse_x_window > (centreX - 290)
					&& mouse_x_window < (centreX - 270)
					&& mouse_y_window > 230
					&& mouse_y_window < 250)
				{
					selectedCopy = false;
				}
				if (mouse_x_window > (centreX - 150)
					&& mouse_x_window < (centreX - 50)
					&& mouse_y_window > 400
					&& mouse_y_window < 440)
				{
					copyLevel();
				}
				if (mouse_x_window > (centreX + 50)
					&& mouse_x_window < (centreX + 150)
					&& mouse_y_window > 400
					&& mouse_y_window < 440)
				{
					menuDialog = NODIALOG;
				}
			}
			break;
			}
		}
		break;
	}
	case SDL_KEYDOWN:
	{
		SDL_KeyboardEvent &e = event.key;

		switch (e.keysym.sym)
		{
		case SDLK_0:
		case SDLK_KP_0:
			typedNumber(0);
			break;
		case SDLK_1:
		case SDLK_KP_1:
			typedNumber(1);
			break;
		case SDLK_2:
		case SDLK_KP_2:
			typedNumber(2);
			break;
		case SDLK_3:
		case SDLK_KP_3:
			typedNumber(3);
			break;
		case SDLK_4:
		case SDLK_KP_4:
			typedNumber(4);
			break;
		case SDLK_5:
		case SDLK_KP_5:
			typedNumber(5);
			break;
		case SDLK_6:
		case SDLK_KP_6:
			typedNumber(6);
			break;
		case SDLK_7:
		case SDLK_KP_7:
			typedNumber(7);
			break;
		case SDLK_8:
		case SDLK_KP_8:
			typedNumber(8);
			break;
		case SDLK_9:
		case SDLK_KP_9:
			typedNumber(9);
			break;
		case SDLK_BACKSPACE:
		case SDLK_DELETE:
			level_id /= 10;
			if (level_id < 0)
				level_id = 0;
			break;
		case SDLK_ESCAPE:
			menuDialog = NODIALOG;
			break;
		case SDLK_RETURN:
			if (menuDialog == NEWLEVELDIALOG)
				newLevel();
			if (menuDialog == COPYLEVELDIALOG)
				copyLevel();
			break;
		default:
			break;
		}
		break;
	}
	case SDL_USEREVENT:// stuff here happens every frame. Watch out, timer produces events on a separate thread to rest of program!
	{
		Uint32 ticksSinceLastFrame = SDL_GetTicks() - lastFrameTick;
		lastFrameTick = SDL_GetTicks();
		if (ticksSinceLastFrame <= 36 && ticksSinceLastFrame >= 30)
		{
			draw();
		}
		break;
	}
	default:
	{
		break;
	}
	}
}

void Mainmenu::typedNumber(const unsigned int value)
{
	if (level_id < 100)
	{
		level_id *= 10;
		level_id += value;
	}
}

void Mainmenu::draw(void)
{
	int centreX = g_window.width / 2;

	SDL_SetRenderDrawBlendMode(g_window.screen_renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(g_window.screen_renderer, g_window.screen_texture);
	SDL_SetRenderDrawColor(g_window.screen_renderer, 240, 240, 240, 255);
	SDL_RenderFillRect(g_window.screen_renderer, NULL);

	//title text
	{
		renderText(titleText, centreX, 4, CENTRE);
	}

	//single level buttons
	{
		renderButton(NewLevelText, centreX, 90, highlighting == NEWLEVEL);
		renderButton(LoadLevelText, centreX, 140, highlighting == LOADLEVEL);
		renderButton(CopyLevelText, centreX, 190, highlighting == COPYLEVEL);
		renderButton(DeleteLevelText, centreX, 240, highlighting == DELETELEVEL);
	}

	//level pack buttons
	{
		renderButton(NewPackText, centreX, 310, highlighting == NEWPACK);
		renderButton(LoadPackText, centreX, 360, highlighting == LOADPACK);
		renderButton(PreviousPackText, centreX, 410, highlighting == PREVIOUSPACK);
	}

	//other buttons
	{
		renderButton(OptionsText, centreX, 480, highlighting == OPTIONS);
		renderButton(QuitText, centreX, 530, highlighting == QUIT);
	}

	//draw strikes through all unimplemented features
	{
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawLine(g_window.screen_renderer, centreX - 300, 380, centreX + 300, 380);//load pack
		SDL_RenderDrawLine(g_window.screen_renderer, centreX - 300, 500, centreX + 300, 500);//options
	}

	switch (menuDialog)
	{
	case NEWLEVELDIALOG:
	{
		SDL_Rect r;
		r.x = centreX - 200;
		r.y = 125;
		r.w = 400;
		r.h = 350;
		SDL_SetRenderDrawColor(g_window.screen_renderer, 220, 220, 220, 255);
		SDL_RenderFillRect(g_window.screen_renderer, &r);
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(g_window.screen_renderer, &r);

		renderText(NewLevelText, centreX, 129, CENTRE);

		renderText(selectTribeText, centreX - 190, 175, LEFT);

		r.x = centreX - 190;
		r.y = 205;
		r.w = 20;
		r.h = 20;
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		if (selectedTribe == CLASSIC)
		{
			SDL_RenderDrawLine(g_window.screen_renderer, centreX - 190, 205, centreX - 171, 224);
			SDL_RenderDrawLine(g_window.screen_renderer, centreX - 171, 205, centreX - 190, 224);
		}
		renderText(classicTribeText, centreX - 160, 205, LEFT);

		r.x = centreX - 190;
		r.y = 235;
		r.w = 20;
		r.h = 20;
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		if (selectedTribe == SHADOW)
		{
			SDL_RenderDrawLine(g_window.screen_renderer, centreX - 190, 235, centreX - 171, 254);
			SDL_RenderDrawLine(g_window.screen_renderer, centreX - 171, 235, centreX - 190, 254);
		}
		renderText(shadowTribeText, centreX - 160, 235, LEFT);

		r.x = centreX - 190;
		r.y = 265;
		r.w = 20;
		r.h = 20;
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		if (selectedTribe == EGYPT)
		{
			SDL_RenderDrawLine(g_window.screen_renderer, centreX - 190, 265, centreX - 171, 284);
			SDL_RenderDrawLine(g_window.screen_renderer, centreX - 171, 265, centreX - 190, 284);
		}
		renderText(egyptTribeText, centreX - 160, 265, LEFT);

		renderText(newLevelIDText, centreX - 190, 295, LEFT);
		r.x = centreX - 80;
		r.y = 293;
		r.w = 40;
		r.h = 24;
		SDL_SetRenderDrawColor(g_window.screen_renderer, 240, 240, 240, 255);
		SDL_RenderFillRect(g_window.screen_renderer, &r);
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		renderNumbers(level_id, centreX - 42, 295);

		renderText(levelIDClassicText, centreX - 190, 315, LEFT);
		renderText(levelIDShadowText, centreX - 190, 335, LEFT);
		renderText(levelIDEgyptText, centreX - 190, 355, LEFT);
		renderText(levelIDPracticeText, centreX - 190, 375, LEFT);
		renderText(levelIDDemoText, centreX - 190, 395, LEFT);

		r.x = centreX - 150;
		r.y = 425;
		r.w = 100;
		r.h = 40;
		SDL_SetRenderDrawColor(g_window.screen_renderer, 200, 200, 200, 255);
		SDL_RenderFillRect(g_window.screen_renderer, &r);
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		renderText(OKText, centreX - 100, 435, CENTRE);

		r.x = centreX + 50;
		r.y = 425;
		r.w = 100;
		r.h = 40;
		SDL_SetRenderDrawColor(g_window.screen_renderer, 200, 200, 200, 255);
		SDL_RenderFillRect(g_window.screen_renderer, &r);
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		renderText(CancelText, centreX + 100, 435, CENTRE);
	}
	break;
	case COPYLEVELDIALOG:
	{
		SDL_Rect r;
		r.x = centreX - 300;
		r.y = 150;
		r.w = 600;
		r.h = 300;
		SDL_SetRenderDrawColor(g_window.screen_renderer, 220, 220, 220, 255);
		SDL_RenderFillRect(g_window.screen_renderer, &r);
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(g_window.screen_renderer, &r);

		renderText(CopyLevelText, centreX, 154, CENTRE);

		r.x = centreX - 290;
		r.y = 200;
		r.w = 20;
		r.h = 20;
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		if (selectedCopy)
		{
			SDL_RenderDrawLine(g_window.screen_renderer, centreX - 290, 200, centreX - 271, 219);
			SDL_RenderDrawLine(g_window.screen_renderer, centreX - 271, 200, centreX - 290, 219);
		}
		renderText(copyText, centreX - 260, 200, LEFT);

		r.x = centreX - 290;
		r.y = 230;
		r.w = 20;
		r.h = 20;
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		if (!selectedCopy)
		{
			SDL_RenderDrawLine(g_window.screen_renderer, centreX - 290, 230, centreX - 271, 249);
			SDL_RenderDrawLine(g_window.screen_renderer, centreX - 271, 230, centreX - 290, 249);
		}
		renderText(renumberText, centreX - 260, 230, LEFT);

		renderText(newLevelIDText, centreX - 290, 260, LEFT);
		r.x = centreX - 180;
		r.y = 258;
		r.w = 40;
		r.h = 24;
		SDL_SetRenderDrawColor(g_window.screen_renderer, 240, 240, 240, 255);
		SDL_RenderFillRect(g_window.screen_renderer, &r);
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		renderNumbers(level_id, centreX - 142, 260);

		renderText(levelIDClassicText, centreX - 290, 280, LEFT);
		renderText(levelIDShadowText, centreX - 290, 300, LEFT);
		renderText(levelIDEgyptText, centreX - 290, 320, LEFT);
		renderText(levelIDPracticeText, centreX - 290, 340, LEFT);
		renderText(levelIDDemoText, centreX - 290, 360, LEFT);

		r.x = centreX - 150;
		r.y = 400;
		r.w = 100;
		r.h = 40;
		SDL_SetRenderDrawColor(g_window.screen_renderer, 200, 200, 200, 255);
		SDL_RenderFillRect(g_window.screen_renderer, &r);
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		renderText(OKText, centreX - 100, 410, CENTRE);

		r.x = centreX + 50;
		r.y = 400;
		r.w = 100;
		r.h = 40;
		SDL_SetRenderDrawColor(g_window.screen_renderer, 200, 200, 200, 255);
		SDL_RenderFillRect(g_window.screen_renderer, &r);
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		renderText(CancelText, centreX + 100, 410, CENTRE);
	}
	break;
	}

	SDL_SetRenderTarget(g_window.screen_renderer, NULL);
	SDL_RenderCopy(g_window.screen_renderer, g_window.screen_texture, NULL, NULL);
	SDL_RenderPresent(g_window.screen_renderer);
}

void Mainmenu::renderText(SDL_Texture * tex, const int x, const int topY, const renderAlign align)
{
	int textW, textH;
	SDL_Rect textRect;

	SDL_QueryTexture(tex, NULL, NULL, &textW, &textH);
	if (align == CENTRE)
		textRect.x = x - (textW / 2);
	else
		textRect.x = x;
	textRect.y = topY;
	textRect.w = textW;
	textRect.h = textH;
	SDL_RenderCopy(g_window.screen_renderer, tex, NULL, &textRect);
}

void Mainmenu::renderButton(SDL_Texture * tex, const int centreX, const int topY, const bool highlight)
{
	int textW, textH;
	SDL_Rect buttonRect;

	SDL_QueryTexture(tex, NULL, NULL, &textW, &textH);
	buttonRect.x = centreX - (textW / 2) - 2;
	buttonRect.y = topY;
	buttonRect.w = textW + 4;
	buttonRect.h = textH + 4;

	if (highlight)
		SDL_SetRenderDrawColor(g_window.screen_renderer, 255, 255, 255, 255);
	else
		SDL_SetRenderDrawColor(g_window.screen_renderer, 200, 200, 200, 255);
	SDL_RenderFillRect(g_window.screen_renderer, &buttonRect);
	SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(g_window.screen_renderer, &buttonRect);

	renderText(tex, centreX, topY + 2, CENTRE);
}

void Mainmenu::renderNumbers(int num, const int rightX, const int y)
{
	int numChars[3];
	int size = 0;
	if (num == 0)
	{
		size = 1;
		numChars[0] = 0;
	}
	else
	{
		while (num != 0)
		{
			//load number characters into array in reverse order
			numChars[size] = num % 10;
			num /= 10;
			size++;
		}
	}

	int textW, textH, drawX;
	SDL_Rect textRect;
	drawX = rightX;
	for (int i = 0; i < size; i++)
	{
		SDL_QueryTexture(numbers[numChars[i]], NULL, NULL, &textW, &textH);
		drawX -= textW;
		textRect.x = drawX;
		textRect.y = y;
		textRect.w = textW;
		textRect.h = textH;
		SDL_RenderCopy(g_window.screen_renderer, numbers[numChars[i]], NULL, &textRect);
	}
}

bool Mainmenu::confirmOverwrite(fs::path filePath, int id)
{
	fs::path levelPath = filePath;
	bool levelOverwrite = fs::exists(levelPath);

	fs::path tempPath = filePath.parent_path();
	tempPath /= "TEMP";
	tempPath += l3_filename_number(id);
	tempPath += ".OBS";
	bool tempOverwrite = fs::exists(tempPath);

	fs::path permPath = filePath.parent_path();
	permPath /= "PERM";
	permPath += l3_filename_number(id);
	permPath += ".OBS";
	bool permOverwrite = fs::exists(permPath);

	if (levelOverwrite || tempOverwrite || permOverwrite)
	{
		std::string message = "You are about to overwrite the following files:\n";
		if (levelOverwrite)
		{
			message += levelPath.filename().generic_string();
			message += "\n";
		}
		if (tempOverwrite)
		{
			message += "TEMP";
			message += l3_filename_number(id);
			message += ".OBS\n";
		}
		if (permOverwrite)
		{
			message += "PERM";
			message += l3_filename_number(id);
			message += ".OBS\n";
		}
		message += "Are you sure you want to do this?";
		if (tinyfd_messageBox(
			"Overwrite?",
			message.c_str(),
			"okcancel",
			"question",
			0
		) == 0)
			return false;
		if (levelOverwrite)
			fs::remove(levelPath);
		if (tempOverwrite)
			fs::remove(tempPath);
		if (permOverwrite)
			fs::remove(permPath);
	}
	return true;
}

void Mainmenu::newLevelDialog(void)
{
	level_id = 1;
	highlighting = NONE;
	menuDialog = NEWLEVELDIALOG;
	selectedTribe = CLASSIC;
}

void Mainmenu::newLevel(void)
{
	char const * fileToSaveTo = NULL;
	char const * filterPatterns[1] = { "*.DAT" };
	fileToSaveTo = tinyfd_saveFileDialog("Save New Level", NULL, 1, filterPatterns, "Lemmings 3 Level File (*.DAT)");

	if (!fileToSaveTo)
		return;

	fs::path destinationPath = fileToSaveTo;
	if (!destinationPath.has_extension())
		destinationPath += ".DAT";
	if (!confirmOverwrite(destinationPath, level_id))
		return;

	//create blank TEMP###.OBS file
	fs::path tempPath = destinationPath.parent_path();
	tempPath /= "TEMP";
	tempPath += l3_filename_number(level_id);
	tempPath += ".OBS";

	{
		std::ofstream f(tempPath, std::ios_base::binary | std::ios_base::in | std::ios_base::trunc);
		if (!f)
		{
			SDL_Log("Failed to open '%s'\n", tempPath.generic_string().c_str());
			return;
		}
		f.close();
	}

	//create blank PERM###.OBS file
	fs::path permPath = destinationPath.parent_path();
	permPath /= "PERM";
	permPath += l3_filename_number(level_id);
	permPath += ".OBS";

	{
		std::ofstream f(permPath, std::ios_base::binary | std::ios_base::in | std::ios_base::trunc);
		if (!f)
		{
			SDL_Log("Failed to open '%s'\n", permPath.generic_string().c_str());
			return;
		}
		f.close();
	}

	drawLoadingBanner();
	highlighting = NONE;
	menuDialog = NODIALOG;
	g_currentMode = EDITORMODE;
	editor_ptr->create(destinationPath, selectedTribe, level_id);
}

void Mainmenu::loadLevel(void)
{
	char const * fileToOpen = NULL;
	char const * filterPatterns[1] = { "*.DAT" };
	fileToOpen = tinyfd_openFileDialog("Open level", NULL, 1, filterPatterns, "Lemmings 3 Level File (*.DAT)", 0);
	if (!fileToOpen)
		return;

	drawLoadingBanner();
	highlighting = NONE;
	g_currentMode = EDITORMODE;
	editor_ptr->load(fileToOpen);
}

void Mainmenu::copyLevelDialog(void)
{
	char const * fileToCopy = NULL;
	char const * filterPatterns[1] = { "*.DAT" };
	fileToCopy = tinyfd_openFileDialog("Select level to Copy or Renumber", NULL, 1, filterPatterns, "Lemmings 3 Level File (*.DAT)", 0);

	if (!fileToCopy)
		return;

	filePath = fileToCopy;
	if (!fs::exists(filePath))
		return;

	fileOBS = loadOBSValues(filePath);
	if (fileOBS.temp >= 1000 && fileOBS.perm >= 1000)
		return;

	std::string levelNum = filePath.stem().generic_string();
	levelNum = levelNum.substr(5, 8);
	level_id = atoi(levelNum.c_str());

	highlighting = NONE;
	menuDialog = COPYLEVELDIALOG;
	selectedCopy = true;
}

void Mainmenu::copyLevel(void)
{
	char const * fileToCopyTo = NULL;
	char const * filterPatterns[1] = { "*.DAT" };
	fs::path defaultPath = filePath.parent_path();
	defaultPath /= "LEVEL";
	defaultPath += l3_filename_number(level_id);
	defaultPath += ".DAT";
	std::string temporary = defaultPath.generic_string();
	char const * defaultPath_c = temporary.c_str();
	fileToCopyTo = tinyfd_saveFileDialog("Save New Copy", defaultPath_c, 1, filterPatterns, "Lemmings 3 Level File (*.DAT)");

	if (!fileToCopyTo)
		return;

	fs::path destinationPath = fileToCopyTo;
	if (!destinationPath.has_extension())
		destinationPath += ".DAT";

	//Check that we're not copying or renumbering over the same file
	if (fs::equivalent(filePath, destinationPath) || fileOBS.temp == level_id || fileOBS.perm == level_id)
	{
		tinyfd_messageBox("Oh No!", "One or more of the files are trying to copy over itself!\n\nPlease choose a different level ID.", "ok", "error", 1);
		return;
	}

	if (!confirmOverwrite(destinationPath, level_id))
		return;

	fs::path sourcePath = filePath.parent_path();
	fs::path fromPath;
	fs::path toPath;

	fromPath = filePath;
	toPath = destinationPath;
	fs::copy(fromPath, toPath);
	if (!updateOBSValues(toPath, level_id))
	{
		tinyfd_messageBox("Oh No!", "Lem3edit could not update the temp and perm file references in the new level file for some reason!\n\nLevel copying aborted.", "ok", "error", 1);
		return;
	}

	if (!selectedCopy)
		fs::remove(fromPath);

	fromPath = sourcePath;
	fromPath /= "TEMP";
	fromPath += l3_filename_number(fileOBS.temp);
	fromPath += ".OBS";
	toPath = destinationPath.parent_path();
	toPath /= "TEMP";
	toPath += l3_filename_number(level_id);
	toPath += ".OBS";
	fs::copy(fromPath, toPath);
	if (!selectedCopy)
		fs::remove(fromPath);

	fromPath = sourcePath;
	fromPath /= "PERM";
	fromPath += l3_filename_number(fileOBS.perm);
	fromPath += ".OBS";
	toPath = destinationPath.parent_path();
	toPath /= "PERM";
	toPath += l3_filename_number(level_id);
	toPath += ".OBS";
	fs::copy(fromPath, toPath);
	if (!selectedCopy)
		fs::remove(fromPath);

	menuDialog = NODIALOG;
}

void Mainmenu::deleteLevel(void)
{
	char const * fileToDelete = NULL;
	char const * filterPatterns[1] = { "*.DAT" };
	fileToDelete = tinyfd_openFileDialog("Select level to Delete", NULL, 1, filterPatterns, "Lemmings 3 Level File (*.DAT)", 0);

	if (!fileToDelete)
		return;

	filePath = fileToDelete;
	if (!fs::exists(filePath))
		return;

	fileOBS = loadOBSValues(filePath);
	if (fileOBS.temp >= 1000 || fileOBS.perm >= 1000)
		return;

	std::string title = "Deleting ";
	title += filePath.stem().generic_string();

	std::string message = "You will also delete the following files:\nTEMP";
	message += l3_filename_number(fileOBS.temp);
	message += ".OBS\nPERM";
	message += l3_filename_number(fileOBS.perm);
	message += ".OBS\nAre you sure you want to do this?";

	if (tinyfd_messageBox(
		title.c_str(),
		message.c_str(),
		"okcancel",
		"warning",
		0
	) == 0)
		return;

	bool success = true;

	if (!fs::remove(filePath))
		success = false;

	fs::path tempPath = filePath.parent_path();
	tempPath /= "TEMP";
	tempPath += l3_filename_number(fileOBS.temp);
	tempPath += ".OBS";
	if (!fs::remove(tempPath))
		success = false;

	fs::path permPath = filePath.parent_path();
	permPath /= "PERM";
	permPath += l3_filename_number(fileOBS.perm);
	permPath += ".OBS";
	if (!fs::remove(permPath))
		success = false;

	if (success)
		tinyfd_messageBox("Level Deleted", "Level deleted!", "ok", "info", 1);
	else
		tinyfd_messageBox("Oh No!", "Lem3edit could not delete one or more of these files!", "ok", "error", 1);
}

//draw loading banner to provide feedback that something is happening
void Mainmenu::drawLoadingBanner(void)
{
	SDL_SetRenderTarget(g_window.screen_renderer, g_window.screen_texture);
	SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
	SDL_RenderClear(g_window.screen_renderer);
	renderButton(loadingText, g_window.width / 2, 260, true);
	SDL_SetRenderTarget(g_window.screen_renderer, NULL);
	SDL_RenderCopy(g_window.screen_renderer, g_window.screen_texture, NULL, NULL);
	SDL_RenderPresent(g_window.screen_renderer);
}

Mainmenu::OBSValues Mainmenu::loadOBSValues(fs::path DATfilepath)
{
	std::ifstream f(DATfilepath, std::ios::binary);
	if (!f)
	{
		SDL_Log("loadOBSValues: Failed to open '%s'\n", DATfilepath.generic_string().c_str());
		return { 1000 , 1000 };
	}

	Uint16 temp;
	Uint16 perm;

	f.seekg(6);
	f.read((char *)&temp, sizeof(temp));
	f.read((char *)&perm, sizeof(perm));
	f.close();

	return { temp , perm };
}

bool Mainmenu::updateOBSValues(fs::path DATfilepath, const int id)
{
	std::fstream f(DATfilepath, std::ios_base::binary | std::ios_base::in | std::ios_base::out);
	if (!f)
	{
		SDL_Log("updateOBSValues: Failed to open '%s'\n", DATfilepath.generic_string().c_str());
		return false;
	}

	Uint16 fileStats[15];

	for (int i = 0; i < 15; i++)
	{
		f.read((char *)&fileStats[i], sizeof(fileStats[i]));
	}

	fileStats[3] = fileStats[4] = id;

	f.clear();
	f.seekg(0, std::ios::beg);

	for (int i = 0; i < 15; i++)
	{
		f.write((char *)&fileStats[i], sizeof(fileStats[i]));
	}

	f.close();
	return true;
}
