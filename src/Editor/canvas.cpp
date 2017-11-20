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
This file includes code to manage drawing in the editor, and the properties fo the part of the screen
where the level is displayed
*/

#include "bar.hpp"
#include "canvas.hpp"
#include "../window.hpp"

void Canvas::setReferences(Window * w, Editor * e, Bar * b, Style * s)
{
	window_ptr = w;
	editor_ptr = e;
	bar_ptr = b;
	style_ptr = s;
}

void Canvas::load(void)
{
	resize(window_ptr->height);
}

void Canvas::resize(int h)
{
	height = h - BAR_HEIGHT;
}