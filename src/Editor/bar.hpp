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
#define PIECESIZE 132

#include "SDL.h"

class Window;
class Editor;
class Style;

class Bar
{
public:

	Window * window_ptr;
	Editor * editor_ptr;
	Style * style_ptr;

	int barScrollX;
	int barPERMCount;
	int barMaxPERM;
	int barTEMPCount;
	int barMaxTEMP;
	SDL_Rect barScrollRect;

	void load(Window * w, Editor * e, Style * s);

	void resizeBarScrollRect(int windowWidth, int windowHeight);
	void scroll(signed int moveAmount);
	void updateBarScrollPos(int xPos);

	void draw( void );

	Bar(void) { /* nothing to do */ }

};

#endif // EDITOR_HPP