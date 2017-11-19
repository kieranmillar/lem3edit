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

	bool redraw;
	
	void draw();
	
	Del font;
	
	Level level;
	Tribe tribe;
	Style style;
	
	signed int scroll_x, scroll_y;
	signed int zoom;
	Sint32 mouse_remainder_x, mouse_remainder_y;
	bool backgroundOnly;
	
	typedef std::set<Level::Object::Index> Selection;
	Selection selection;
	
	typedef std::vector< std::pair<Level::Object::Index, Level::Object> > Clipboard;
	Clipboard clipboard;
	
	bool select( signed int x, signed int y, bool modify );
	bool select_none( void );
	bool select_all( void );
	
	bool copy_selected( void );
	bool paste( void );
	bool decrease_obj_id(void);
	bool increase_obj_id(void);
	bool delete_selected( void );
	bool move_selected( signed int delta_x, signed int delta_y );
	bool move_selected_z( signed int delta_z );
	
	bool scroll( signed int delta_x, signed int delta_y, bool drag );
	
	bool load(int n, Window * w);
	bool save(int n);

	void draw_selection_box(int x, int y, int width, int height);

	enum borderType { horizontal, vertical };
	void draw_dashed_level_border(borderType type, int pos, int offset);
	
	Editor( void );
	~Editor( void ) { /* nothing to do */ }
	
private:
	Editor( const Editor & );
	Editor & operator=( const Editor & );
};

#endif // EDITOR_HPP