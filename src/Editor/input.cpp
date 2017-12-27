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

	dragging = false;
	startDragTime = 0;

	leftScrollButtonHolding = false;
	rightScrollButtonHolding = false;
	scrollBarHolding = false;
	scrollBarHoldingOffset = 0;
	scrollBarShifting = false;

	holdingID = -1;
	holdingType = -1;

	movingView = false;
	
	resizingLevel = false;
	resizingNewPos = 0;
	resizingWhich = none;

	movingCamera = false;
}

void Editor_input::handleEvents(SDL_Event event)
{
	unsigned int delta_multiplier = (SDL_GetModState() & KMOD_SHIFT) ? 4 : 1;

	Sint32 mouse_x_window, mouse_y_window;
	Uint8 mouse_state = SDL_GetMouseState(&mouse_x_window, &mouse_y_window);
	// Due to zooming we can't rely on the mouse's position in the window to map correctly
	// So we need to create our own variables instead
	// These ones give the real x co-ordinate of the level, ignoring zoom and scroll
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

			if (e.state & SDL_BUTTON(SDL_BUTTON_LEFT) && dragging)
			{
				editor_ptr->move_selected(mouse_x_window - mouse_prev_x, mouse_y_window - mouse_prev_y);
			}
			if (e.state & SDL_BUTTON(SDL_BUTTON_LEFT) && scrollBarHolding)
			{
				bar_ptr->moveScrollBar(mouse_x_window - scrollBarHoldingOffset);
			}
			if (e.state & SDL_BUTTON(SDL_BUTTON_RIGHT) && movingView)
			{
				canvas_ptr->scroll(mouse_prev_x - mouse_x_window, mouse_prev_y - mouse_y_window, false);
			}
			if (e.state & SDL_BUTTON(SDL_BUTTON_LEFT) && resizingLevel)
			{
				switch (resizingWhich)
				{
					case (top) :
					{
						resizingNewPos = mouse_y - (mouse_y % 4);
						if (level_ptr->height - resizingNewPos > 400)
							resizingNewPos = level_ptr->height - 400;
						if (level_ptr->height - resizingNewPos < 160)
							resizingNewPos = level_ptr->height - 160;
						break;
					}
					case (bottom) :
					{
						resizingNewPos = mouse_y - (mouse_y % 4);
						if (resizingNewPos > 400)
							resizingNewPos = 400;
						if (resizingNewPos < 160)
							resizingNewPos = 160;
						break;
					}
					case (left) :
					{
						resizingNewPos = mouse_x - (mouse_x % 8);
						if (level_ptr->width - resizingNewPos > 2048)
							resizingNewPos = level_ptr->width - 2048;
						if (level_ptr->width - resizingNewPos < 320)
							resizingNewPos = level_ptr->width - 320;
						break;
					}
					case (right) :
					{
						resizingNewPos = mouse_x - (mouse_x % 8);
						if (resizingNewPos > 2048)
							resizingNewPos = 2048;
						if (resizingNewPos < 320)
							resizingNewPos = 320;
						break;
					}
				}
			}
			if (e.state & SDL_BUTTON(SDL_BUTTON_LEFT) && movingCamera)
			{
				editor_ptr->move_camera(mouse_x_window - mouse_prev_x, mouse_y_window - mouse_prev_y);
			}

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
				if (mouse_y_window < canvas_ptr->height)
					// canvas
				{
					if (holdingType != -1 && holdingID != -1)
					{
						editor_ptr->addObject(holdingID, holdingType, mouse_x - (mouse_x % 8), mouse_y - (mouse_y % 2));

						Level::Object::Index o(holdingType, level_ptr->object[holdingType].size() - 1);
						editor_ptr->selection.insert(o);

						holdingType = -1;
						holdingID = -1;
					}
					else
					{
						if (editor_ptr->startCameraOn == true
							&& mouse_x >= level_ptr->x
							&& mouse_x <= level_ptr->x + 320
							&& mouse_y >= level_ptr->y
							&& mouse_y <= level_ptr->y + 160)
						{
							movingCamera = true;
						}
						else
						{
							bool selectedSomething = editor_ptr->select(mouse_x, mouse_y, ctrl_down);
							if (!selectedSomething)
							{
								// grab level borders
								if (mouse_x <= 8 && mouse_x >= -8)
								{
									resizingLevel = true;
									resizingNewPos = 0;
									resizingWhich = left;
								}
								else if (mouse_x >= level_ptr->width - 8 && mouse_x <= level_ptr->width + 8)
								{
									resizingLevel = true;
									resizingNewPos = level_ptr->width;
									resizingWhich = right;
								}
								else if (mouse_y <= 8 && mouse_y >= -8)
								{
									resizingLevel = true;
									resizingNewPos = 0;
									resizingWhich = top;
								}
								else if (mouse_y >= level_ptr->height - 8 && mouse_y <= level_ptr->height + 8)
								{
									resizingLevel = true;
									resizingNewPos = level_ptr->height;
									resizingWhich = bottom;
								}
							}
						}
					}
				}
				else if (mouse_x_window < PANEL_WIDTH)
					// options panel
				{
					holdingType = -1;
					holdingID = -1;
					if (mouse_y_window > window_ptr->height - BAR_HEIGHT + 3 && mouse_y_window < window_ptr->height - BAR_HEIGHT + 35)
						//first row of buttons
					{
						if (mouse_x_window > 3 && mouse_x_window < 35)
						{
							bar_ptr->changeType(PERM);
						}
						if (mouse_x_window > 39 && mouse_x_window < 71)
						{
							bar_ptr->changeType(TEMP);
						}
						if (mouse_x_window > 75 && mouse_x_window < 107)
						{
							bar_ptr->changeType(TOOL);
						}
						if (mouse_x_window > 111 && mouse_x_window < 143)
						{
							editor_ptr->save(level_ptr->level_id);
						}
					}
					if (mouse_y_window > window_ptr->height - BAR_HEIGHT + 39 && mouse_y_window < window_ptr->height - BAR_HEIGHT + 71)
						//second row of buttons
					{
						if (mouse_x_window > 3 && mouse_x_window < 35)
						{
							canvas_ptr->toggleLayerVisibility(PERM);
						}
						if (mouse_x_window > 39 && mouse_x_window < 71)
						{
							canvas_ptr->toggleLayerVisibility(TEMP);
						}
						if (mouse_x_window > 75 && mouse_x_window < 107)
						{
							canvas_ptr->toggleLayerVisibility(TOOL);
						}
						/*if (mouse_x_window > 111 && mouse_x_window < 143)
						{
							
						}*/
					}
					if (mouse_y_window > window_ptr->height - BAR_HEIGHT + 75 && mouse_y_window < window_ptr->height - BAR_HEIGHT + 107)
						//third row of buttons
					{
						if (mouse_x_window > 3 && mouse_x_window < 35)
						{
							editor_ptr->moveToBack();
						}
						if (mouse_x_window > 39 && mouse_x_window < 71)
						{
							editor_ptr->moveToFront();
						}
						if (mouse_x_window > 75 && mouse_x_window < 107)
						{
							editor_ptr->toggleCameraVisibility();
						}
						/*if (mouse_x_window > 111 && mouse_x_window < 143)
						{
						
						}*/
					}
				}
				else if (mouse_y_window < window_ptr->height - 16)
					// piece browser
				{
					if (holdingType == -1 && holdingID == -1)
					{
						int pieceSelected = bar_ptr->getPieceIDByScreenPos(mouse_x_window);
						if (pieceSelected != -1)
						{
							holdingType = bar_ptr->type;
							holdingID = pieceSelected;
							editor_ptr->select_none();
						}
					}
					else
					{
						holdingType = -1;
						holdingID = -1;
					}
				}
				else if (mouse_x_window < PANEL_WIDTH + 16)
					// piece browser scroll bar left button
				{
					leftScrollButtonHolding = true;
				}
				else if (mouse_x_window > window_ptr->width - 16)
					// piece browser scroll bar right button
				{
					rightScrollButtonHolding = true;
				}
				else if (mouse_x_window > bar_ptr->barScrollRect.x && mouse_x_window < (bar_ptr->barScrollRect.x + bar_ptr->barScrollRect.w))
					// piece browser scroll bar bar
				{
					scrollBarHolding = true;
					scrollBarHoldingOffset = mouse_x_window - bar_ptr->barScrollRect.x;
				}
				else
					// piece browser scroll bar area
				{
					scrollBarShifting = true;
				}
			}
			if (e.button == SDL_BUTTON_RIGHT)
			{
				if (mouse_y_window < canvas_ptr->height)
					// canvas
				{
					movingView = true;
				}
			}
			break;
		}
		case SDL_MOUSEBUTTONUP://when released
		{
			SDL_MouseButtonEvent &e = event.button;

			if (e.button == SDL_BUTTON_LEFT)
			{
				canvas_ptr->mouse_remainder_x = canvas_ptr->mouse_remainder_y = 0;
				leftScrollButtonHolding = false;
				rightScrollButtonHolding = false;
				scrollBarHolding = false;
				scrollBarShifting = false;
				movingCamera = false;
				if (dragging)
				{
					if (startDragTime >= editor_ptr->gameFrameCount - 5) //single-clicked instead of dragging
					{
						editor_ptr->selection.clear();
						Level::Object::Index temp = level_ptr->get_object_by_position(mouse_x, mouse_y, *style_ptr, *canvas_ptr);
						if (temp.i != -1)
							editor_ptr->selection.insert(temp);
					}
					dragging = false;
					startDragTime = 0;
				}
				if (resizingLevel)
				{
					switch (resizingWhich)
					{
						case (top) :
						{
							level_ptr->resizeLevel(0, -resizingNewPos, true);
							canvas_ptr->scroll(0, -resizingNewPos * canvas_ptr->zoom, false);
							break;
						}
						case (bottom) :
						{
							level_ptr->resizeLevel(0, resizingNewPos - level_ptr->height, false);
							break;
						}
						case (left) :
						{
							level_ptr->resizeLevel(-resizingNewPos, 0, true);
							canvas_ptr->scroll(-resizingNewPos * canvas_ptr->zoom, 0, false);
							break;
						}
						case (right) :
						{
							level_ptr->resizeLevel(resizingNewPos - level_ptr->width, 0, false);
							break;
						}
					}
					resizingLevel = false;
					resizingWhich = none;
					resizingNewPos = 0;
					canvas_ptr->redraw = true;
				}
			}
			if (e.button == SDL_BUTTON_RIGHT)
			{
				movingView = false;
			}
			break;
		}
		case SDL_MOUSEWHEEL:
		{
			SDL_MouseWheelEvent &e = event.wheel;
			if (!mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT) && mouse_y_window < canvas_ptr->height)
			{
				if (e.y == 1)
				{
					canvas_ptr->zoomCanvas(mouse_x, mouse_y, canvas_ptr->zoomIn);
				}
				if (e.y == -1)
				{
					canvas_ptr->zoomCanvas(mouse_x, mouse_y, canvas_ptr->zoomOut);
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
				if (ctrl_down)
					canvas_ptr->toggleLayerVisibility(PERM);
				else
					bar_ptr->changeType(PERM);
				break;
			case SDLK_2:
				if (ctrl_down)
					canvas_ptr->toggleLayerVisibility(TEMP);
				else
					bar_ptr->changeType(TEMP);
				break;
			case SDLK_3:
				if (ctrl_down)
					canvas_ptr->toggleLayerVisibility(TOOL);
				else
					bar_ptr->changeType(TOOL);
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
			case SDLK_UP:
			case SDLK_DOWN:
				if (!mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT))
					editor_ptr->move_selected(0, (e.keysym.sym == SDLK_UP ? -2 : e.keysym.sym == SDLK_DOWN ? 2 : 0) * delta_multiplier * canvas_ptr->zoom);
				break;
			case SDLK_LEFT:
			case SDLK_RIGHT:
				if (!mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT))
					editor_ptr->move_selected((e.keysym.sym == SDLK_LEFT ? -8 : e.keysym.sym == SDLK_RIGHT ? 8 : 0) * delta_multiplier * canvas_ptr->zoom, 0);
				break;
			case SDLK_DELETE:
				editor_ptr->delete_selected();
				break;
			case SDLK_COMMA:
				editor_ptr->moveToBack();
				break;
			case SDLK_PERIOD:
				editor_ptr->moveToFront();
				break;
			case SDLK_LEFTBRACKET:
				editor_ptr->decrease_obj_id();
				break;
			case SDLK_RIGHTBRACKET:
				editor_ptr->increase_obj_id();
				break;
			case SDLK_SPACE:
				editor_ptr->toggleCameraVisibility();
				break;
			case SDLK_q:
				die();
				break;
			default:
				break;
			}

			break;
		}
		case SDL_USEREVENT:// stuff here happens every frame. Watch out, timer produces events on a separate thread to rest of program!
		{

			const Uint8 *key_state = SDL_GetKeyboardState(NULL);

			{ // canvas scroll

				const signed int left = key_state[SDL_GetScancodeFromKey(SDLK_j)] ? 8 : 0;
				const signed int right = key_state[SDL_GetScancodeFromKey(SDLK_l)] ? 8 : 0;
				signed int delta_x = (-left + right) * delta_multiplier;

				const signed int up = key_state[SDL_GetScancodeFromKey(SDLK_i)] ? 8 : 0;
				const signed int down = key_state[SDL_GetScancodeFromKey(SDLK_k)] ? 8 : 0;
				signed int delta_y = (-up + down) * delta_multiplier;

				canvas_ptr->scroll(delta_x, delta_y, dragging);
			}
			{ // bar scroll

				const signed int left = key_state[SDL_GetScancodeFromKey(SDLK_z)] ? 20 : 0;
				const signed int right = key_state[SDL_GetScancodeFromKey(SDLK_x)] ? 20 : 0;
				signed int delta_x = (-left + right) * delta_multiplier;

				bar_ptr->scroll(delta_x);
			}
			if (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				if (mouse_y_window > window_ptr->height - 16) // scroll bar area
				{
					if (leftScrollButtonHolding
						&& mouse_x_window > PANEL_WIDTH
						&& mouse_x_window < PANEL_WIDTH + 16)
					{
						bar_ptr->scroll(-50);
					}
					if (rightScrollButtonHolding
						&& mouse_x_window > window_ptr->width - 16)
					{
						bar_ptr->scroll(50);
					}
					if (scrollBarShifting
						&& mouse_x_window < window_ptr->width - 16
						&& mouse_x_window > PANEL_WIDTH + 16)
					{
						if (bar_ptr->barScrollRect.x > mouse_x_window)
						{
							bar_ptr->moveScrollBar(bar_ptr->barScrollRect.x - (bar_ptr->barScrollRect.w / 2));
						}
						else if (bar_ptr->barScrollRect.x + bar_ptr->barScrollRect.w < mouse_x_window)
						{
							bar_ptr->moveScrollBar(bar_ptr->barScrollRect.x + (bar_ptr->barScrollRect.w / 2));
						}
					}
				}
			}

			//Only draw if time between this frame and last was neither too long nor too short
			Uint32 ticksSinceLastFrame = SDL_GetTicks() - editor_ptr->gameFrameTick;
			editor_ptr->gameFrameTick = SDL_GetTicks();
			if (ticksSinceLastFrame <= 36 && ticksSinceLastFrame >= 30)
			{
				canvas_ptr->draw();
				bar_ptr->draw();
			}
			//SDL_Log("Frame: %d", ticksSinceLastFrame);

			SDL_SetRenderTarget(window_ptr->screen_renderer, NULL);
			SDL_RenderCopy(window_ptr->screen_renderer, window_ptr->screen_texture, NULL, NULL);

			if (holdingType != -1 && holdingID != -1)
				canvas_ptr->drawHeldObject(holdingType, holdingID, mouse_x_window, mouse_y_window);

			SDL_RenderPresent(window_ptr->screen_renderer);

			editor_ptr->gameFrameCount++;

			break;
		}
		default:
		{
			break;
		}
	}
}