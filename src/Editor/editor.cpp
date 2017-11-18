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
This file includes all the primary functionality of the level editor
*/
#include "editor.hpp"

#include <cassert>
#include <iostream>
using namespace std;

Editor::Editor(void)
	: scroll_x(0), scroll_y(0), zoom(2), backgroundOnly(false),
	mouse_remainder_x(0), mouse_remainder_y(0)
{
	font.load("FONT");
}

bool Editor::load( int n, Window window )
{
	level.load(n);
	tribe.load(level.tribe);
	style.load(level.style, window, tribe.palette);
	
	return redraw = true;
}

bool Editor::save(int n)
{
	level.save(n);

	return redraw = true;
}

bool Editor::select( signed int x, signed int y, bool modify_selection )
{
	Level::Object::Index temp = level.get_object_by_position(x, y, style, backgroundOnly);
	
	if (temp.i == -1)  // selected nothing
	{
		selection.clear();
	}
	else
	{
		bool already_selected = selection.find(temp) != selection.end();
		
		if (!modify_selection && !already_selected)
			selection.clear();
		if (modify_selection && already_selected)
			selection.erase(temp);
		else
			selection.insert(temp);
	}
	
	return redraw = true;
}

bool Editor::select_none( void )
{
	if (selection.empty())
		return false;
	
	selection.clear();
	
	return redraw = true;
}

bool Editor::select_all( void )
{
	for (unsigned int type = 0; type < COUNTOF(level.object); ++type)
	{
		for (unsigned int i = 0; i < level.object[type].size(); ++i)
		{
			selection.insert(Level::Object::Index(type, i));
		}
	}
	
	return redraw = !selection.empty();
}

bool Editor::copy_selected( void )
{
	clipboard.clear();
	
	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		clipboard.push_back(pair<Level::Object::Index, Level::Object>(*i, level.object[i->type][i->i]));
	}
	
	return !clipboard.empty();
}

bool Editor::paste( void )
{
	selection.clear(); // maybe delete selection instead?
	
	for (Clipboard::const_iterator i = clipboard.begin(); i != clipboard.end(); ++i)
	{
		const Level::Object::Index &index = i->first;
		
		level.object[index.type].push_back(i->second);
		
		selection.insert(Level::Object::Index(index.type, level.object[index.type].size() - 1));
	}
	
	return redraw = !clipboard.empty();
}

bool Editor::decrease_obj_id( void )
{
	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		Level::Object &o = level.object[i->type][i->i];
		o.id = style.object_prev_id(i->type, o.id);
	}

	return redraw = true;
}

bool Editor::increase_obj_id( void )
{
	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		Level::Object &o = level.object[i->type][i->i];
		o.id = style.object_next_id(i->type, o.id);
	}

	return redraw = true;
}

bool Editor::delete_selected( void )
{
	if (selection.empty())
		return false;
	
	for (Selection::const_reverse_iterator i = selection.rbegin(); i != selection.rend(); ++i)
		level.object[i->type].erase(level.object[i->type].begin() + i->i);
	
	selection.clear();
	
	return redraw = true;
}

//This function takes in how much the mouse has moved in the last frame
//Input is independent of zoom, so provide true pixel values
//So to ensure smooth scrolling, it maintains a remainder after relocating the moved objects
//and snapping them to the grid
bool Editor::move_selected( signed int delta_x, signed int delta_y )
{
	delta_x += mouse_remainder_x;
	delta_y += mouse_remainder_y;
	mouse_remainder_x = delta_x % (8*zoom);
	mouse_remainder_y = delta_y % (2*zoom);

	delta_x /= (8*zoom);
	delta_y /= (2*zoom);
	
	if (selection.empty() || (delta_x == 0 && delta_y == 0))
		return false;
	
	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		Level::Object &o = level.object[i->type][i->i];
		o.x += delta_x*8;
		o.y += delta_y*2;
	}
	
	return redraw = true;
}

bool Editor::move_selected_z( signed int delta_z )
{
	if (selection.empty())
		return false;
	
	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		assert(false); // TODO
		(void)delta_z;
	}
	
	return redraw = true;
}

bool Editor::scroll( signed int delta_x, signed int delta_y, bool drag )
{
	signed int old_scroll_x = scroll_x, old_scroll_y = scroll_y;
	
	scroll_x = BETWEEN(-200, scroll_x + delta_x, level.width);
	delta_x = scroll_x - old_scroll_x;
	
	scroll_y = BETWEEN(-200, scroll_y + delta_y, level.height);
	delta_y = scroll_y - old_scroll_y;
	
	if (delta_x != 0 || delta_y != 0)
	{
		if (drag)
			move_selected(delta_x*zoom, delta_y*zoom);
		
		return redraw = true;
	}
	
	return false;
}

void Editor::draw( Window window )
{
	
	SDL_Rect level_area;
	level_area.x = (0 - scroll_x)*zoom;
	level_area.y = (0 - scroll_y)*zoom;
	level_area.w = level.width*zoom;
	level_area.h = level.height*zoom;

	if (redraw)
	{
		SDL_SetRenderDrawBlendMode(window.screen_renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderTarget(window.screen_renderer, window.screen_texture);
		SDL_SetRenderDrawColor(window.screen_renderer, 100, 100, 100, 255);
		SDL_RenderFillRect(window.screen_renderer, NULL);

		SDL_SetRenderDrawColor(window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(window.screen_renderer, &level_area);

		level.draw(window, scroll_x, scroll_y, style, backgroundOnly, zoom);

		for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
		{
			Level::Object &o = level.object[i->type][i->i];
			int so = style.object_by_id(i->type, o.id);
			draw_selection_box(window.screen_renderer, (o.x - scroll_x)*zoom, (o.y - scroll_y)*zoom, style.object[i->type][so].width * 8 * zoom, style.object[i->type][so].height * 2 * zoom);
		}

		SDL_SetRenderDrawColor(window.screen_renderer, 0, 0, 0, 255);
		SDL_SetRenderTarget(window.screen_renderer, NULL);

		redraw = false;
	}

	SDL_RenderCopy(window.screen_renderer, window.screen_texture, NULL, NULL);
	SDL_RenderPresent(window.screen_renderer);

}

void Editor::draw_selection_box(SDL_Renderer *renderer, int x, int y, int width, int height)
{
	SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255/8);
	SDL_Rect r;
	r.x = x - 1;
	r.y = y - 1;
	r.w = width + 1;
	r.h = height + 1;
	SDL_RenderFillRect(renderer, &r);
	SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
	SDL_RenderDrawRect(renderer, &r);
}