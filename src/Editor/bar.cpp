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
This file includes code to deal with the object bar at the bottom of the editor screen
*/

#include "bar.hpp"
#include "editor.hpp"
#include "../font.hpp"
#include "../style.hpp"
#include "../window.hpp"

#include "SDL.h"

#include <cassert>
#include <stdlib.h>
#include <string>

void Bar::setReferences(Editor * e, Canvas * c, Style * s)
{
	editor_ptr = e;
	canvas_ptr = c;
	style_ptr = s;
}

void Bar::load(void)
{
	for (int i = 0; i < 3; i++)
	{
		barTypeCount[i] = style_ptr->object[i].size();
		barTypeMax[i] = barTypeCount[i] * PIECESIZE;
	}
	barScrollX = 0;
	type = PERM;

	resizeBarScrollRect(g_window.width, g_window.height);
	barScrollRect.h = 13;

	loadButtonGraphic(button_layerBackground, "./gfx/layerBackground_off.bmp", "./gfx/layerBackground_on.bmp");
	loadButtonGraphic(button_layerTerrain, "./gfx/layerTerrain_off.bmp", "./gfx/layerTerrain_on.bmp");
	loadButtonGraphic(button_layerTool, "./gfx/layerTool_off.bmp", "./gfx/layerTool_on.bmp");
	loadButtonGraphic(button_layerBackgroundVisible, "./gfx/layerVisible_off.bmp", "./gfx/layerVisible_on.bmp");
	loadButtonGraphic(button_layerTerrainVisible, "./gfx/layerVisible_off.bmp", "./gfx/layerVisible_on.bmp");
	loadButtonGraphic(button_layerToolVisible, "./gfx/layerVisible_off.bmp", "./gfx/layerVisible_on.bmp");
	loadButtonGraphic(button_save, "./gfx/save_up.bmp", NULL);
	loadButtonGraphic(button_moveToBack, "./gfx/moveToBack_up.bmp", NULL);
	loadButtonGraphic(button_moveToFront, "./gfx/moveToFront_up.bmp", NULL);
	loadButtonGraphic(button_camera, "./gfx/camera_off.bmp", "./gfx/camera_on.bmp");
	loadButtonGraphic(button_levelProperties, "./gfx/levelProperties_up.bmp", NULL);
	loadButtonGraphic(button_copy, "./gfx/copy_up.bmp", NULL);
	loadButtonGraphic(button_paste, "./gfx/paste_up.bmp", NULL);
	loadButtonGraphic(button_delete, "./gfx/delete_up.bmp", NULL);
	loadButtonGraphic(button_quit, "./gfx/quit_up.bmp", NULL);

	tooltipFont = TTF_OpenFont("./gfx/DejaVuSansMono.ttf", 14);

	setButtonTooltip(button_layerBackground, "Select Background Layer (1)");
	setButtonTooltip(button_layerTerrain, "Select Terrain Layer (2)");
	setButtonTooltip(button_layerTool, "Select Tool and Creature Layer (3)");
	setButtonTooltip(button_layerBackgroundVisible, "Toggle Background Layer Visibility (Ctrl+1)");
	setButtonTooltip(button_layerTerrainVisible, "Toggle Terrain Layer Visibility (Ctrl+2)");
	setButtonTooltip(button_layerToolVisible, "Toggle Tool and Creature Layer Visibility (Ctrl+3)");
	setButtonTooltip(button_save, "Save Level (s)");
	setButtonTooltip(button_moveToBack, "Move Selected Objects To Back (,)");
	setButtonTooltip(button_moveToFront, "Move Selected Objects To Front (.)");
	setButtonTooltip(button_camera, "Toggle Start Camera Visibility (space)");
	setButtonTooltip(button_levelProperties, "Edit Level Properties (p)");
	setButtonTooltip(button_copy, "Copy Selected Objects (Ctrl+c)");
	setButtonTooltip(button_paste, "Paste Copied Objects (Ctrl+v)");
	setButtonTooltip(button_delete, "Delete Selected Objects (delete)");
	setButtonTooltip(button_quit, "Quit and return to main menu. ALL UNSAVED CHANGES WILL BE LOST! (q)");

	TTF_CloseFont(tooltipFont);
}

bool Bar::loadButtonGraphic(buttonInfo & button, const char * filePathUp, const char * filePathDown)
{
	SDL_SetRenderDrawColor(g_window.screen_renderer, 255, 255, 255, 255);
	SDL_Surface* graphic = NULL;
	//first load the botton's up graphic
	graphic = SDL_LoadBMP(filePathUp);
	if (graphic == NULL)
	{
		SDL_Log("Unable to load image '%s'! SDL Error: %s\n", filePathUp, SDL_GetError());
		return false;
	}
	SDL_ConvertSurfaceFormat(graphic, SDL_PIXELFORMAT_RGBA8888, 0);

	button.buttonTexUp = SDL_CreateTextureFromSurface(g_window.screen_renderer, graphic);

	SDL_FreeSurface(graphic);
	graphic = NULL;

	//then the down graphic
	if (filePathDown != NULL)
	{
		graphic = SDL_LoadBMP(filePathDown);
		if (graphic == NULL)
		{
			SDL_Log("Unable to load image '%s'! SDL Error: %s\n", filePathDown, SDL_GetError());
			return false;
		}

		button.buttonTexDown = SDL_CreateTextureFromSurface(g_window.screen_renderer, graphic);

		SDL_FreeSurface(graphic);
		graphic = NULL;
	}

	return true;
}

bool Bar::setButtonTooltip(buttonInfo & button, const char * text)
{
	button.tooltip = Font::createTextureFromString(tooltipFont, text);
	return true;
}

void Bar::resizeBarScrollRect(int windowWidth, int windowHeight)
{
	barScrollRect.y = windowHeight - 14;
	barScrollRect.w = (g_window.width - PANEL_WIDTH - 33) * 1000 / barTypeMax[type];
	barScrollRect.w *= (g_window.width - PANEL_WIDTH - 33);
	barScrollRect.w /= 1000;
	barScrollRect.w += 1;
	scroll(0);
}

void Bar::scroll(signed int moveAmount)
{
	barScrollX += moveAmount;
	if (barScrollX < 0)
		barScrollX = 0;
	if (barScrollX > barTypeMax[type] - (g_window.width - PANEL_WIDTH))
		barScrollX = barTypeMax[type] - (g_window.width - PANEL_WIDTH);
	updateBarScrollPos(barScrollX);
}

void Bar::updateBarScrollPos(int xPos)
{
	barScrollRect.x = (xPos * 1000) / barTypeMax[type];
	barScrollRect.x *= (g_window.width - PANEL_WIDTH - 33);
	barScrollRect.x /= 1000;
	barScrollRect.x += PANEL_WIDTH + 18;
}

void Bar::moveScrollBar(int moveLocationInWindow)
{
	int x = moveLocationInWindow - PANEL_WIDTH - 18;
	int xMax = g_window.width - PANEL_WIDTH - 33;
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
	resizeBarScrollRect(g_window.width, g_window.height);
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

void Bar::draw(int mouseX, int mouseY)
{
	{ // bar background in the absense of a proper graphic
		SDL_SetRenderDrawBlendMode(g_window.screen_renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderTarget(g_window.screen_renderer, g_window.screen_texture);

		SDL_SetRenderDrawColor(g_window.screen_renderer, 150, 150, 150, 255);
		SDL_Rect rPieceSelector;
		rPieceSelector.x = PANEL_WIDTH;
		rPieceSelector.y = canvas_ptr->height;
		rPieceSelector.w = g_window.width - PANEL_WIDTH;
		rPieceSelector.h = g_window.height;
		SDL_RenderFillRect(g_window.screen_renderer, &rPieceSelector);
	}

	{ // draw the pieces
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
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
			SDL_RenderDrawRect(g_window.screen_renderer, &rPieceBox);

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

			style_ptr->draw_object_texture(x + 4 + pieceXOffset, canvas_ptr->height + 4 + pieceYOffset, type, pieceVectorPosition, pieceZoom, 128);
			x += PIECESIZE;

			if (x > g_window.width)
				break;
		}
	}

	{ // draw the scroll bar area
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		//button boxes
		SDL_RenderDrawLine(g_window.screen_renderer, PANEL_WIDTH, g_window.height - 16, g_window.width, g_window.height - 16);
		SDL_RenderDrawLine(g_window.screen_renderer, PANEL_WIDTH + 16, g_window.height - 16, PANEL_WIDTH + 16, g_window.height);
		SDL_RenderDrawLine(g_window.screen_renderer, g_window.width - 16, g_window.height - 16, g_window.width - 16, g_window.height);
		//left button arrow
		SDL_RenderDrawLine(g_window.screen_renderer, PANEL_WIDTH + 11, g_window.height - 14, PANEL_WIDTH + 5, g_window.height - 8);
		SDL_RenderDrawLine(g_window.screen_renderer, PANEL_WIDTH + 5, g_window.height - 8, PANEL_WIDTH + 11, g_window.height - 2);
		SDL_RenderDrawLine(g_window.screen_renderer, PANEL_WIDTH + 11, g_window.height - 14, PANEL_WIDTH + 11, g_window.height - 2);
		//right button arrow
		SDL_RenderDrawLine(g_window.screen_renderer, g_window.width - 11, g_window.height - 14, g_window.width - 5, g_window.height - 8);
		SDL_RenderDrawLine(g_window.screen_renderer, g_window.width - 5, g_window.height - 8, g_window.width - 11, g_window.height - 2);
		SDL_RenderDrawLine(g_window.screen_renderer, g_window.width - 11, g_window.height - 14, g_window.width - 11, g_window.height - 2);
		//the scroll bar
		SDL_SetRenderDrawColor(g_window.screen_renderer, 100, 100, 100, 255);
		SDL_RenderFillRect(g_window.screen_renderer, &barScrollRect);
	}

	{ // draw the options panel
		SDL_SetRenderDrawColor(g_window.screen_renderer, 130, 130, 130, 255);
		SDL_Rect rOptionsBox;
		rOptionsBox.x = 0;
		rOptionsBox.y = canvas_ptr->height;
		rOptionsBox.w = PANEL_WIDTH;
		rOptionsBox.h = g_window.height;
		SDL_RenderFillRect(g_window.screen_renderer, &rOptionsBox);

		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawLine(g_window.screen_renderer, 0, canvas_ptr->height, g_window.width, canvas_ptr->height);
		SDL_RenderDrawLine(g_window.screen_renderer, PANEL_WIDTH, canvas_ptr->height, PANEL_WIDTH, g_window.height);

		{
			//draw background layer buttons
			if (type == PERM)
				drawButton(button_layerBackground, on, 3, canvas_ptr->height + 3);
			else
				drawButton(button_layerBackground, off, 3, canvas_ptr->height + 3);
			if (canvas_ptr->layerVisible[PERM])
				drawButton(button_layerBackgroundVisible, on, 3, canvas_ptr->height + 39);
			else
				drawButton(button_layerBackgroundVisible, off, 3, canvas_ptr->height + 39);
			SDL_Rect rect;
			rect.x = 1;
			rect.y = canvas_ptr->height + 1;
			rect.w = 36;
			rect.h = 72;
			SDL_RenderDrawRect(g_window.screen_renderer, &rect);
		}
		{
			//draw terrain layer buttons
			if (type == TEMP)
				drawButton(button_layerTerrain, on, 39, canvas_ptr->height + 3);
			else
				drawButton(button_layerTerrain, off, 39, canvas_ptr->height + 3);
			if (canvas_ptr->layerVisible[TEMP])
				drawButton(button_layerTerrainVisible, on, 39, canvas_ptr->height + 39);
			else
				drawButton(button_layerTerrainVisible, off, 39, canvas_ptr->height + 39);
			SDL_Rect rect;
			rect.x = 37;
			rect.y = canvas_ptr->height + 1;
			rect.w = 36;
			rect.h = 72;
			SDL_RenderDrawRect(g_window.screen_renderer, &rect);
		}
		{
			//draw tool layer buttons
			if (type == TOOL)
				drawButton(button_layerTool, on, 75, canvas_ptr->height + 3);
			else
				drawButton(button_layerTool, off, 75, canvas_ptr->height + 3);
			if (canvas_ptr->layerVisible[TOOL])
				drawButton(button_layerToolVisible, on, 75, canvas_ptr->height + 39);
			else
				drawButton(button_layerToolVisible, off, 75, canvas_ptr->height + 39);
			SDL_Rect rect;
			rect.x = 73;
			rect.y = canvas_ptr->height + 1;
			rect.w = 36;
			rect.h = 72;
			SDL_RenderDrawRect(g_window.screen_renderer, &rect);
		}
		{
			//draw all the other buttons
			drawButton(button_save, off, 111, canvas_ptr->height + 3);
			drawButton(button_moveToBack, off, 3, canvas_ptr->height + 75);
			drawButton(button_moveToFront, off, 39, canvas_ptr->height + 75);
			if (editor_ptr->startCameraOn)
				drawButton(button_camera, on, 75, canvas_ptr->height + 75);
			else
				drawButton(button_camera, off, 75, canvas_ptr->height + 75);
			drawButton(button_levelProperties, off, 111, canvas_ptr->height + 75);
			drawButton(button_copy, off, 3, canvas_ptr->height + 111);
			drawButton(button_paste, off, 39, canvas_ptr->height + 111);
			drawButton(button_delete, off, 75, canvas_ptr->height + 111);
			drawButton(button_quit, off, 111, canvas_ptr->height + 111);
		}
	}
	{
		//draw tooltip
		if (mouseY > canvas_ptr->height)
		{
			if (mouseY > canvas_ptr->height + 3 && mouseY < canvas_ptr->height + 35)
				//first row of buttons
			{
				if (mouseX > 3 && mouseX < 35)
				{
					drawTooltip(button_layerBackground, mouseX, mouseY);
				}
				if (mouseX > 39 && mouseX < 71)
				{
					drawTooltip(button_layerTerrain, mouseX, mouseY);
				}
				if (mouseX > 75 && mouseX < 107)
				{
					drawTooltip(button_layerTool, mouseX, mouseY);
				}
				if (mouseX > 111 && mouseX < 143)
				{
					drawTooltip(button_save, mouseX, mouseY);
				}
			}
			if (mouseY > g_window.height - BAR_HEIGHT + 39 && mouseY < g_window.height - BAR_HEIGHT + 71)
				//second row of buttons
			{
				if (mouseX > 3 && mouseX < 35)
				{
					drawTooltip(button_layerBackgroundVisible, mouseX, mouseY);
				}
				if (mouseX > 39 && mouseX < 71)
				{
					drawTooltip(button_layerTerrainVisible, mouseX, mouseY);
				}
				if (mouseX > 75 && mouseX < 107)
				{
					drawTooltip(button_layerToolVisible, mouseX, mouseY);
				}
				/*if (mouseX > 111 && mouseX < 143)
				{
				}*/
			}
			if (mouseY > g_window.height - BAR_HEIGHT + 75 && mouseY < g_window.height - BAR_HEIGHT + 107)
				//third row of buttons
			{
				if (mouseX > 3 && mouseX < 35)
				{
					drawTooltip(button_moveToBack, mouseX, mouseY);
				}
				if (mouseX > 39 && mouseX < 71)
				{
					drawTooltip(button_moveToFront, mouseX, mouseY);
				}
				if (mouseX > 75 && mouseX < 107)
				{
					drawTooltip(button_camera, mouseX, mouseY);
				}
				if (mouseX > 111 && mouseX < 143)
				{
					drawTooltip(button_levelProperties, mouseX, mouseY);
				}
			}
			if (mouseY > g_window.height - BAR_HEIGHT + 111 && mouseY < g_window.height - BAR_HEIGHT + 143)
				//fourth row of buttons
			{
				if (mouseX > 3 && mouseX < 35)
				{
					drawTooltip(button_copy, mouseX, mouseY);
				}
				if (mouseX > 39 && mouseX < 71)
				{
					drawTooltip(button_paste, mouseX, mouseY);
				}
				if (mouseX > 75 && mouseX < 107)
				{
					drawTooltip(button_delete, mouseX, mouseY);
				}
				if (mouseX > 111 && mouseX < 143)
				{
					drawTooltip(button_quit, mouseX, mouseY);
				}
			}
		}
	}

	SDL_SetRenderTarget(g_window.screen_renderer, NULL);
}

void Bar::drawButton(const buttonInfo & button, buttonState state, int x, int y)
{
	SDL_Rect destRect;
	destRect.x = x;
	destRect.y = y;
	destRect.w = 32;
	destRect.h = 32;
	if (state == off)
	{
		SDL_RenderCopy(g_window.screen_renderer, button.buttonTexUp, NULL, &destRect);
	}
	if (state == on)
	{
		SDL_RenderCopy(g_window.screen_renderer, button.buttonTexDown, NULL, &destRect);
	}
}

void Bar::drawTooltip(const buttonInfo & button, int x, int y)
{
	int tooltipW, tooltipH;
	SDL_QueryTexture(button.tooltip, NULL, NULL, &tooltipW, &tooltipH);

	SDL_Rect tooltipRect;
	tooltipRect.x = x;
	tooltipRect.y = y - tooltipH - 2;
	tooltipRect.w = tooltipW + 2;
	tooltipRect.h = tooltipH + 2;

	SDL_SetRenderDrawColor(g_window.screen_renderer, 200, 200, 200, 255);
	SDL_RenderFillRect(g_window.screen_renderer, &tooltipRect);
	SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(g_window.screen_renderer, &tooltipRect);

	tooltipRect.x++;
	tooltipRect.y++;
	tooltipRect.w -= 2;
	tooltipRect.h -= 2;

	SDL_RenderCopy(g_window.screen_renderer, button.tooltip, NULL, &tooltipRect);
}

void Bar::destroy(void)
{
	destroyButtonTextures(button_layerBackground);
	destroyButtonTextures(button_layerTerrain);
	destroyButtonTextures(button_layerTool);
	destroyButtonTextures(button_layerBackgroundVisible);
	destroyButtonTextures(button_layerTerrainVisible);
	destroyButtonTextures(button_layerToolVisible);
	destroyButtonTextures(button_save);
	destroyButtonTextures(button_moveToBack);
	destroyButtonTextures(button_moveToFront);
	destroyButtonTextures(button_camera);
	destroyButtonTextures(button_levelProperties);
	destroyButtonTextures(button_copy);
	destroyButtonTextures(button_paste);
	destroyButtonTextures(button_delete);
	destroyButtonTextures(button_quit);
}

void Bar::destroyButtonTextures(buttonInfo &button)
{
	if (button.buttonTexUp != NULL)	SDL_DestroyTexture(button.buttonTexUp);
	if (button.buttonTexDown != NULL)	SDL_DestroyTexture(button.buttonTexDown);
	if (button.tooltip != NULL)	SDL_DestroyTexture(button.tooltip);
}
