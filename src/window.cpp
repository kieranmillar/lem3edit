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

#define DEFAULT_WINDOW_WIDTH 640
#define DEFAULT_WINDOW_HEIGHT 480

#include "window.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

bool Window::initialise( void )
{
	screen = NULL;
	screen_renderer = NULL;
	screen_texture = NULL;

	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO))
	{
		cerr << "failed to initialize SDL: " << SDL_GetError() << endl;
		return false;
	}

	loop_timer_id = SDL_AddTimer((100.0 / 30.0) * 10.0, loop_timer, NULL);

	//int already_init = screen != NULL || screen_renderer != NULL;
	//preserve palette if display is being reinitialized
	//if (already_init)
		//memcpy(palette_buffer, main_surface->format->palette->colors, sizeof(palette_buffer));

	screen = SDL_CreateWindow("Lem3Edit", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, /*SDL_WINDOW_RESIZABLE*/0);
	if (screen == NULL)
	{
		cerr << "failed to initialize window: " << SDL_GetError() << endl;
		return false;
	}

	screen_renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (screen_renderer == NULL)
	{
		cerr << "failed to initialize renderer: " << SDL_GetError() << endl;
		return false;
	}

	screen_texture = SDL_CreateTexture(screen_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 640, 480);
	if (screen_texture == NULL)
	{
		cerr << "failed to initialize texture: " << SDL_GetError() << endl;
		return false;
	}

	SDL_SetRenderDrawColor(screen_renderer, 255, 255, 255, 255);

	//if (already_init)
		//SDL_SetSurfacePalette(main_surface, palette_buffer);

	//SDL_RenderSetLogicalSize(screen_renderer, 640, 480);

	return true;
}

void Window::destroy(void)
{

	SDL_RemoveTimer(loop_timer_id);


	if (screen_texture != NULL) SDL_DestroyTexture(screen_texture);
	if (screen_renderer != NULL) SDL_DestroyRenderer(screen_renderer);
	if (screen != NULL) SDL_DestroyWindow(screen);

	SDL_Quit();
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