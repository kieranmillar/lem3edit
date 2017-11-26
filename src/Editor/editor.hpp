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

#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "bar.hpp"
#include "canvas.hpp"
#include "input.hpp"
#include "../del.hpp"
#include "../level.hpp"
#include "../tribe.hpp"
#include "../window.hpp"

#include "SDL.h"

#include <set>

class Editor
{
public:
	Window * window_ptr;

	Bar bar;
	Canvas canvas;
	Editor_input editor_input;
	
	Del font;
	
	Level level;
	Tribe tribe;
	Style style;
	
	typedef std::set<Level::Object::Index> Selection;
	Selection selection;
	
	typedef std::vector< std::pair<Level::Object::Index, Level::Object> > Clipboard;
	Clipboard clipboard;

	Uint32 gameFrameCount;
	Uint32 gameFrameTick;
	
	void resize(int w, int h);

	bool select( signed int x, signed int y, bool modify );
	bool select_none( void );
	bool select_all( void );

	bool copy_selected(void);
	bool paste(void);
	
	bool addObject(int idToAdd, int typeToAdd, int xToAdd, int yToAdd);

	bool moveToFront(void);
	bool moveToBack(void);

	bool decrease_obj_id(void);
	bool increase_obj_id(void);
	bool delete_selected( void );
	bool move_selected( signed int delta_x, signed int delta_y );
	
	bool load(int n, Window * w);
	bool save(int n);
	
	Editor( void );
	~Editor( void ) { /* nothing to do */ }
	
private:
	Editor( const Editor & );
	Editor & operator=( const Editor & );
};

#endif // EDITOR_HPP