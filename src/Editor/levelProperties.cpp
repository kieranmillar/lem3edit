/*
* lem3edit
* Copyright (C) 2008-2009 Carl Reinke
* Copyright (C) 2017 Kieran Millar
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
This file handles the menu inside the editor that lets you edit level properties.
It also includes the event handling / user inputs for this menu.
*/

#include "levelProperties.hpp"

#include "bar.hpp"
#include "canvas.hpp"
#include "editor.hpp"
#include "../font.hpp"
#include "../lem3edit.hpp"
#include "../level.hpp"
#include "../window.hpp"

#include "SDL.h"
#include "SDL_ttf.h"

void LevelProperties::setReferences(Window * w, Editor * e, Bar * b, Canvas * c, Level * l)
{
	window_ptr = w;
	bar_ptr = b;
	editor_ptr = e;
	canvas_ptr = c;
	level_ptr = l;
}

void LevelProperties::openDialog(void)
{
	g_currentMode = LEVELPROPERTIESMODE;
	releaseRate = level_ptr->release_rate;
	spawnDelay = level_ptr->release_delay;
	timeLimitMins = level_ptr->time / 60;
	timeLimitSecs = level_ptr->time % 60;
	redraw = true;
}

void LevelProperties::closeDialog(bool saveChanges)
{
	if (saveChanges)
	{
		level_ptr->release_rate = releaseRate;
		level_ptr->release_delay = spawnDelay;
		level_ptr->time = (timeLimitMins * 60) + timeLimitSecs;
	}
	g_currentMode = EDITORMODE;
	canvas_ptr->redraw = true;
}

void LevelProperties::handleLevelPropertiesEvents(SDL_Event event)
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
				window_ptr->width = e.data1;
				window_ptr->height = e.data2;
				window_ptr->resize();
				editor_ptr->resize(e.data1, e.data2);
				canvas_ptr->redraw = true;
				canvas_ptr->draw();
				bar_ptr->draw(mouse_x_window, mouse_y_window);
				redraw = true;
			}
			break;
		}
		case SDL_MOUSEBUTTONDOWN://when initially pressed
		{
			SDL_MouseButtonEvent &e = event.button;

			if (e.button == SDL_BUTTON_LEFT)
			{

			}
			break;
		}
		case SDL_MOUSEBUTTONUP://when released
		{
			SDL_MouseButtonEvent &e = event.button;

			if (e.button == SDL_BUTTON_LEFT)
			{

			}
			break;
		}
		case SDL_MOUSEWHEEL:
		{
			SDL_MouseWheelEvent &e = event.wheel;
			if (e.y == 1) // wheel up
			{
				
			}
			if (e.y == -1) // wheel down
			{
				
			}
			break;
		}
		case SDL_KEYDOWN:
		{
			SDL_KeyboardEvent &e = event.key;

			switch (e.keysym.sym)
			{
			case SDLK_1:
				
				break;
			case SDLK_2:
				
				break;
			case SDLK_3:
				
				break;
			case SDLK_UP:

				break;
			case SDLK_DOWN:
				
				break;
			case SDLK_LEFT:

				break;
			case SDLK_RIGHT:
				
				break;
			case SDLK_DELETE:
				
				break;
			case SDLK_ESCAPE:
				closeDialog(false);
				break;
			case SDLK_RETURN:
				closeDialog(true);
				break;
			case SDLK_q:
				die();
				break;
			default:
				break;
			}
			break;
		}
		case SDL_USEREVENT:// stuff here happens every frame. Watch out, timer produces events on a separate thread to rest of program!
		{
			draw();
			break;
		}
		default:
		{
			break;
		}
	}
}

void LevelProperties::draw(void)
{
	if (redraw)
	{
		SDL_SetRenderDrawBlendMode(window_ptr->screen_renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderTarget(window_ptr->screen_renderer, window_ptr->screen_texture);

		int dialogX, dialogY;
		dialogX = (window_ptr->width / 2) - 150;
		dialogY = (window_ptr->height / 2) - 100;
		{
			//draw dialog background
			SDL_Rect dialogArea;
			dialogArea.x = dialogX;
			dialogArea.y = dialogY;
			dialogArea.w = 300;
			dialogArea.h = 200;

			SDL_SetRenderDrawColor(window_ptr->screen_renderer, 200, 200, 200, 255);
			SDL_RenderFillRect(window_ptr->screen_renderer, &dialogArea);
			SDL_SetRenderDrawColor(window_ptr->screen_renderer, 0, 0, 0, 255);
			SDL_RenderDrawRect(window_ptr->screen_renderer, &dialogArea);
		}



		{
			//draw all to screen
			SDL_SetRenderTarget(window_ptr->screen_renderer, NULL);
			SDL_RenderCopy(window_ptr->screen_renderer, window_ptr->screen_texture, NULL, NULL);
			SDL_RenderPresent(window_ptr->screen_renderer);
		}
	}
}