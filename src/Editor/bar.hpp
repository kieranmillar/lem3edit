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

#ifndef BAR_HPP
#define BAR_HPP

#define BAR_HEIGHT 150
#define PANEL_WIDTH 150
#define PIECESIZE 132

#include "SDL.h"
#include "SDL_ttf.h"

class Window;
class Editor;
class Canvas;
class Style;

class Bar
{
public:

	Window * window_ptr;
	Editor * editor_ptr;
	Canvas * canvas_ptr;
	Style * style_ptr;

	int barScrollX;
	int barTypeCount[3];
	int barTypeMax[3];
	int type;
	SDL_Rect barScrollRect;

	struct buttonInfo { SDL_Texture * buttonTexUp; SDL_Texture * buttonTexDown; SDL_Texture * tooltip; };
	enum buttonState { on, off };

	buttonInfo button_layerBackground;
	buttonInfo button_layerTerrain;
	buttonInfo button_layerTool;
	buttonInfo button_layerBackgroundVisible;
	buttonInfo button_layerTerrainVisible;
	buttonInfo button_layerToolVisible;
	buttonInfo button_save;
	buttonInfo button_moveToBack;
	buttonInfo button_moveToFront;
	buttonInfo button_camera;
	buttonInfo button_levelProperties;
	buttonInfo button_copy;
	buttonInfo button_paste;
	buttonInfo button_delete;
	buttonInfo button_quit;

	TTF_Font * tooltipFont;

	void setReferences(Window * w, Editor * e, Canvas * c, Style * s);
	void load(void);
	bool loadButtonGraphic(buttonInfo & button, const char * filePathUp, const char * filePathDown);
	bool setButtonTooltip(buttonInfo & button, const char * text);

	void resizeBarScrollRect(int windowWidth, int windowHeight);
	void scroll(signed int moveAmount);
	void updateBarScrollPos(int xPos);
	void moveScrollBar(int moveLocationInWindow);

	void changeType(int t);

	int getPieceIDByScreenPos(int mousePos);

	void draw(int mouseX, int mouseY);

	void drawButton(const buttonInfo & button, buttonState state, int x, int y);
	void drawTooltip(const buttonInfo & button, int x, int y);

	Bar(void) { /* nothing to do */ };

	void destroy(void);
	void destroyButtonTextures(buttonInfo & button);
};

#endif // EDITOR_HPP
