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

#include "bar.hpp"
#include "editor.hpp"
#include "../style.hpp"
#include "../window.hpp"

#include <cassert>
#include <stdlib.h>
#include <string>

void Bar::load(Window * w, Editor * e, Style * s)
{
	window_ptr = w;
	editor_ptr = e;
	style_ptr = s;
	barScrollX = 7000;
	barPERMCount = style_ptr->object[0].size();
	barMaxPERM = barPERMCount * PIECESIZE;
	barTEMPCount = style_ptr->object[1].size();
	barMaxTEMP = barTEMPCount * PIECESIZE;
	
	resizeBarScrollRect(window_ptr->width, window_ptr->height);
	barScrollRect.h = 14;
}

void Bar::resizeBarScrollRect(int windowWidth, int windowHeight)
{
	barScrollRect.y = windowHeight - 15;
	barScrollRect.w = (windowWidth - BAR_HEIGHT - 33) * 1000 / barMaxPERM;
	barScrollRect.w *= (windowWidth - BAR_HEIGHT - 33);
	barScrollRect.w /= 1000;
	updateBarScrollPos(barScrollX);

}

void Bar::updateBarScrollPos(int xPos)
{
	barScrollRect.x = (xPos * 1000) / barMaxPERM;
	barScrollRect.x *= (window_ptr->width - BAR_HEIGHT - 33);
	barScrollRect.x /= 1000;
	barScrollRect.x += BAR_HEIGHT + 17;
}

void Bar::draw( void )
{
	{ // bar background in the absense of a proper graphic
		SDL_SetRenderDrawBlendMode(window_ptr->screen_renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderTarget(window_ptr->screen_renderer, window_ptr->screen_texture);

		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 150, 150, 150, 255);
		SDL_Rect rPieceSelector;
		rPieceSelector.x = BAR_HEIGHT;
		rPieceSelector.y = window_ptr->height - BAR_HEIGHT;
		rPieceSelector.w = window_ptr->width - BAR_HEIGHT;
		rPieceSelector.h = window_ptr->height;
		SDL_RenderFillRect(window_ptr->screen_renderer, &rPieceSelector);

	}

	{ // draw the pieces
		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 0, 0, 0, 255);
		int pieceStart = barScrollX / PIECESIZE;
		int x = BAR_HEIGHT + 1 - (barScrollX % PIECESIZE);
		int type = 0;
		assert((unsigned)type < COUNTOF(style_ptr->object));

		for (std::vector<Style::Object>::iterator i = style_ptr->object[type].begin() + pieceStart; i != style_ptr->object[type].end(); ++i)
		{
			SDL_Rect rPieceBox;
			rPieceBox.x = x + 2;
			rPieceBox.y = window_ptr->height - BAR_HEIGHT + 2;
			rPieceBox.w = PIECESIZE - 1;
			rPieceBox.h = PIECESIZE - 1;
			SDL_RenderDrawRect(window_ptr->screen_renderer, &rPieceBox);

			int pieceZoom = 1;
			int pieceVectorPosition = i - style_ptr->object[type].begin();
			int pieceWidth = style_ptr->object[type][pieceVectorPosition].width * 8;
			int pieceHeight = style_ptr->object[type][pieceVectorPosition].height * 2;

			if (pieceWidth < 32 && pieceHeight < 32)
				pieceZoom = 4;
			else if (pieceWidth < 64 && pieceHeight < 64)
				pieceZoom = 2;

			int pieceXOffset = (128 - (pieceWidth * pieceZoom)) / 2;
			int pieceYOffset = (128 - (pieceHeight * pieceZoom)) / 2;

			if (pieceWidth > 128)
				pieceXOffset = 0;
			if (pieceHeight > 128)
				pieceYOffset = 0;

			style_ptr->draw_object_texture(window_ptr, x + 4 + pieceXOffset, window_ptr->height - BAR_HEIGHT + 4 + pieceYOffset, type, pieceVectorPosition, pieceZoom, 128);
			x += PIECESIZE;

			if (x > window_ptr->width)
				break;
		}

	}

	{ // draw the scroll bar area
		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 0, 0, 0, 255);
		//button boxes
		SDL_RenderDrawLine(window_ptr->screen_renderer, BAR_HEIGHT, window_ptr->height - 16, window_ptr->width, window_ptr->height - 16);
		SDL_RenderDrawLine(window_ptr->screen_renderer, BAR_HEIGHT + 16, window_ptr->height - 16, BAR_HEIGHT + 16, window_ptr->height);
		SDL_RenderDrawLine(window_ptr->screen_renderer, window_ptr->width - 16, window_ptr->height - 16, window_ptr->width - 16, window_ptr->height);
		//left button arrow
		SDL_RenderDrawLine(window_ptr->screen_renderer, BAR_HEIGHT + 11, window_ptr->height - 14, BAR_HEIGHT + 5, window_ptr->height - 8);
		SDL_RenderDrawLine(window_ptr->screen_renderer, BAR_HEIGHT + 5, window_ptr->height - 8, BAR_HEIGHT + 11, window_ptr->height - 2);
		SDL_RenderDrawLine(window_ptr->screen_renderer, BAR_HEIGHT + 11, window_ptr->height - 14, BAR_HEIGHT + 11, window_ptr->height - 2);
		//right button arrow
		SDL_RenderDrawLine(window_ptr->screen_renderer, window_ptr->width - 11, window_ptr->height - 14, window_ptr->width - 5, window_ptr->height - 8);
		SDL_RenderDrawLine(window_ptr->screen_renderer, window_ptr->width - 5, window_ptr->height - 8, window_ptr->width - 11, window_ptr->height - 2);
		SDL_RenderDrawLine(window_ptr->screen_renderer, window_ptr->width - 11, window_ptr->height - 14, window_ptr->width - 11, window_ptr->height - 2);
		//the scroll bar
		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 100, 100, 100, 255);
		SDL_RenderFillRect(window_ptr->screen_renderer, &barScrollRect);
	}

	{ // draw the options box
		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 130, 130, 130, 255);
		SDL_Rect rOptionsBox;
		rOptionsBox.x = 0;
		rOptionsBox.y = window_ptr->height - BAR_HEIGHT;
		rOptionsBox.w = BAR_HEIGHT;
		rOptionsBox.h = window_ptr->height;
		SDL_RenderFillRect(window_ptr->screen_renderer, &rOptionsBox);

		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawLine(window_ptr->screen_renderer, 0, window_ptr->height - BAR_HEIGHT, window_ptr->width, window_ptr->height - BAR_HEIGHT);
		SDL_RenderDrawLine(window_ptr->screen_renderer, BAR_HEIGHT, window_ptr->height - BAR_HEIGHT, BAR_HEIGHT, window_ptr->height);
	}

	SDL_SetRenderTarget(window_ptr->screen_renderer, NULL);
}