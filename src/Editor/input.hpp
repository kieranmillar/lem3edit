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

#ifndef EDITORINPUT_HPP
#define EDITORINPUT_HPP

#include "SDL.h"

class Window;
class Editor;
class Bar;
class Canvas;
class Style;
class Level;

class Editor_input
{
public:

	Window * window_ptr;
	Editor * editor_ptr;
	Bar * bar_ptr;
	Canvas * canvas_ptr;
	Style * style_ptr;
	Level * level_ptr;

	bool redraw;

	int mouse_x, mouse_y;
	Sint32 mouse_prev_x, mouse_prev_y;

	bool dragging;
	Uint32 startDragTime;

	bool leftScrollButtonHolding, rightScrollButtonHolding;
	bool scrollBarHolding, scrollBarShifting;
	int scrollBarHoldingOffset;

	int holdingID, holdingType;

	bool movingView;

	enum whichBorder {none, top, bottom, left, right};
	bool resizingLevel;
	int resizingNewPos;
	whichBorder resizingWhich;

	void setReferences(Window * w, Editor * e, Bar * b, Canvas * c, Style * s, Level * l);
	void load(void);

	void handleEvents(SDL_Event event);

	Editor_input(void) { /* nothing to do */ }

};

#endif // EDITORINPUT_HPP