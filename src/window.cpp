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
This file handles the initialisation of the program, so setting up the window and SDL, etc.
*/

#include "window.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

bool Window::initialise(int w, int h)
{
	width = w;
	height = h;
	screen = NULL;
	screen_renderer = NULL;
	screen_texture = NULL;

	loop_timer_id = SDL_AddTimer((100.0 / 30.0) * 10.0, loop_timer, NULL);

	//int already_init = screen != NULL || screen_renderer != NULL;
	//preserve palette if display is being reinitialized
	//if (already_init)
	//memcpy(palette_buffer, main_surface->format->palette->colors, sizeof(palette_buffer));

	screen = SDL_CreateWindow("Lem3Edit", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE);
	if (screen == NULL)
	{
		SDL_Log("failed to initialize window: %s", SDL_GetError());
		return false;
	}

	SDL_SetWindowMinimumSize(screen, 400, 300);

	screen_renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/);
	if (screen_renderer == NULL)
	{
		SDL_Log("failed to initialize renderer: %s", SDL_GetError());
		return false;
	}

	screen_texture = SDL_CreateTexture(screen_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, width, height);
	if (screen_texture == NULL)
	{
		SDL_Log("failed to initialize texture: %s", SDL_GetError());
		return false;
	}

	SDL_SetRenderDrawColor(screen_renderer, 255, 255, 255, 255);

	SDL_RenderSetViewport(screen_renderer, NULL);

	//if (already_init)
	//SDL_SetSurfacePalette(main_surface, palette_buffer);

	return true;
}

void Window::destroy(void)
{
	SDL_RemoveTimer(loop_timer_id);

	if (screen_texture != NULL) SDL_DestroyTexture(screen_texture);
	if (screen_renderer != NULL) SDL_DestroyRenderer(screen_renderer);
	if (screen != NULL) SDL_DestroyWindow(screen);
}

bool Window::resize(void)
{
	SDL_RenderSetLogicalSize(screen_renderer, width, height);
	if (screen_texture != NULL) SDL_DestroyTexture(screen_texture);
	screen_texture = SDL_CreateTexture(screen_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, width, height);
	if (screen_texture == NULL)
	{
		SDL_Log("failed to initialize texture: %s", SDL_GetError());
		return false;
	}

	return true;
}

Uint32 loop_timer(Uint32 interval, void *param)
{
	(void)param;

	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = 0;
	event.user.data1 = NULL;
	event.user.data2 = NULL;

	SDL_PushEvent(&event);

	return interval;
}
