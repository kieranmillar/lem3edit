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
This file includes code to deal eith the object bar at the bottom of the editor screen
TODO: All this stuff.
*/

#define BAR_HEIGHT 150

#include "bar.hpp"
#include "editor.hpp"
#include "../style.hpp"
#include "../window.hpp"

#include <cassert>
#include <stdlib.h>

void Bar::setReferences(Window * w, Editor * e, Style * s)
{
	window_ptr = w;
	editor_ptr = e;
	style_ptr = s;
}

void Bar::draw( void )
{
	if (redraw)
	{
		{ // bar background in the absense of a proper graphic
			SDL_SetRenderDrawBlendMode(window_ptr->screen_renderer, SDL_BLENDMODE_BLEND);
			SDL_SetRenderTarget(window_ptr->screen_renderer, window_ptr->screen_texture);

			SDL_SetRenderDrawColor(window_ptr->screen_renderer, 130, 130, 130, 255);
			SDL_Rect rOptionsBox;
			rOptionsBox.x = 0;
			rOptionsBox.y = window_ptr->height - BAR_HEIGHT;
			rOptionsBox.w = BAR_HEIGHT;
			rOptionsBox.h = window_ptr->height;
			SDL_RenderFillRect(window_ptr->screen_renderer, &rOptionsBox);

			SDL_SetRenderDrawColor(window_ptr->screen_renderer, 150, 150, 150, 255);
			SDL_Rect rPieceSelector;
			rPieceSelector.x = BAR_HEIGHT;
			rPieceSelector.y = window_ptr->height - BAR_HEIGHT;
			rPieceSelector.w = window_ptr->width - BAR_HEIGHT;
			rPieceSelector.h = window_ptr->height;
			SDL_RenderFillRect(window_ptr->screen_renderer, &rPieceSelector);

			SDL_SetRenderDrawColor(window_ptr->screen_renderer, 0, 0, 0, 255);
			SDL_RenderDrawLine(window_ptr->screen_renderer, 0, window_ptr->height - BAR_HEIGHT, window_ptr->width, window_ptr->height - BAR_HEIGHT);
			SDL_RenderDrawLine(window_ptr->screen_renderer, BAR_HEIGHT, window_ptr->height - BAR_HEIGHT, BAR_HEIGHT, window_ptr->height);
		}

		{
			int x = BAR_HEIGHT;
			int type = 0;
			assert((unsigned)type < COUNTOF(style_ptr->object));

			for (std::vector<Style::Object>::iterator i = style_ptr->object[type].begin(); i != style_ptr->object[type].end(); ++i)
			{
				SDL_Rect rPieceBox;
				rPieceBox.x = x + 2;
				rPieceBox.y = window_ptr->height - BAR_HEIGHT + 2;
				rPieceBox.w = 131;
				rPieceBox.h = 131;
				SDL_RenderDrawRect(window_ptr->screen_renderer, &rPieceBox);

				int pieceZoom = 1;
				int pieceVectorPosition = i - style_ptr->object[type].begin();
				int pieceWidth = style_ptr->object[type][pieceVectorPosition].width * 8;
				int pieceHeight = style_ptr->object[type][pieceVectorPosition].height * 2;

				while (pieceWidth * pieceZoom * 2 < 128 && pieceHeight * pieceZoom * 2 < 128)
					pieceZoom *= 2;

				if (pieceZoom > 4)
					pieceZoom = 4;

				int pieceXOffset = (128 - (pieceWidth * pieceZoom)) / 2;
				int pieceYOffset = (128 - (pieceHeight * pieceZoom)) / 2;

				if (pieceWidth > 128)
					pieceXOffset = 0;
				if (pieceHeight > 128)
					pieceYOffset = 0;

				style_ptr->draw_object_texture(window_ptr, x + 4 + pieceXOffset, window_ptr->height - BAR_HEIGHT + 4 + pieceYOffset, type, pieceVectorPosition, pieceZoom, 128);
				x += 132;
			}

		}

		SDL_SetRenderTarget(window_ptr->screen_renderer, NULL);

		//redraw = false;
	}
}