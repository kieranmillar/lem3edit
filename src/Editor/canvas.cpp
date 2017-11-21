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
This file includes code to manage drawing in the editor, and the properties of the part of the screen
where the level is displayed
*/

#include "canvas.hpp"

#include "bar.hpp"
#include "editor.hpp"
#include "../level.hpp"
#include "../style.hpp"
#include "../window.hpp"

void Canvas::setReferences(Window * w, Editor * e, Bar * b, Style * s, Level * l)
{
	window_ptr = w;
	editor_ptr = e;
	bar_ptr = b;
	style_ptr = s;
	level_ptr = l;
}

void Canvas::load(void)
{
	scroll_x = scroll_y = 0;
	scrollOffset_x = scrollOffset_y = 0;
	zoom = 2;
	backgroundOnly = false;
	mouse_remainder_x = 0;
	mouse_remainder_y = 0;
	resize(window_ptr->height);
}

void Canvas::resize(int h)
{
	height = h - BAR_HEIGHT;
}

// Moves the view
// Measured in screen pixels (so zoom dependent)
bool Canvas::scroll(signed int delta_x, signed int delta_y, bool drag)
{
	scrollOffset_x += delta_x;
	scrollOffset_y += delta_y;

	signed int unzoomedMove_x = floor(scrollOffset_x / zoom);
	signed int unzoomedMove_y = floor(scrollOffset_y / zoom);

	/*if (scrollOffset_x < 0)
		unzoomedMove_x -= 1;
	if (scrollOffset_y < 0)
		unzoomedMove_y -= 1;*/

	scrollOffset_x %= zoom;
	scrollOffset_y %= zoom;

	scroll_x = BETWEEN(-200, scroll_x + unzoomedMove_x, level_ptr->width);

	scroll_y = BETWEEN(-200, scroll_y + unzoomedMove_y, level_ptr->height);

	if (delta_x != 0 || delta_y != 0)
	{
		if (drag)
			editor_ptr->move_selected(delta_x, delta_y);

		return redraw = true;
	}

	return true;
}

void Canvas::draw()
{

	SDL_Rect level_area;
	level_area.x = 0 - (scroll_x * zoom) - scrollOffset_x;
	level_area.y = 0 - (scroll_y * zoom) - scrollOffset_y;
	level_area.w = level_ptr->width*zoom;
	level_area.h = level_ptr->height*zoom;

	if (redraw)
	{
		SDL_SetRenderDrawBlendMode(window_ptr->screen_renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderTarget(window_ptr->screen_renderer, window_ptr->screen_texture);
		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 100, 100, 100, 255);
		SDL_RenderFillRect(window_ptr->screen_renderer, NULL);

		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(window_ptr->screen_renderer, &level_area);

		level_ptr->draw(window_ptr, scroll_x, scrollOffset_x, scroll_y, scrollOffset_y, editor_ptr->style, backgroundOnly, zoom);

		for (Editor::Selection::const_iterator i = editor_ptr->selection.begin(); i != editor_ptr->selection.end(); ++i)
		{
			Level::Object &o = level_ptr->object[i->type][i->i];
			int so = style_ptr->object_by_id(i->type, o.id);
			draw_selection_box((o.x - scroll_x)*zoom - scrollOffset_x, (o.y - scroll_y)*zoom - scrollOffset_y, style_ptr->object[i->type][so].width * 8 * zoom, style_ptr->object[i->type][so].height * 2 * zoom);
		}

		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 0, 0, 0, 255);

		//Draw dotted lines on the level border
		if (scroll_x <= 0 && (scroll_x * zoom) + window_ptr->width - 1 >= 0)
		{
			draw_dashed_level_border(vertical, ((0 - scroll_x) * zoom) - scrollOffset_x - 1, scroll_y * zoom + scrollOffset_y);
		}
		if (scroll_x <= level_ptr->width && scroll_x + (window_ptr->width / zoom) - 1 >= level_ptr->width)
		{
			draw_dashed_level_border(vertical, ((level_ptr->width - scroll_x) * zoom) - scrollOffset_x, scroll_y * zoom + scrollOffset_y);
		}
		if (scroll_y <= 0 && (scroll_y * zoom) + window_ptr->height - 1 >= 0)
		{
			draw_dashed_level_border(horizontal, ((0 - scroll_y) * zoom) - 1 - scrollOffset_y, scroll_x * zoom + scrollOffset_x);
		}
		if (scroll_y <= level_ptr->height && scroll_y + (window_ptr->height / zoom) - 1 >= level_ptr->height)
		{
			draw_dashed_level_border(horizontal, ((level_ptr->height - scroll_y) * zoom) - scrollOffset_y, scroll_x * zoom + scrollOffset_x);
		}

		SDL_SetRenderTarget(window_ptr->screen_renderer, NULL);

		redraw = false;

	}

}

void Canvas::draw_selection_box(int x, int y, int width, int height)
{
	if (x > window_ptr->width || y > (window_ptr->height - BAR_HEIGHT) || (x + width) < 0 || (x + height) < 0)
		return;
	SDL_SetRenderDrawColor(window_ptr->screen_renderer, 255, 0, 255, 255 / 8);
	SDL_Rect r;
	r.x = x - 1;
	r.y = y - 1;
	r.w = width + 1;
	r.h = height + 1;
	SDL_RenderFillRect(window_ptr->screen_renderer, &r);
	SDL_SetRenderDrawColor(window_ptr->screen_renderer, 255, 0, 255, 255);
	SDL_RenderDrawRect(window_ptr->screen_renderer, &r);
}

void Canvas::drawHeldObject(int holdingType, int holdingID, int x, int y)
{
	int scrollOffsetX, scrollOffsetY, drawX, drawY;
	if (y < height)
	{
		scrollOffsetX = (scroll_x % 8) * zoom;
		scrollOffsetY = (scroll_y % 2) * zoom;
		drawX = x - ((x + scrollOffsetX) % (8 * zoom));
		drawY = y - ((y + scrollOffsetY) % (2 * zoom));
	}
	else
	{
		drawX = x;
		drawY = y;
	}
	int drawID = style_ptr->object_by_id(holdingType, holdingID);
	style_ptr->draw_object_texture(window_ptr, drawX, drawY, holdingType, drawID, zoom, NULL);
}

void Canvas::draw_dashed_level_border(borderType type, int pos, int offset)
{
	//We have an offset so the lines don't scroll out of synch with the view when scrolling
	int initialOffset = offset % 20;
	SDL_SetRenderDrawColor(window_ptr->screen_renderer, 200, 200, 200, 255);
	int end, x1, y1, x2, y2;
	if (type == horizontal)
	{
		end = window_ptr->width;
		x1 = 0 - initialOffset;
		x2 = 10 - initialOffset;
		y1 = pos;
		y2 = pos;
		for (x1; x1 < end; x1 += 20)
		{
			SDL_RenderDrawLine(window_ptr->screen_renderer, x1, y1, x2, y2);
			x2 += 20;
		}
	}
	if (type == vertical)
	{
		end = height;
		x1 = pos;
		x2 = pos;
		y1 = 0 - initialOffset;
		y2 = 10 - initialOffset;
		for (y1; y1 < end; y1 += 20)
		{
			SDL_RenderDrawLine(window_ptr->screen_renderer, x1, y1, x2, y2);
			y2 += 20;
		}
	}
}