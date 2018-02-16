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
#include "Editor/bar.hpp"
#include "Editor/canvas.hpp"
#include "lem3edit.hpp"
#include "level.hpp"
#include "style.hpp"

#include <cassert>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>

using namespace std;
namespace fs = std::experimental::filesystem::v1;

void Level::setReferences(Canvas * c, Style * s)
{
	canvas_ptr = c;
	style_ptr = s;
}

void Level::draw(signed int x, signed int xOffset, signed int y, signed int yOffset, int zoom) const
{
	for (int i = 0; i < 3; i++)
	{
		if (canvas_ptr->layerVisible[i])
			draw_objects(x, xOffset, y, yOffset, i, zoom);
	}
}

void Level::draw_objects(signed int x, signed int xOffset, signed int y, signed int yOffset, int type, int zoom) const
{
	assert((unsigned)type < COUNTOF(this->object));

	for (vector<Object>::const_iterator i = object[type].begin(); i != object[type].end(); ++i)
	{
		const Object &o = *i;

		unsigned int so = style_ptr->object_by_id(type, o.id);
		if (so == -1)
			continue;

		int onScreenX = (o.x - x)*zoom - xOffset;
		int onScreenY = (o.y - y)*zoom - yOffset;
		if (onScreenY < g_window.height - BAR_HEIGHT)
		{
			style_ptr->draw_object_texture(onScreenX, onScreenY, type, so, zoom, 0);
		}
	}
}

Level::Object::Index Level::get_object_by_position(signed int x, signed int y) const
{
	signed int i;

	for (int j = 2; j >= 0; j--)
	{
		if (canvas_ptr->layerVisible[j])
		{
			i = get_object_by_position(x, y, j);
			if (i != -1)
				return Object::Index(j, i);
		}
	}

	return Object::Index(0, -1);
}

signed int Level::get_object_by_position(signed int x, signed int y, int type) const
{
	assert((unsigned)type < COUNTOF(this->object));

	for (vector<Object>::const_reverse_iterator i = object[type].rbegin(); i != object[type].rend(); ++i)
	{
		const Object &o = *i;

		if (x < o.x || y < o.y)
			continue;

		int so = style_ptr->object_by_id(type, o.id);
		if (so == -1)
			continue;

		if (x < o.x + style_ptr->object[type][so].width * 8 && y < o.y + style_ptr->object[type][so].height * 2)
			return object[type].rend() - i - 1; // index
	}

	return -1;
}

std::vector<int> Level::get_objects_in_area(int areaX, int areaY, int areaW, int areaH, int type) const
{
	vector<int> tmp;

	for (vector<Object>::const_reverse_iterator i = object[type].rbegin(); i != object[type].rend(); ++i)
	{
		const Object &o = *i;
		int so = style_ptr->object_by_id(type, o.id);
		if (so == -1)
			continue;
		if (areaX >= o.x + style_ptr->object[type][so].width * 8)
			continue;
		if (areaY >= o.y + style_ptr->object[type][so].height * 2)
			continue;
		if (areaX + areaW < o.x)
			continue;
		if (areaY + areaH < o.y)
			continue;
		tmp.push_back(object[type].rend() - i - 1);
	}
	return tmp;
}

//Creates a new level.
//You need to create the 2 OBS files under the correct number BEFORE calling this function
//then pass the .DAT filepath as an argument
void Level::newLevel(const fs::path filename, const tribeName t, const int n)
{
	levelPath = filename;
	level_id = n;

	object[TEMP].clear();
	object[PERM].clear();
	object[TOOL].clear();

	switch (t)
	{
	case CLASSIC:
		tribe = 4;
		style = 1;
		break;
	case SHADOW:
		tribe = 10;
		style = 2;
		break;
	case EGYPT:
		tribe = 5;
		style = 3;
		break;
	}
	cave_map = 0;
	cave_raw = 0;
	temp = level_id;
	perm = level_id;
	width = 320;
	height = 160;
	cameraX = 0;
	cameraY = 0;
	time = 420;
	extra_lemmings = 0;
	unknown = 2;
	release_rate = 23;
	release_delay = 46;
	enemies = 0;

	save(false);
}

bool Level::load(const fs::path filename)
{
	levelPath = filename;
	level_id = 0;

	string levelNum = filename.stem().generic_string();
	levelNum = levelNum.substr(5, 8);
	level_id = atoi(levelNum.c_str());

	return load_level(filename) &&
		load_objects(PERM, levelPath.parent_path(), "PERM", perm) &&
		load_objects(TEMP, levelPath.parent_path(), "TEMP", temp);
}

bool Level::load_level(const std::string &path, const std::string &name, unsigned int n)
{
	const string dat = "DAT";

	return load_level(l3_filename_level(path, name, n, dat));
}

bool Level::load_level(const fs::path filename)
{
	ifstream f(filename, ios::binary);
	if (!f)
	{
		SDL_Log("Failed to open '%s'\n", filename.generic_string().c_str());
		return false;
	}

	f.read((char *)&tribe, sizeof(tribe));
	f.read((char *)&cave_map, sizeof(cave_map));
	f.read((char *)&cave_raw, sizeof(cave_raw));
	f.read((char *)&temp, sizeof(temp));
	f.read((char *)&perm, sizeof(perm));
	f.read((char *)&style, sizeof(style));
	f.read((char *)&width, sizeof(width));
	f.read((char *)&height, sizeof(height));
	f.read((char *)&cameraX, sizeof(cameraX));
	f.read((char *)&cameraY, sizeof(cameraY));
	f.read((char *)&time, sizeof(time));
	f.read((char *)&extra_lemmings, sizeof(extra_lemmings));
	f.read((char *)&unknown, sizeof(unknown));
	f.read((char *)&release_rate, sizeof(release_rate));
	f.read((char *)&release_delay, sizeof(release_delay));
	f.read((char *)&enemies, sizeof(enemies));

	SDL_Log("Loaded level from  '%s'\n", filename.generic_string().c_str());

	if (temp == perm)
		level_id = temp;

	f.close();
	return true;
}

bool Level::load_objects(int type, const fs::path parentPath, const string &name, unsigned int n)
{
	assert((unsigned)type < COUNTOF(this->object));

	const string obs = "OBS";

	return load_objects(type, l3_filename_level(parentPath, name, n, obs));
}

bool Level::load_objects(int type, const fs::path filename)
{
	assert((unsigned)type < COUNTOF(this->object));

	object[type].clear();
	if (type == PERM)
		object[TOOL].clear();

	ifstream f(filename, ios::binary);
	if (!f)
	{
		SDL_Log("Failed to open '%s'\n", filename.generic_string().c_str());
		return false;
	}

	while (true)
	{
		Object o;

		f.read((char *)&o.id, sizeof(o.id));
		f.read((char *)&o.x, sizeof(o.x));
		f.read((char *)&o.y, sizeof(o.y));

		if (!f)
			break;

		if (o.id < 5000)
		{
			object[type].push_back(o);
		}
		else
		{
			object[TOOL].push_back(o);
		}
	}

	if (type == PERM)
		SDL_Log("Loaded %d + %d objects from '%s'\n", object[type].size(), object[TOOL].size(), filename.generic_string().c_str());
	if (type == TEMP)
		SDL_Log("Loaded %d objects from '%s'\n", object[type].size(), filename.generic_string().c_str());
	f.close();
	return true;
}

//Return if object has invalid id or lies entirely outside level borders
bool Level::validate(const Object * o, const int type)
{
	int so = style_ptr->object_by_id(type, o->id);
	if (so == -1)
	{
		SDL_Log("Didn't save invalid object type: %d\n", o->id);
		return false;
	}

	int w = style_ptr->object[type][so].width * 8;
	int h = style_ptr->object[type][so].height * 2;
	bool outsideBorder = false;

	if (o->x + w <= 0)
		outsideBorder = true;
	if (o->y + h <= 0)
		outsideBorder = true;
	if (o->x >= width)
		outsideBorder = true;
	if (o->y >= height)
		outsideBorder = true;

	if (outsideBorder)
	{
		SDL_Log("Didn't save object outside borders: ID %d, X %d, Y %d, W %d, H %d\n", o->id, o->x, o->y, w, h);
		return false;
	}

	return true;
}

bool Level::save(const bool giveFeedback)
{
	enemies = 0;
	extra_lemmings = 0;

	if (save_objects(PERM, levelPath.parent_path(), perm) &&
		save_objects(TEMP, levelPath.parent_path(), temp) &&
		save_level(levelPath) == true)
	{
		if (giveFeedback)
			SDL_ShowSimpleMessageBox(0, "Save Complete", "Level saved!", NULL);
	}
	else
	{
		SDL_ShowSimpleMessageBox(0, "Oh no!", "Failed to save :(", NULL);
		return false;
	}
	return true;
}

bool Level::save_level(const fs::path parentPath, unsigned int n)
{
	return save_level(l3_filename_level(parentPath, "LEVEL", n, "DAT"));
}

bool Level::save_level(const fs::path filename)
{
	ofstream f(filename, ios::binary | ios::trunc);
	if (!f)
	{
		SDL_Log("Failed to open '%s'\n", filename.generic_string().c_str());
		return false;
	}

	f.write((char *)&tribe, sizeof(tribe));
	f.write((char *)&cave_map, sizeof(cave_map));
	f.write((char *)&cave_raw, sizeof(cave_raw));
	f.write((char *)&temp, sizeof(temp));
	f.write((char *)&perm, sizeof(perm));
	f.write((char *)&style, sizeof(style));
	f.write((char *)&width, sizeof(width));
	f.write((char *)&height, sizeof(height));
	f.write((char *)&cameraX, sizeof(cameraX));
	f.write((char *)&cameraY, sizeof(cameraY));
	f.write((char *)&time, sizeof(time));
	f.write((char *)&extra_lemmings, sizeof(extra_lemmings));
	f.write((char *)&unknown, sizeof(unknown));
	f.write((char *)&release_rate, sizeof(release_rate));
	f.write((char *)&release_delay, sizeof(release_delay));
	f.write((char *)&enemies, sizeof(enemies));

	SDL_Log("Wrote level to '%s'\n", filename.generic_string().c_str());
	f.close();
	return true;
}

bool Level::save_objects(int type, fs::path parentPath, unsigned int n)
{
	assert((unsigned)type < COUNTOF(this->object));

	const string obs = "OBS";
	string name;

	if (type == PERM)
		name = "PERM";
	else
		name = "TEMP";
	return save_objects(type, l3_filename_level(parentPath, name, n, "OBS"));
}

bool Level::save_objects(int type, const fs::path filename)
{
	assert((unsigned)type < COUNTOF(this->object));

	ofstream f(filename, ios::binary | ios::trunc);
	if (!f)
	{
		SDL_Log("Failed to open '%s'\n", filename.generic_string().c_str());
		return false;
	}

	int count = 0;
	int numTypesToSave = 1;
	int savingType = type;
	if (type == PERM)
		numTypesToSave = 2;
	for (int j = 0; j < numTypesToSave; j++)
	{
		for (vector<Object>::const_iterator i = object[savingType].begin(); i != object[savingType].end(); ++i)
		{
			const Object &o = *i;
			if (validate(&o, savingType))
			{
				count++;
				f.write((char *)&o.id, sizeof(o.id));
				f.write((char *)&o.x, sizeof(o.x));
				f.write((char *)&o.y, sizeof(o.y));
				if (o.id == 10006 || o.id == 10007)
					extra_lemmings++;
				if (o.id >= 10010 && o.id <= 10017)
					enemies++;
			}
		}
		savingType = TOOL;
	}

	SDL_Log("Wrote %d objects to '%s'\n", count, filename.generic_string().c_str());

	f.close();
	return true;
}

void Level::resizeLevel(int delta_x, int delta_y, bool shiftLevel)
{
	width += delta_x;
	height += delta_y;

	if (shiftLevel)
	{
		for (vector<Object>::iterator i = object[PERM].begin(); i != object[PERM].end(); ++i)
		{
			Object &o = *i;
			o.x += delta_x;
			o.y += delta_y;
		}
		for (vector<Object>::iterator i = object[TEMP].begin(); i != object[TEMP].end(); ++i)
		{
			Object &o = *i;
			o.x += delta_x;
			o.y += delta_y;
		}
		for (vector<Object>::iterator i = object[TOOL].begin(); i != object[TOOL].end(); ++i)
		{
			Object &o = *i;
			o.x += delta_x;
			o.y += delta_y;
		}
		cameraX += delta_x;
		cameraY += delta_y;
	}

	if (cameraX < 0)
		cameraX = 0;
	if (cameraY < 0)
		cameraY = 0;
	if (cameraX + 320 > width)
		cameraX = width - 320;
	if (cameraY + 160 > height)
		cameraY = height - 160;
}
