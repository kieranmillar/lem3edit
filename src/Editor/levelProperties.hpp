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

#ifndef LEVELPROPERTIES_HPP
#define LEVELPROPERTIES_HPP

#include "SDL.h"
#include "SDL_ttf.h"

class Window;
class Editor;
class Bar;
class Canvas;
class Level;

class LevelProperties
{
public:

	Window * window_ptr;
	Editor * editor_ptr;
	Bar * bar_ptr;
	Canvas * canvas_ptr;
	Level * level_ptr;

	int releaseRate;
	int spawnDelay;
	int timeLimitMins;
	int timeLimitSecs;

	bool redraw;

	void setReferences(Window * w, Editor * e, Bar * b, Canvas * c, Level * l);

	void openDialog(void);
	void closeDialog(bool saveChanges);

	void handleLevelPropertiesEvents(SDL_Event event);

	void draw(void);

	LevelProperties(void) { /* nothing to do */ }

};

#endif // LEVELPROPERTIES_HPP