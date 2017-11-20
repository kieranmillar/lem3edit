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
This file includes code to deal with the object bar at the bottom of the editor screen
*/

#include "bar.hpp"
#include "editor.hpp"
#include "../style.hpp"
#include "../window.hpp"

#include <cassert>
#include <stdlib.h>
#include <string>

void Bar::setReferences(Window * w, Editor * e, Canvas * c, Style * s)
{
	window_ptr = w;
	editor_ptr = e;
	canvas_ptr = c;
	style_ptr = s;
}

void Bar::load( void )
{
	barPERMCount = style_ptr->object[PERM].size();
	barMaxPERM = barPERMCount * PIECESIZE;
	barTEMPCount = style_ptr->object[TEMP].size();
	barMaxTEMP = barTEMPCount * PIECESIZE;
	barScrollX = 0;
	type = PERM;
	barMax = barMaxPERM;

	resizeBarScrollRect(window_ptr->width, window_ptr->height);
	barScrollRect.h = 13;
}

void Bar::resizeBarScrollRect(int windowWidth, int windowHeight)
{
	barScrollRect.y = windowHeight - 14;
	barScrollRect.w = (window_ptr->width - BAR_HEIGHT - 33) * 1000 / barMax;
	barScrollRect.w *= (window_ptr->width - BAR_HEIGHT - 33);
	barScrollRect.w /= 1000;
	barScrollRect.w += 1;
	scroll(0);
}

void Bar::scroll(signed int moveAmount)
{
	barScrollX += moveAmount;
	if (barScrollX < 0)
		barScrollX = 0;
	if (barScrollX > barMax - (window_ptr->width - BAR_HEIGHT))
		barScrollX = barMax - (window_ptr->width - BAR_HEIGHT);
	updateBarScrollPos(barScrollX);
}

void Bar::updateBarScrollPos(int xPos)
{
	barScrollRect.x = (xPos * 1000) / barMax;
	barScrollRect.x *= (window_ptr->width - BAR_HEIGHT - 33);
	barScrollRect.x /= 1000;
	barScrollRect.x += BAR_HEIGHT + 18;
}

void Bar::moveScrollBar(int moveLocationInWindow)
{
	int x = moveLocationInWindow - BAR_HEIGHT - 18;
	int xMax = window_ptr->width - BAR_HEIGHT - 33;
	if (x < 0)
		x = 0;
	if (x > xMax - barScrollRect.w)
		x = xMax - barScrollRect.w;
	int factor = x * 1000 / xMax;
	barScrollX = (barMax * factor) / 1000;
	scroll(0);
}

void Bar::changeType(int t)
{
	if (type == t)
		return;
	type = t;
	barScrollX = 0;
	if (t == PERM)
		barMax = barMaxPERM;
	if (t == TEMP)
		barMax = barMaxTEMP;
	resizeBarScrollRect(window_ptr->width, window_ptr->height);
}

int Bar::getPieceIDByScreenPos(int mousePos)
{
	int piece = mousePos + barScrollX - BAR_HEIGHT;
	piece /= PIECESIZE;

	if (piece > style_ptr->object[type].size())
		return -1;
	int id = style_ptr->object[type][piece].id;
	return id;
}

void Bar::draw( void )
{
	{ // bar background in the absense of a proper graphic
		SDL_SetRenderDrawBlendMode(window_ptr->screen_renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderTarget(window_ptr->screen_renderer, window_ptr->screen_texture);

		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 150, 150, 150, 255);
		SDL_Rect rPieceSelector;
		rPieceSelector.x = BAR_HEIGHT;
		rPieceSelector.y = canvas_ptr->height;
		rPieceSelector.w = window_ptr->width - BAR_HEIGHT;
		rPieceSelector.h = window_ptr->height;
		SDL_RenderFillRect(window_ptr->screen_renderer, &rPieceSelector);

	}

	{ // draw the pieces
		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 0, 0, 0, 255);
		int pieceStart = barScrollX / PIECESIZE;
		int x = BAR_HEIGHT + 1 - (barScrollX % PIECESIZE);
		assert((unsigned)type < COUNTOF(style_ptr->object));

		for (std::vector<Style::Object>::iterator i = style_ptr->object[type].begin() + pieceStart; i != style_ptr->object[type].end(); ++i)
		{
			SDL_Rect rPieceBox;
			rPieceBox.x = x + 2;
			rPieceBox.y = canvas_ptr->height + 2;
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

			style_ptr->draw_object_texture(window_ptr, x + 4 + pieceXOffset, canvas_ptr->height + 4 + pieceYOffset, type, pieceVectorPosition, pieceZoom, 128);
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
		rOptionsBox.y = canvas_ptr->height;
		rOptionsBox.w = BAR_HEIGHT;
		rOptionsBox.h = window_ptr->height;
		SDL_RenderFillRect(window_ptr->screen_renderer, &rOptionsBox);

		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawLine(window_ptr->screen_renderer, 0, canvas_ptr->height, window_ptr->width, canvas_ptr->height);
		SDL_RenderDrawLine(window_ptr->screen_renderer, BAR_HEIGHT, canvas_ptr->height, BAR_HEIGHT, window_ptr->height);
	}

	SDL_SetRenderTarget(window_ptr->screen_renderer, NULL);
}