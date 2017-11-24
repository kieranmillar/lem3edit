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
#include "input.hpp"
#include "../level.hpp"
#include "../style.hpp"
#include "../window.hpp"

void Canvas::setReferences(Window * w, Editor * e, Editor_input * i, Bar * b, Style * s, Level * l)
{
	window_ptr = w;
	editor_ptr = e;
	input_ptr = i;
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

	bool up = false;
	bool down = false;
	bool left = false;
	bool right = false;

	if (delta_x < 0)
		left = true;
	if (delta_x > 0)
		right = true;
	if (delta_y < 0)
		up = true;
	if (delta_y > 0)
		down = true;
	scrollOffset_x += delta_x;
	scrollOffset_y += delta_y;

	scroll_x += scrollOffset_x / zoom;
	scroll_y += scrollOffset_y / zoom;

	scrollOffset_x %= zoom;
	scrollOffset_y %= zoom;

	if (scroll_x <= 0 - window_ptr->width && left)
	{
		scroll_x = 0 - window_ptr->width;
		scrollOffset_x = 0;
		delta_x = 0;
	}
	if (scroll_x >= level_ptr->width + ((window_ptr->width / zoom) * (zoom - 1)) && right)
	{
		scroll_x = level_ptr->width + ((window_ptr->width / zoom) * (zoom - 1));
		scrollOffset_x = 0;
		delta_x = 0;
	}
	if (scroll_y <= 0 - height && up)
	{
		scroll_y = 0 - height;
		scrollOffset_y = 0;
		delta_y = 0;
	}
	if (scroll_y >= level_ptr->height + ((height / zoom) * (zoom - 1)) && down)
	{
		scroll_y = level_ptr->height + ((height / zoom) * (zoom - 1));
		scrollOffset_y = 0;
		delta_y = 0;
	}

	if (delta_x != 0 || delta_y != 0)
	{
		if (drag)
			editor_ptr->move_selected(delta_x, delta_y);
	}

	return redraw = true;
}

// x and y values are real values (so zoom independent)
void Canvas::zoomCanvas(signed int zoomFocusX, signed int zoomFocusY, zoomType zoomDir)
{
	int moveDiff_x = zoomFocusX - scroll_x;
	int moveDiff_y = zoomFocusY - scroll_y;

	if (zoomDir == zoomIn)
	{
		if (zoom == 16)
			return;
		zoom *= 2;
		scrollOffset_x *= 2;
		scrollOffset_y *= 2;
		moveDiff_x /= 2;
		moveDiff_y /= 2;
	}
	if (zoomDir == zoomOut)
	{
		if (zoom == 1)
			return;
		zoom /= 2;
		scrollOffset_x /= 2;
		scrollOffset_y /= 2;
		moveDiff_x = -moveDiff_x;
		moveDiff_y = -moveDiff_y;
	}

	scroll(moveDiff_x * zoom, moveDiff_y * zoom, false);
	redraw = true;
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

		//START - Draw dotted lines on the level border
		enum whichBorder { none, top, bottom, left, right };
		whichBorder hoverBorder = none;
		if (input_ptr->resizingLevel == false) // only highlight borders on hover if not resizing
		{
			if (input_ptr->mouse_x <= 8 && input_ptr->mouse_x >= -8)
			{
				hoverBorder = left;
			}
			else if (input_ptr->mouse_x >= level_ptr->width - 8 && input_ptr->mouse_x <= level_ptr->width + 8)
			{
				hoverBorder = right;
			}
			else if (input_ptr->mouse_y <= 8 && input_ptr->mouse_y >= -8)
			{
				hoverBorder = top;
			}
			else if (input_ptr->mouse_y >= level_ptr->height - 8 && input_ptr->mouse_y <= level_ptr->height + 8)
			{
				hoverBorder = bottom;
			}
		}
		bool highlight = false;
		if (scroll_x <= 0 && (scroll_x * zoom) + window_ptr->width - 1 >= 0)
		{
			if (input_ptr->resizingLevel == false || input_ptr->resizingWhich != Editor_input::whichBorder::left)
			{
				if (hoverBorder == left)
					highlight = true;
				draw_dashed_level_border(vertical, ((0 - scroll_x) * zoom) - scrollOffset_x - 1, scroll_y * zoom + scrollOffset_y, highlight);
				highlight = false;
			}
		}
		if (scroll_x <= level_ptr->width && scroll_x + (window_ptr->width / zoom) - 1 >= level_ptr->width)
		{
			if (input_ptr->resizingLevel == false || input_ptr->resizingWhich != Editor_input::whichBorder::right)
			{
				if (hoverBorder == right)
					highlight = true;
				draw_dashed_level_border(vertical, ((level_ptr->width - scroll_x) * zoom) - scrollOffset_x, scroll_y * zoom + scrollOffset_y, highlight);
				highlight = false;
			}
		}
		if (scroll_y <= 0 && (scroll_y * zoom) + window_ptr->height - 1 >= 0)
		{
			if (input_ptr->resizingLevel == false || input_ptr->resizingWhich != Editor_input::whichBorder::top)
			{
				if (hoverBorder == top)
					highlight = true;
				draw_dashed_level_border(horizontal, ((0 - scroll_y) * zoom) - 1 - scrollOffset_y, scroll_x * zoom + scrollOffset_x, highlight);
				highlight = false;
			}
		}
		if (scroll_y <= level_ptr->height && scroll_y + (window_ptr->height / zoom) - 1 >= level_ptr->height)
		{
			if (input_ptr->resizingLevel == false || input_ptr->resizingWhich != Editor_input::whichBorder::bottom)
			{
				if (hoverBorder == bottom)
					highlight = true;
				draw_dashed_level_border(horizontal, ((level_ptr->height - scroll_y) * zoom) - scrollOffset_y, scroll_x * zoom + scrollOffset_x, highlight);
				highlight = false;
			}
		}
		if (input_ptr->resizingLevel) // draw the one border we are currently resizing
		{
			if (input_ptr->resizingWhich == Editor_input::whichBorder::left || input_ptr->resizingWhich == Editor_input::whichBorder::right)
				draw_dashed_level_border(vertical, (input_ptr->resizingNewPos - scroll_x) * zoom - scrollOffset_x, scroll_y * zoom + scrollOffset_y, true);
			if (input_ptr->resizingWhich == Editor_input::whichBorder::top || input_ptr->resizingWhich == Editor_input::whichBorder::bottom)
				draw_dashed_level_border(horizontal, (input_ptr->resizingNewPos - scroll_y) * zoom - scrollOffset_y, scroll_x * zoom + scrollOffset_x, true);
		}

		//END - Draw dotted lines on the level border

		SDL_SetRenderTarget(window_ptr->screen_renderer, NULL);

		redraw = false;
	}

}

void Canvas::draw_selection_box(int x, int y, int w, int h)
{
	if (x > window_ptr->width || y > height || (x + w) < 0 || (y + h) < 0)
		return;
	SDL_SetRenderDrawColor(window_ptr->screen_renderer, 255, 0, 255, 255 / 8);
	SDL_Rect r;
	r.x = x - 1;
	r.y = y - 1;
	r.w = w + 1;
	r.h = h + 1;
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

void Canvas::draw_dashed_level_border(borderType type, int pos, int offset, bool highlight)
{
	//We have an offset so the lines don't scroll out of synch with the view when scrolling
	int initialOffset = offset % 20;
	int rendColour = 200;
	if (highlight)
		rendColour = 250;
	SDL_SetRenderDrawColor(window_ptr->screen_renderer, rendColour, rendColour, rendColour, 255);
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