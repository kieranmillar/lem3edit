/*
 * lem3edit
 * Copyright (C) 2008-2009 Carl Reinke
 * Copyright (C) 2017-2018 Kieran Millar
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
#include "font.hpp"
#include "ini.hpp"
#include "level.hpp"
#include "raw.hpp"
#include "style.hpp"
#include "tinyfiledialogs.h"
#include "tribe.hpp"
#include "window.hpp"

#include "SDL.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

const char *prog_name = "lem3edit";
const char *prog_ver = "1.1";
const char *prog_date = "12/01/2018";

void version(void);

programMode g_currentMode;

int main(int argc, char *argv[])
{
	version();

	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO))
	{
		SDL_Log("failed to initialize SDL: %s\n", SDL_GetError());
		return false;
	}

	if (TTF_Init() == -1)
	{
		SDL_Log("failed to initialize SDL_ttf: %s\n", TTF_GetError());
		return false;
	}

	Window window;

	if (window.initialise(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT) == false) // Initialise the main program window
	{
		return EXIT_FAILURE;
	}

	//SDL_EnableKeyboardRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	//load ini file
	Ini ini;

	if (! ini.load())
	{
		tinyfd_messageBox(
			"Welcome to Lem3edit!",
			"Welcome to Lem3edit!\n\nBefore we begin, Lem3edit needs to know the location of L3CD.EXE so it can extract the game's graphics.\n\nIf you have not already done so, you need to extract the contents of your Lemmings 3 CD to your hard drive, and you will find L3CD there.\n\nYou will now be asked to browse to and select L3CD.EXE",
			"ok",
			"info",
			0);
		char const * filterPatterns[1] = { "L3CD.EXE" };
		char const * lem3cd = NULL;
		lem3cd = tinyfd_openFileDialog("Find L3CD.EXE", NULL, 1, filterPatterns, "Lemmings 3 Executable (L3CD.EXE)", 0);
		if (lem3cd == NULL)
		{
			tinyfd_messageBox(
				"Fatal Error!",
				"Without a link to L3CD, Lem3Edit cannot run. Sorry!",
				"ok",
				"error",
				0);
			return EXIT_FAILURE;
		}
		ini.setLem3cdPath(lem3cd);
		ini.save();
	}

	int level_id = 1;
	char const * fileToOpen = NULL;

	if (argc > 1)
	{
		level_id = atoi(argv[1]);
	}
	else
	{
		char const * filterPatterns[1] = { "LEVEL*.DAT" };
		fileToOpen = tinyfd_openFileDialog("Open level", NULL, 1, filterPatterns, "Lemmings 3 Level File (LEVEL###.DAT)", 0);
	}

	g_currentMode = EDITORMODE;

	Editor editor;
	if (!fileToOpen)
	{
		editor.load(level_id, &window);
	}
	else
	{
		editor.load(fileToOpen, &window);
	}

	SDL_Event event;
	while (SDL_WaitEvent(&event) && event.type != SDL_QUIT)
	{
		switch (g_currentMode)
		{
		case EDITORMODE:
			editor.editor_input.handleEditorEvents(event);
			break;
		case LEVELPROPERTIESMODE:
			editor.levelProperties.handleLevelPropertiesEvents(event);
			break;
		}
	}

	//NOTE to self: These should be destructors, but objects need to go out of scope before
	//calling Quit functions below or else will crash on exit

	editor.bar.destroy();
	window.destroy();

	TTF_Quit();
	SDL_Quit();

	editor.style.destroy_all_objects(PERM);
	editor.style.destroy_all_objects(TEMP);
	editor.style.destroy_all_objects(TOOL);

	return EXIT_SUCCESS;
}

void die(void)
{
	SDL_Event event;
	event.type = SDL_QUIT;
	SDL_PushEvent(&event);
}

string l3_filename_number(const int n)
{
	ostringstream filename;
	filename << setfill('0') << setw(3) << n;
	return filename.str();
}

string l3_filename(const string &path, const string &name, const string &ext)
{
	ostringstream filename;
	filename << path << name << ext;
	return filename.str();
}

string l3_filename(const string &path, const string &name, int n, const string &ext)
{
	ostringstream filename;
	filename << path << name << l3_filename_number(n) << ext;
	return filename.str();
}

void version(void)
{
	SDL_Log("%s %s (%s)\n", prog_name, prog_ver, prog_date);
	SDL_Log("Copyright(C) 2008 - 2009 Carl Reinke\n");
	SDL_Log("Copyright (C) 2017 Kieran Millar\n");
	SDL_Log("This is free software.  You may redistribute copies of it under the terms of\n");
	SDL_Log("the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.\n");
	SDL_Log("There is NO WARRANTY, to the extent permitted by law.\n");
}