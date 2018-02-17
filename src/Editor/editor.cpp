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

 /*
 This file is the top level object of the editor and handles some general purpose functionality
 */
#include "editor.hpp"
#include "../tinyfiledialogs.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <experimental/filesystem>
using namespace std;
namespace fs = std::experimental::filesystem::v1;

Editor::Editor(fs::path data)
{
	dataPath = data;
	bar.setReferences(this, &canvas, &style);
	canvas.setReferences(this, &editor_input, &bar, &style, &level);
	editor_input.setReferences(this, &bar, &canvas, &style, &level);
	levelProperties.setReferences(this, &bar, &canvas, &level);
	level.setReferences(&canvas, &style);
	font.setReferences(&style);
}

void Editor::resize(int w, int h)
{
	canvas.resize(h);
	bar.resizeBarScrollRect(w, h);
}

void Editor::create(const fs::path filename, const tribeName t, const int n)
{
	returnMode = MAINMENUMODE;
	level.newLevel(filename, t, n);
	initiate();
	canvas.redraw = true;
}

bool Editor::load(const fs::path filename, programMode modeToReturnTo)
{
	returnMode = modeToReturnTo;
	level.load(filename);
	initiate();
	return canvas.redraw = true;
}

void Editor::initiate(void)
{
	tribe.load(level.tribe, dataPath);
	style.load(level.style, tribe.palette, dataPath);
	//font.load("FONT"); //The in-game font. Not very practical for the editor so commented out
	//font.createFont();
	bar.load();
	canvas.load();
	editor_input.load();
	levelProperties.setup();
	gameFrameCount = 0;
	gameFrameTick = SDL_GetTicks();
	startCameraOn = false;

	//prevent open file dialog mouse clicks from carrying over once level loaded
	SDL_PumpEvents();
	SDL_FlushEvents(SDL_MOUSEMOTION, SDL_MOUSEWHEEL);
}

void Editor::closeLevel(bool askToSave)
{
	if (askToSave)
	{
		int answer = tinyfd_messageBox(
			"Save Level?",
			"Save before quitting?",
			"yesnocancel",
			"question",
			0);
		if (answer == 0)
			return;
		else if (answer == 1)
			level.save(false);
	}
	bar.destroy();
	style.destroy_all_objects(PERM);
	style.destroy_all_objects(TEMP);
	style.destroy_all_objects(TOOL);
	levelProperties.destroyTextures();
	selection.clear();
	clipboard.clear();
	g_currentMode = returnMode;
}

bool Editor::select(signed int x, signed int y, bool modify_selection)
{
	Level::Object::Index temp = level.get_object_by_position(x, y);

	if (temp.i == -1)  // selected nothing
	{
		if (modify_selection == false) // don't clear the selection if holding CTRL otherwise misclicks are annoying
			selection.clear();
		canvas.redraw = true;
		return false;
	}
	else
	{
		bool already_selected = selection.find(temp) != selection.end();
		if (already_selected)
		{
			if (modify_selection)
				selection.erase(temp);
			else
			{
				editor_input.dragging = true;
				editor_input.startDragTime = gameFrameCount;
			}
		}
		else
		{
			if (modify_selection)
				selection.insert(temp);
			else
			{
				selection.clear();
				selection.insert(temp);
				editor_input.dragging = true;
			}
		}
		canvas.redraw = true;
		return true;
	}
}

bool Editor::select_none(void)
{
	if (selection.empty())
		return false;

	selection.clear();

	return canvas.redraw = true;
}

bool Editor::select_all(void)
{
	for (unsigned int type = 0; type < COUNTOF(level.object); ++type)
	{
		if (canvas.layerVisible[type])
		{
			for (unsigned int i = 0; i < level.object[type].size(); ++i)
			{
				selection.insert(Level::Object::Index(type, i));
			}
		}
	}

	return canvas.redraw = !selection.empty();
}

bool Editor::select_area(const int areaX, const int areaY, const int areaW, const int areaH)
{
	vector<int> tmp[3];

	int x = areaX;
	int y = areaY;
	int w = areaW;
	int h = areaH;
	// We want the area to always have x and y be in the top-left corner, with w and h positive
	if (areaW < 0)
	{
		x = areaX + areaW;
		w = 0 - areaW;
	}
	if (areaH < 0)
	{
		y = areaY + areaH;
		h = 0 - areaH;
	}

	for (int i = 0; i < 3; ++i)
	{
		if (canvas.layerVisible[i] == false)
			continue;

		tmp[i] = level.get_objects_in_area(x, y, w, h, i);

		for (std::vector<int>::const_reverse_iterator j = tmp[i].rbegin(); j != tmp[i].rend(); ++j)
		{
			selection.insert(Level::Object::Index(i, *j));
		}
	}

	canvas.redraw = !selection.empty();
	return true;
}

bool Editor::copy_selected(void)
{
	clipboard.clear();

	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		clipboard.push_back(pair<Level::Object::Index, Level::Object>(*i, level.object[i->type][i->i]));
	}

	return !clipboard.empty();
}

bool Editor::paste(void)
{
	selection.clear(); // maybe delete selection instead?

	for (Clipboard::const_iterator i = clipboard.begin(); i != clipboard.end(); ++i)
	{
		const Level::Object::Index &index = i->first;

		level.object[index.type].push_back(i->second);

		selection.insert(Level::Object::Index(index.type, level.object[index.type].size() - 1));
	}

	return canvas.redraw = !clipboard.empty();
}

bool Editor::addObject(int idToAdd, int typeToAdd, int xToAdd, int yToAdd)
{
	Level::Object o;

	o.id = idToAdd;
	o.x = xToAdd;
	o.y = yToAdd;
	level.object[typeToAdd].push_back(o);

	return canvas.redraw = true;
}

bool Editor::moveToFront(void)
{
	std::vector<int> indexes[COUNTOF(level.object)];

	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		indexes[i->type].push_back(i->i);
	}
	selection.clear();

	for (unsigned int type = 0; type < COUNTOF(level.object); ++type)
	{
		std::sort(indexes[type].begin(), indexes[type].end());

		for (unsigned int i = 0; i < indexes[type].size(); i++)
		{
			int j = indexes[type][i] - i;
			level.object[type].push_back(level.object[type][j]);
			level.object[type].erase(level.object[type].begin() + j);

			selection.insert(Level::Object::Index(type, level.object[type].size() - i - 1));
		}
	}

	return canvas.redraw = true;
}

bool Editor::moveToBack(void)
{
	std::vector<int> indexes[COUNTOF(level.object)];

	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		indexes[i->type].push_back(i->i);
	}
	selection.clear();

	for (unsigned int type = 0; type < COUNTOF(level.object); ++type)
	{
		std::sort(indexes[type].begin(), indexes[type].end());
		std::reverse(indexes[type].begin(), indexes[type].end());

		for (unsigned int i = 0; i < indexes[type].size(); i++)
		{
			int j = indexes[type][i] + i;
			level.object[type].insert(level.object[type].begin(), level.object[type][j]);
			level.object[type].erase(level.object[type].begin() + j + 1);

			selection.insert(Level::Object::Index(type, i));
		}
	}

	return canvas.redraw = true;
}

bool Editor::decrease_obj_id(void)
{
	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		Level::Object &o = level.object[i->type][i->i];
		o.id = style.object_prev_id(i->type, o.id);
	}

	return canvas.redraw = true;
}

bool Editor::increase_obj_id(void)
{
	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		Level::Object &o = level.object[i->type][i->i];
		o.id = style.object_next_id(i->type, o.id);
	}

	return canvas.redraw = true;
}

bool Editor::delete_selected(void)
{
	if (selection.empty())
		return false;

	for (Selection::const_reverse_iterator i = selection.rbegin(); i != selection.rend(); ++i)
		level.object[i->type].erase(level.object[i->type].begin() + i->i);

	selection.clear();

	return canvas.redraw = true;
}

//This function takes in how much the mouse has moved in the last frame
//Input is dependent of zoom, so provide screen pixel values
//So to ensure smooth scrolling, it maintains a remainder after relocating the moved objects
//and snapping them to the grid
bool Editor::move_selected(signed int delta_x, signed int delta_y)
{
	delta_x += canvas.mouse_remainder_x;
	delta_y += canvas.mouse_remainder_y;
	canvas.mouse_remainder_x = delta_x % (8 * canvas.zoom);
	canvas.mouse_remainder_y = delta_y % (2 * canvas.zoom);

	delta_x /= (8 * canvas.zoom);
	delta_y /= (2 * canvas.zoom);

	if (selection.empty() || (delta_x == 0 && delta_y == 0))
		return false;

	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		Level::Object &o = level.object[i->type][i->i];
		o.x += delta_x * 8;
		o.y += delta_y * 2;
	}

	return canvas.redraw = true;
}

bool Editor::toggleCameraVisibility(void)
{
	if (startCameraOn)
		startCameraOn = false;
	else
		startCameraOn = true;
	canvas.redraw = true;
	return true;
}

bool Editor::move_camera(signed int delta_x, signed int delta_y)
{
	delta_x += canvas.mouse_remainder_x;
	delta_y += canvas.mouse_remainder_y;
	canvas.mouse_remainder_x = delta_x % (8 * canvas.zoom);
	canvas.mouse_remainder_y = delta_y % (2 * canvas.zoom);

	delta_x /= (8 * canvas.zoom);
	delta_y /= (2 * canvas.zoom);

	if (delta_x == 0 && delta_y == 0)
		return false;

	level.cameraX += delta_x * 8;
	level.cameraY += delta_y * 2;

	if (level.cameraX < 0)
		level.cameraX = 0;
	if (level.cameraY < 0)
		level.cameraY = 0;
	if (level.cameraX + 320 > level.width)
		level.cameraX = level.width - 320;
	if (level.cameraY + 160 > level.height)
		level.cameraY = level.height - 160;

	return canvas.redraw = true;
}
