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

#ifndef BAR_HPP
#define BAR_HPP

#define BAR_HEIGHT 150
#define PANEL_WIDTH 150
#define PIECESIZE 132

#include "SDL.h"

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

	void setReferences(Window * w, Editor * e, Canvas * c, Style * s);
	void load(void);

	void resizeBarScrollRect(int windowWidth, int windowHeight);
	void scroll(signed int moveAmount);
	void updateBarScrollPos(int xPos);
	void moveScrollBar(int moveLocationInWindow);

	void changeType( int t);

	int getPieceIDByScreenPos ( int mousePos );

	void draw( void );

	Bar(void) { /* nothing to do */ };

};

#endif // EDITOR_HPP