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
	scroll_x = 0;
	scroll_y = 0;
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

void Canvas::draw()
{

	SDL_Rect level_area;
	level_area.x = 0 - scroll_x * zoom;
	level_area.y = 0 - scroll_y * zoom;
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

		level_ptr->draw(window_ptr, scroll_x, scroll_y, editor_ptr->style, backgroundOnly, zoom);

		for (Editor::Selection::const_iterator i = editor_ptr->selection.begin(); i != editor_ptr->selection.end(); ++i)
		{
			Level::Object &o = level_ptr->object[i->type][i->i];
			int so = style_ptr->object_by_id(i->type, o.id);
			editor_ptr->draw_selection_box((o.x - scroll_x)*zoom, (o.y - scroll_y)*zoom, style_ptr->object[i->type][so].width * 8 * zoom, style_ptr->object[i->type][so].height * 2 * zoom);
		}

		SDL_SetRenderDrawColor(window_ptr->screen_renderer, 0, 0, 0, 255);

		//Draw dotted lines on the level border
		if (scroll_x < 0 && (scroll_x * zoom) + window_ptr->width > 0)
		{
			editor_ptr->draw_dashed_level_border(Editor::vertical, ((0 - scroll_x) * zoom) - 1, scroll_y * zoom);
		}
		if (scroll_x < level_ptr->width && scroll_x + (window_ptr->width / zoom) > level_ptr->width)
		{
			editor_ptr->draw_dashed_level_border(Editor::vertical, ((level_ptr->width - scroll_x) * zoom), scroll_y * zoom);
		}
		if (scroll_y < 0 && (scroll_y * zoom) + window_ptr->height > 0)
		{
			editor_ptr->draw_dashed_level_border(Editor::horizontal, ((0 - scroll_y) * zoom) - 1, scroll_x * zoom);
		}
		if (scroll_y < level_ptr->height && scroll_y + (window_ptr->height / zoom) > level_ptr->height)
		{
			editor_ptr->draw_dashed_level_border(Editor::horizontal, ((level_ptr->height - scroll_y) * zoom), scroll_x * zoom);
		}

		SDL_SetRenderTarget(window_ptr->screen_renderer, NULL);

		redraw = false;

	}

	bar_ptr->draw();

	SDL_SetRenderTarget(window_ptr->screen_renderer, NULL);
	SDL_RenderCopy(window_ptr->screen_renderer, window_ptr->screen_texture, NULL, NULL);
	SDL_RenderPresent(window_ptr->screen_renderer);


}