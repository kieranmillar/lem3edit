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

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include "SDL.h"

class Window
{
public:

	Uint16 width;
	Uint16 height;

	SDL_Window *screen;
	SDL_Renderer *screen_renderer;
	SDL_Texture *screen_texture;

	SDL_TimerID loop_timer_id;

	SDL_Palette palette_buffer[256];

	bool initialise(int w, int h);
	void destroy(void);
	bool resize(void);
};

Uint32 loop_timer(Uint32 interval, void *param);

#endif // LEVEL_HPP
