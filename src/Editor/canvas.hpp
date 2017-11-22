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

#ifndef CANVAS_HPP
#define CANVAS_HPP

#include "SDL.h"

class Window;
class Editor;
class Bar;
class Style;
class Level;

class Canvas
{
public:

	Window * window_ptr;
	Editor * editor_ptr;
	Bar * bar_ptr;
	Style * style_ptr;
	Level * level_ptr;

	int height;

	bool redraw;

	signed int scroll_x, scroll_y;
	signed int scrollOffset_x, scrollOffset_y;
	signed int zoom;
	Sint32 mouse_remainder_x, mouse_remainder_y;
	bool backgroundOnly;

	void setReferences(Window * w, Editor * e, Bar * b, Style * s, Level * l);
	void load(void);
	void resize(int h);

	bool scroll(signed int delta_x, signed int delta_y, bool drag);

	enum zoomType { zoomIn, zoomOut };
	void zoomCanvas(signed int zoomFocusX, signed int zoomFocusY, zoomType zoomDir);

	void draw(void);

	void draw_selection_box(int x, int y, int width, int height);

	void drawHeldObject(int holdingType, int holdingID, int x, int y);

	enum borderType { horizontal, vertical };
	void draw_dashed_level_border(borderType type, int pos, int offset);

	Canvas(void) { /* nothing to do */ }

};

#endif // CANVAS_HPP