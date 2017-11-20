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

#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600

#include "lem3edit.hpp"

#include "Editor/editor.hpp"
#include "level.hpp"
#include "raw.hpp"
#include "style.hpp"
#include "tribe.hpp"
#include "window.hpp"

#include "SDL.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

const char *prog_name = "lem3edit";
const char *prog_ver = "0.4";
const char *prog_date = "12/11/2017";

void version(void);

void die( void );

signed int snap( signed int &value, unsigned int snap );

Sint32 mouse_prev_x, mouse_prev_y;



int main( int argc, char *argv[] )
{

	version();

	Window window;

	if (window.initialise(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT) == false) // Initialise the main program window
	{ 
		return EXIT_FAILURE;
	}
	
	//SDL_EnableKeyboardRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	
	int level_id = (argc > 1) ? atoi(argv[1]) : 1;
	
	Editor editor;
	editor.load(level_id, &window);

	mouse_prev_x = mouse_prev_y = 0;

	SDL_Event event;
	while (SDL_WaitEvent(&event) && event.type != SDL_QUIT)
	{
		unsigned int delta_multiplier = (SDL_GetModState() & KMOD_SHIFT) ? 8 : 2;

		Sint32 mouse_x_window, mouse_y_window;
		Uint8 mouse_state = SDL_GetMouseState(&mouse_x_window, &mouse_y_window);
		// Due to zooming we can't rely on the mouse's position in the window to map correctly
		// So we need to create our own variables instead
		// These ones give the true x co-ordinate of the level, ignoring zoom and scroll
		Sint32 mouse_x, mouse_y;
		mouse_x = (mouse_x_window / editor.canvas.zoom) + editor.canvas.scroll_x;
		mouse_y = (mouse_y_window / editor.canvas.zoom) + editor.canvas.scroll_y;

		switch (event.type)
		{
			case SDL_WINDOWEVENT:
			{
				SDL_WindowEvent &e = event.window;

				if (e.event == SDL_WINDOWEVENT_RESIZED)
				{
					window.width = e.data1;
					window.height = e.data2;
					window.resize();
					editor.resize(e.data1, e.data2);
					editor.canvas.redraw = true;
				}
			}
			case SDL_MOUSEMOTION:
			{
				SDL_MouseMotionEvent &e = event.motion;

				if (e.state & SDL_BUTTON(SDL_BUTTON_LEFT))
					editor.move_selected(mouse_x_window - mouse_prev_x, mouse_y_window - mouse_prev_y );

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
					editor.canvas.mouse_remainder_x = 0;
					editor.canvas.mouse_remainder_y = 0;
				}

				if (e.button == SDL_BUTTON_LEFT || e.button == SDL_BUTTON_RIGHT)
					editor.select(mouse_x, mouse_y, ctrl_down);
				
				break;
			}
			case SDL_MOUSEBUTTONUP://when released
			{
				SDL_MouseButtonEvent &e = event.button;

				if (e.button == SDL_BUTTON_LEFT)
					editor.canvas.mouse_remainder_x = editor.canvas.mouse_remainder_y = 0;

				break;
			}
			case SDL_MOUSEWHEEL:
			{
				SDL_MouseWheelEvent &e = event.wheel;
				if (!mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT))
				{
					if (e.y == 1)
					{
						if (editor.canvas.zoom != 16)
							editor.canvas.zoom *= 2;
						editor.canvas.redraw = true;
					}
					if (e.y == -1)
					{
						if (editor.canvas.zoom != 1)
							editor.canvas.zoom /= 2;
						editor.canvas.redraw = true;
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
						editor.bar.changeType(PERM);
						break;
					case SDLK_2:
						editor.bar.changeType(TEMP);
						break;
					case SDLK_s:
						editor.save(level_id);
						break;
					case SDLK_ESCAPE:
						editor.select_none();
						break;
					case SDLK_a:
						editor.select_all();
						break;
					case SDLK_c:
						if (ctrl_down) {
							editor.copy_selected();
						}
						break;
					case SDLK_v:
						if (ctrl_down) {
							editor.paste();
						}
						break;
					case SDLK_z:
						//editor.decrease_obj_id();
						editor.bar.scroll(-25 * delta_multiplier);
						break;
					case SDLK_x:
						//editor.increase_obj_id();
						editor.bar.scroll(25 * delta_multiplier);
						break;
					case SDLK_UP:
					case SDLK_DOWN:
						if (!mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT))
							editor.move_selected(0, (e.keysym.sym == SDLK_UP ? -1 : e.keysym.sym == SDLK_DOWN ? 1 : 0) * delta_multiplier * editor.canvas.zoom);
						break;
					case SDLK_LEFT:
					case SDLK_RIGHT:
						if (!mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT))
							editor.move_selected((e.keysym.sym == SDLK_LEFT ? -4 : e.keysym.sym == SDLK_RIGHT ? 4 : 0) * delta_multiplier * editor.canvas.zoom, 0);
						break;
					case SDLK_DELETE:
						editor.delete_selected();
						break;
					case SDLK_q:
						die();
						break;
					case SDLK_b:
						if (editor.canvas.backgroundOnly)
						{
							editor.canvas.backgroundOnly = false;
						}
						else
						{
							editor.canvas.backgroundOnly = true;
						}
							
						editor.canvas.redraw = true;
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
					const int mouse_scroll_trigger = min(window.width, window.height) / 32;
					
					const signed int left = (mouse_x_window < mouse_scroll_trigger) ? /*2*/0 : 0 + key_state[SDL_GetScancodeFromKey(SDLK_j)] ? 1 : 0;
					const signed int right = (mouse_x_window >= (signed)window.width - mouse_scroll_trigger) ? /*2*/0 : 0 + key_state[SDL_GetScancodeFromKey(SDLK_l)] ? 1 : 0;
					signed int delta_x = (-left + right) * delta_multiplier;
					
					const signed int up = (mouse_y_window < mouse_scroll_trigger) ? /*2*/0 : 0 + key_state[SDL_GetScancodeFromKey(SDLK_i)] ? 1 : 0;
					const signed int down = (mouse_y_window >= (signed)window.height - mouse_scroll_trigger) ? /*2*/0 : 0 + key_state[SDL_GetScancodeFromKey(SDLK_k)] ? 1 : 0;
					signed int delta_y = (-up + down) * delta_multiplier;


					editor.scroll(delta_x, delta_y, mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT));

				}

				editor.canvas.draw();
							
				break;
			}
			default:
				break;
		}
	}

	window.destroy();

	editor.style.destroy_all_objects(PERM);
	editor.style.destroy_all_objects(TEMP);
	
	return EXIT_SUCCESS;
}

void die( void )
{
	SDL_Event event;
	event.type = SDL_QUIT;
	SDL_PushEvent(&event);
}



string l3_filename( const string &path, const string &name, const string &ext )
{
	ostringstream filename;
	filename << path << name << ext;
	return filename.str();
}

string l3_filename( const string &path, const string &name, int n, const string &ext )
{
	ostringstream filename;
	filename << path << name << setfill('0') << setw(3) << n << ext;
	return filename.str();
}

void version(void)
{
	cerr << prog_name << " " << prog_ver << " (" << prog_date << ")" << endl;
	cerr << "Copyright (C) 2008-2009 Carl Reinke" << endl;
	cerr << "Copyright (C) 2017 Kieran Millar" << endl << endl;
	cerr << "This is free software.  You may redistribute copies of it under the terms of" << endl
		<< "the GNU General Public License <http://www.gnu.org/licenses/gpl.html>." << endl
		<< "There is NO WARRANTY, to the extent permitted by law." << endl << endl;
}

void printDebugNumber(int n)
{
	std::string str = std::to_string(n);
	char const *pchar = str.c_str();  //use char const* as target type
	SDL_ShowSimpleMessageBox(0, "Debug", pchar, NULL);
}