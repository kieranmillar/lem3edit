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

#include "SDL.h"

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
	for (int i = 0; i < 3; i++)
	{
		barTypeCount[i] = style_ptr->object[i].size();
		barTypeMax[i] = barTypeCount[i] * PIECESIZE;
	}
	barScrollX = 0;
	type = PERM;

	resizeBarScrollRect(window_ptr->width, window_ptr->height);
	barScrollRect.h = 13;

	buttonTexture = SDL_CreateTexture(window_ptr->screen_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, 512, 32);
	SDL_SetTextureBlendMode(buttonTexture, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(buttonTexture, 255);

	currentButtonTextureX = 0;

	loadButtonGraphic(button_layerBackground, "./gfx/layerBackground_off.bmp", "./gfx/layerBackground_on.bmp");
	loadButtonGraphic(button_layerTerrain, "./gfx/layerTerrain_off.bmp", "./gfx/layerTerrain_on.bmp");
	loadButtonGraphic(button_layerTool, "./gfx/layerTool_off.bmp", "./gfx/layerTool_on.bmp");
	loadButtonGraphic(button_layerVisible, "./gfx/layerVisible_off.bmp", "./gfx/layerVisible_on.bmp");
	loadButtonGraphic(button_save, "./gfx/save_up.bmp", "./gfx/save_down.bmp");
	loadButtonGraphic(button_moveToBack, "./gfx/moveToBack_up.bmp", "./gfx/moveToBack_down.bmp");
	loadButtonGraphic(button_moveToFront, "./gfx/moveToFront_up.bmp", "./gfx/moveToFront_down.bmp");
	loadButtonGraphic(button_camera, "./gfx/camera_off.bmp", "./gfx/camera_on.bmp");
}

bool Bar::loadButtonGraphic(buttonInfo & button, const char * filePathUp, const char * filePathDown)
{

	SDL_SetRenderDrawColor(window_ptr->screen_renderer, 255, 255, 255, 255);
	SDL_Surface* graphic = NULL;
	//first load the botton's up graphic
	graphic = SDL_LoadBMP(filePathUp);
	if (graphic == NULL)
	{ 
		SDL_Log("Unable to load image '%s'! SDL Error: %s\n", filePathUp, SDL_GetError());
		return false;
	}
	SDL_ConvertSurfaceFormat(graphic, SDL_PIXELFORMAT_RGB888, 0);

	SDL_Texture * texture = SDL_CreateTextureFromSurface(window_ptr->screen_renderer, graphic);
	SDL_Rect rect;
	rect.x = currentButtonTextureX;
	rect.y = 0;
	rect.w = 32;
	rect.h = 32;

	SDL_SetRenderTarget(window_ptr->screen_renderer, buttonTexture);
	SDL_RenderCopy(window_ptr->screen_renderer, texture, NULL, &rect);

	SDL_FreeSurface(graphic);
	graphic = NULL;
	SDL_DestroyTexture(texture);
	texture = NULL;
	button.texOffX = rect.x;
	button.texOffY = rect.y;

	currentButtonTextureX += rect.w;

	//then the down graphic
	graphic = SDL_LoadBMP(filePathDown);
	if (graphic == NULL)
	{
		SDL_Log("Unable to load image '%s'! SDL Error: %s\n", filePathDown, SDL_GetError());
		return false;
	}

	texture = SDL_CreateTextureFromSurface(window_ptr->screen_renderer, graphic);
	rect.x = currentButtonTextureX;

	SDL_SetRenderTarget(window_ptr->screen_renderer, buttonTexture);
	SDL_RenderCopy(window_ptr->screen_renderer, texture, NULL, &rect);

	SDL_FreeSurface(graphic);
	graphic = NULL;
	SDL_DestroyTexture(texture);
	texture = NULL;

	button.texOnX = rect.x;
	button.texOnY = rect.y;

	currentButtonTextureX += rect.w;

	return true;
}

void Bar::resizeBarScrollRect(int windowWidth, int windowHeight)
{
	barScrollRect.y = windowHeight - 14;
	barScrollRect.w = (window_ptr->width - PANEL_WIDTH - 33) * 1000 / barTypeMax[type];
	barScrollRect.w *= (window_ptr->width - PANEL_WIDTH - 33);
	barScrollRect.w /= 1000;
	barScrollRect.w += 1;
	scroll(0);
}

void Bar::scroll(signed int moveAmount)
{
	barScrollX += moveAmount;
	if (barScrollX < 0)
		barScrollX = 0;
	if (barScrollX > barTypeMax[type] - (window_ptr->width - PANEL_WIDTH))
		barScrollX = barTypeMax[type] - (window_ptr->width - PANEL_WIDTH);
	updateBarScrollPos(barScrollX);
}

void Bar::updateBarScrollPos(int xPos)
{
	barScrollRect.x = (xPos * 1000) / barTypeMax[type];
	barScrollRect.x *= (window_ptr->width - PANEL_WIDTH - 33);
	barScrollRect.x /= 1000;
	barScrollRect.x += PANEL_WIDTH + 18;
}

void Bar::moveScrollBar(int moveLocationInWindow)
{
	int x = moveLocationInWindow - PANEL_WIDTH - 18;
	int xMax = window_ptr->width - PANEL_WIDTH - 33;
	if (x < 0)
		x = 0;
	if (x > xMax - barScrollRect.w)
		x = xMax - barScrollRect.w;
	int factor = x * 1000 / xMax;
	barScrollX = (barTypeMax[type] * factor) / 1000;
	scroll(0);
}

void Bar::changeType(int t)
{
	canvas_ptr->layerVisible[t] = true;
	canvas_ptr->redraw = true;
	if (type == t)
		return;
	type = t;
	barScrollX = 0;
	resizeBarScrollRect(window_ptr->width, window_ptr->height);
}

int Bar::getPieceIDByScreenPos(int mousePos)
{
	unsigned int piece = mousePos + barScrollX - PANEL_WIDTH;
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
		rPieceSelector.x = PANEL_WIDTH;
		rPieceSelector.y = canvas_ptr->height;
		rPieceSelector.w = window_ptr->width - PANEL_WIDTH;
		rPieceSelector.h = window_ptr->height;
		SDL_RenderFillRect(window_ptr->screen_renderer, &rPieceSelector);

	}

	{ // draw the pieces
		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 0, 0, 0, 255);
		int pieceStart = barScrollX / PIECESIZE;
		int x = PANEL_WIDTH + 1 - (barScrollX % PIECESIZE);
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
		SDL_RenderDrawLine(window_ptr->screen_renderer, PANEL_WIDTH, window_ptr->height - 16, window_ptr->width, window_ptr->height - 16);
		SDL_RenderDrawLine(window_ptr->screen_renderer, PANEL_WIDTH + 16, window_ptr->height - 16, PANEL_WIDTH + 16, window_ptr->height);
		SDL_RenderDrawLine(window_ptr->screen_renderer, window_ptr->width - 16, window_ptr->height - 16, window_ptr->width - 16, window_ptr->height);
		//left button arrow
		SDL_RenderDrawLine(window_ptr->screen_renderer, PANEL_WIDTH + 11, window_ptr->height - 14, PANEL_WIDTH + 5, window_ptr->height - 8);
		SDL_RenderDrawLine(window_ptr->screen_renderer, PANEL_WIDTH + 5, window_ptr->height - 8, PANEL_WIDTH + 11, window_ptr->height - 2);
		SDL_RenderDrawLine(window_ptr->screen_renderer, PANEL_WIDTH + 11, window_ptr->height - 14, PANEL_WIDTH + 11, window_ptr->height - 2);
		//right button arrow
		SDL_RenderDrawLine(window_ptr->screen_renderer, window_ptr->width - 11, window_ptr->height - 14, window_ptr->width - 5, window_ptr->height - 8);
		SDL_RenderDrawLine(window_ptr->screen_renderer, window_ptr->width - 5, window_ptr->height - 8, window_ptr->width - 11, window_ptr->height - 2);
		SDL_RenderDrawLine(window_ptr->screen_renderer, window_ptr->width - 11, window_ptr->height - 14, window_ptr->width - 11, window_ptr->height - 2);
		//the scroll bar
		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 100, 100, 100, 255);
		SDL_RenderFillRect(window_ptr->screen_renderer, &barScrollRect);
	}

	{ // draw the options panel
		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 130, 130, 130, 255);
		SDL_Rect rOptionsBox;
		rOptionsBox.x = 0;
		rOptionsBox.y = canvas_ptr->height;
		rOptionsBox.w = PANEL_WIDTH;
		rOptionsBox.h = window_ptr->height;
		SDL_RenderFillRect(window_ptr->screen_renderer, &rOptionsBox);

		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawLine(window_ptr->screen_renderer, 0, canvas_ptr->height, window_ptr->width, canvas_ptr->height);
		SDL_RenderDrawLine(window_ptr->screen_renderer, PANEL_WIDTH, canvas_ptr->height, PANEL_WIDTH, window_ptr->height);

		{
			//draw background layer buttons
			if (type == PERM)
				drawButton(button_layerBackground, on, 3, canvas_ptr->height + 3);
			else
				drawButton(button_layerBackground, off, 3, canvas_ptr->height + 3);
			if (canvas_ptr->layerVisible[PERM])
				drawButton(button_layerVisible, on, 3, canvas_ptr->height + 39);
			else
				drawButton(button_layerVisible, off, 3, canvas_ptr->height + 39);
			SDL_Rect rect;
			rect.x = 1;
			rect.y = canvas_ptr->height + 1;
			rect.w = 36;
			rect.h = 72;
			SDL_RenderDrawRect(window_ptr->screen_renderer, &rect);
		}
		{
			//draw terrain layer buttons
			if (type == TEMP)
				drawButton(button_layerTerrain, on, 39, canvas_ptr->height + 3);
			else
				drawButton(button_layerTerrain, off, 39, canvas_ptr->height + 3);
			if (canvas_ptr->layerVisible[TEMP])
				drawButton(button_layerVisible, on, 39, canvas_ptr->height + 39);
			else
				drawButton(button_layerVisible, off, 39, canvas_ptr->height + 39);
			SDL_Rect rect;
			rect.x = 37;
			rect.y = canvas_ptr->height + 1;
			rect.w = 36;
			rect.h = 72;
			SDL_RenderDrawRect(window_ptr->screen_renderer, &rect);
		}
		{
			//draw tool layer buttons
			if (type == TOOL)
				drawButton(button_layerTool, on, 75, canvas_ptr->height + 3);
			else
				drawButton(button_layerTool, off, 75, canvas_ptr->height + 3);
			if (canvas_ptr->layerVisible[TOOL])
				drawButton(button_layerVisible, on, 75, canvas_ptr->height + 39);
			else
				drawButton(button_layerVisible, off, 75, canvas_ptr->height + 39);
			SDL_Rect rect;
			rect.x = 73;
			rect.y = canvas_ptr->height + 1;
			rect.w = 36;
			rect.h = 72;
			SDL_RenderDrawRect(window_ptr->screen_renderer, &rect);
		}

		drawButton(button_save, off, 111, canvas_ptr->height + 3);
		drawButton(button_moveToBack, off, 3, canvas_ptr->height + 75);
		drawButton(button_moveToFront, off, 39, canvas_ptr->height + 75);
		if (editor_ptr->startCameraOn)
			drawButton(button_camera, on, 75, canvas_ptr->height + 75);
		else
			drawButton(button_camera, off, 75, canvas_ptr->height + 75);

	}

	SDL_SetRenderTarget(window_ptr->screen_renderer, NULL);
}

void Bar::drawButton(const buttonInfo & button, buttonState state, int x, int y)
{
	SDL_Rect sourceRect;
	if (state == off)
	{
		sourceRect.x = button.texOffX;
		sourceRect.y = button.texOffY;
	}
	if (state == on)
	{
		sourceRect.x = button.texOnX;
		sourceRect.y = button.texOnY;
	}
	sourceRect.w = 32;
	sourceRect.h = 32;

	SDL_Rect destRect;
	destRect.x = x;
	destRect.y = y;
	destRect.w = 32;
	destRect.h = 32;

	SDL_RenderCopy(window_ptr->screen_renderer, buttonTexture, &sourceRect, &destRect);
	}