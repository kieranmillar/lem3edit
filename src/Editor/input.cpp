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
This file handles the user inputs (mouse and kepyboard) of the editor
*/

#include "input.hpp"

#include "bar.hpp"
#include "canvas.hpp"
#include "editor.hpp"
#include "../lem3edit.hpp"
#include "../level.hpp"
#include "../style.hpp"
#include "../window.hpp"

void Editor_input::setReferences(Window * w, Editor * e, Bar * b, Canvas * c, Style * s, Level * l)
{
	window_ptr = w;
	editor_ptr = e;
	bar_ptr = b;
	canvas_ptr = c;
	style_ptr = s;
	level_ptr = l;
}

void Editor_input::load(void)
{
	mouse_prev_x = mouse_prev_y = 0;
}

void Editor_input::handleEvents(SDL_Event event)
{
	unsigned int delta_multiplier = (SDL_GetModState() & KMOD_SHIFT) ? 8 : 2;

	Sint32 mouse_x_window, mouse_y_window;
	Uint8 mouse_state = SDL_GetMouseState(&mouse_x_window, &mouse_y_window);
	// Due to zooming we can't rely on the mouse's position in the window to map correctly
	// So we need to create our own variables instead
	// These ones give the true x co-ordinate of the level, ignoring zoom and scroll
	Sint32 mouse_x, mouse_y;
	mouse_x = (mouse_x_window / canvas_ptr->zoom) + canvas_ptr->scroll_x;
	mouse_y = (mouse_y_window / canvas_ptr->zoom) + canvas_ptr->scroll_y;

	switch (event.type)
	{
	case SDL_WINDOWEVENT:
	{
		SDL_WindowEvent &e = event.window;

		if (e.event == SDL_WINDOWEVENT_RESIZED)
		{
			window_ptr->width = e.data1;
			window_ptr->height = e.data2;
			window_ptr->resize();
			editor_ptr->resize(e.data1, e.data2);
			canvas_ptr->redraw = true;
		}
	}
	case SDL_MOUSEMOTION:
	{
		SDL_MouseMotionEvent &e = event.motion;

		if (e.state & SDL_BUTTON(SDL_BUTTON_LEFT))
			editor_ptr->move_selected(mouse_x_window - mouse_prev_x, mouse_y_window - mouse_prev_y);

		mouse_prev_x = mouse_x_window;
		mouse_prev_y = mouse_y_window;

		break;
	}
	case SDL_MOUSEBUTTONDOWN://when initially pressed
	{
		SDL_MouseButtonEvent &e = event.button;
		bool ctrl_down = SDL_GetModState() & KMOD_CTRL;

		if (e.button == SDL_BUTTON_LEFT)
		{
			canvas_ptr->mouse_remainder_x = 0;
			canvas_ptr->mouse_remainder_y = 0;
		}

		if (e.button == SDL_BUTTON_LEFT || e.button == SDL_BUTTON_RIGHT)
			editor_ptr->select(mouse_x, mouse_y, ctrl_down);

		break;
	}
	case SDL_MOUSEBUTTONUP://when released
	{
		SDL_MouseButtonEvent &e = event.button;

		if (e.button == SDL_BUTTON_LEFT)
			canvas_ptr->mouse_remainder_x = canvas_ptr->mouse_remainder_y = 0;

		break;
	}
	case SDL_MOUSEWHEEL:
	{
		SDL_MouseWheelEvent &e = event.wheel;
		if (!mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT))
		{
			if (e.y == 1)
			{
				if (canvas_ptr->zoom != 16)
					canvas_ptr->zoom *= 2;
				canvas_ptr->redraw = true;
			}
			if (e.y == -1)
			{
				if (canvas_ptr->zoom != 1)
					canvas_ptr->zoom /= 2;
				canvas_ptr->redraw = true;
			}
		}
		break;
	}
	case SDL_KEYDOWN:
	{
		SDL_KeyboardEvent &e = event.key;
		bool alt_down = e.keysym.mod & KMOD_ALT;
		bool ctrl_down = e.keysym.mod & KMOD_CTRL;

		Uint8 mouse_state = SDL_GetMouseState(NULL, NULL);

		switch (e.keysym.sym)
		{
		case SDLK_1:
			bar_ptr->changeType(PERM);
			break;
		case SDLK_2:
			bar_ptr->changeType(TEMP);
			break;
		case SDLK_s:
			editor_ptr->save(level_ptr->level_id);
			break;
		case SDLK_ESCAPE:
			editor_ptr->select_none();
			break;
		case SDLK_a:
			editor_ptr->select_all();
			break;
		case SDLK_c:
			if (ctrl_down) {
				editor_ptr->copy_selected();
			}
			break;
		case SDLK_v:
			if (ctrl_down) {
				editor_ptr->paste();
			}
			break;
		case SDLK_z:
			//editor.decrease_obj_id();
			bar_ptr->scroll(-25 * delta_multiplier);
			break;
		case SDLK_x:
			//editor.increase_obj_id();
			bar_ptr->scroll(25 * delta_multiplier);
			break;
		case SDLK_UP:
		case SDLK_DOWN:
			if (!mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT))
				editor_ptr->move_selected(0, (e.keysym.sym == SDLK_UP ? -1 : e.keysym.sym == SDLK_DOWN ? 1 : 0) * delta_multiplier * canvas_ptr->zoom);
			break;
		case SDLK_LEFT:
		case SDLK_RIGHT:
			if (!mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT))
				editor_ptr->move_selected((e.keysym.sym == SDLK_LEFT ? -4 : e.keysym.sym == SDLK_RIGHT ? 4 : 0) * delta_multiplier * canvas_ptr->zoom, 0);
			break;
		case SDLK_DELETE:
			editor_ptr->delete_selected();
			break;
		case SDLK_q:
			die();
			break;
		case SDLK_b:
			if (canvas_ptr->backgroundOnly)
			{
				canvas_ptr->backgroundOnly = false;
			}
			else
			{
				canvas_ptr->backgroundOnly = true;
			}

			canvas_ptr->redraw = true;
			break;
		default:
			break;
		}

		break;
	}
	case SDL_USEREVENT:// stuff here happens every frame
	{

		const Uint8 *key_state = SDL_GetKeyboardState(NULL);

		{ // scroll if ijkl or mouse at border
			const int mouse_scroll_trigger = std::min(window_ptr->width, window_ptr->height) / 32;

			const signed int left = (mouse_x_window < mouse_scroll_trigger) ? /*2*/0 : 0 + key_state[SDL_GetScancodeFromKey(SDLK_j)] ? 1 : 0;
			const signed int right = (mouse_x_window >= (signed)window_ptr->width - mouse_scroll_trigger) ? /*2*/0 : 0 + key_state[SDL_GetScancodeFromKey(SDLK_l)] ? 1 : 0;
			signed int delta_x = (-left + right) * delta_multiplier;

			const signed int up = (mouse_y_window < mouse_scroll_trigger) ? /*2*/0 : 0 + key_state[SDL_GetScancodeFromKey(SDLK_i)] ? 1 : 0;
			const signed int down = (mouse_y_window >= (signed)window_ptr->height - mouse_scroll_trigger) ? /*2*/0 : 0 + key_state[SDL_GetScancodeFromKey(SDLK_k)] ? 1 : 0;
			signed int delta_y = (-up + down) * delta_multiplier;


			editor_ptr->scroll(delta_x, delta_y, mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT));

		}

		canvas_ptr->draw();

		break;
	}
	default:
		break;
	}
}