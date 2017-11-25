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
#include "Editor/bar.hpp"
#include "lem3edit.hpp"
#include "level.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
using namespace std;

void Level::draw(Window * window, signed int x, signed int xOffset, signed int y, signed int yOffset, const Style &style, bool backgroundOnly, int zoom) const
{
	draw_objects(window, x, xOffset, y, yOffset, PERM, 0, 4999, style, zoom);
	if (backgroundOnly == false)
	{
		draw_objects(window, x, xOffset, y, yOffset, TEMP, 0, 10999, style, zoom);
		draw_objects(window, x, xOffset, y, yOffset, PERM, 5000, 10999, style, zoom);
	}

}

void Level::draw_objects(Window * window, signed int x, signed int xOffset, signed int y, signed int yOffset, int type, unsigned int id_min, unsigned int id_max, const Style &style, int zoom) const
{
	assert((unsigned)type < COUNTOF(this->object));

	for (vector<Object>::const_iterator i = object[type].begin(); i != object[type].end(); ++i)
	{

		const Object &o = *i;
		if (o.id < id_min || o.id > id_max)
			continue;
			
		unsigned int so = style.object_by_id(type, o.id);
		if (so == -1)
			continue;

		int onScreenX = (o.x - x)*zoom - xOffset;
		int onScreenY = (o.y - y)*zoom - yOffset;
		if (onScreenY < window->height - BAR_HEIGHT)
		{
			style.draw_object_texture(window, onScreenX, onScreenY, type, so, zoom, NULL);
		}
	}
}

Level::Object::Index Level::get_object_by_position( signed int x, signed int y, const Style &style, bool backgroundOnly ) const
{
	signed int i;

	if (backgroundOnly == false)
	{
		i = get_object_by_position(x, y, PERM, 5000, 10999, style);
		if (i != -1)
			return Object::Index(PERM, i);
	
		i = get_object_by_position(x, y, TEMP, 0, 10999, style);
		if (i != -1)
			return Object::Index(TEMP, i);
	}
		
	i = get_object_by_position(x, y, PERM, 0, 4999, style);
	if (i != -1)
		return Object::Index(PERM, i);
	
	return Object::Index(0, -1);
}

signed int Level::get_object_by_position(signed int x, signed int y, int type, unsigned int id_min, unsigned int id_max, const Style &style) const
{
	assert((unsigned)type < COUNTOF(this->object));
	
	for (vector<Object>::const_reverse_iterator i = object[type].rbegin(); i != object[type].rend(); ++i)
	{
		const Object &o = *i;
		if (o.id < id_min || o.id > id_max)
			continue;
		
		if (x < o.x || y < o.y)
			continue;
		
		int so = style.object_by_id(type, o.id);
		if (so == -1)
			continue;
		
		if (x < o.x + style.object[type][so].width * 8 && y < o.y + style.object[type][so].height * 2)
			return object[type].rend() - i - 1; // index
	}
	
	return -1;
}

bool Level::load( unsigned int n )
{
	const string path = "LEVELS/";
	const string level = "LEVEL";
	const string perm = "PERM", temp = "TEMP";

	level_id = n;
	
	return load_level(path, level, n) &&
	       load_objects(PERM, path, perm, n) &&
	       load_objects(TEMP, path, temp, n);
}

bool Level::load_level( const std::string &path, const std::string &name, unsigned int n )
{
	const string dat = ".DAT";
	
	return load_level(l3_filename(path, name, n, dat));
}

bool Level::load_level( const string &filename )
{
	ifstream f(filename.c_str(), ios::binary);
	if (!f)
	{
		cerr << "failed to open '" << filename << "'" << endl;
		return false;
	}
	
	f.read((char *)&tribe,          sizeof(tribe));
	f.read((char *)&cave_map,       sizeof(cave_map));
	f.read((char *)&cave_raw,       sizeof(cave_raw));
	f.read((char *)&temp,           sizeof(temp));
	f.read((char *)&perm,           sizeof(perm));
	f.read((char *)&style,          sizeof(style));
	f.read((char *)&width,          sizeof(width));
	f.read((char *)&height,         sizeof(height));
	f.read((char *)&x,              sizeof(x));
	f.read((char *)&y,              sizeof(y));
	f.read((char *)&time,           sizeof(time));
	f.read((char *)&extra_lemmings, sizeof(extra_lemmings));
	f.read((char *)&unknown,        sizeof(unknown));
	f.read((char *)&release_rate,   sizeof(release_rate));
	f.read((char *)&release_delay,  sizeof(release_delay));
	f.read((char *)&enemies,        sizeof(enemies));
	
	cout << "loaded level from '" << filename << "'" << endl;

	f.close();
	return true;
}

bool Level::load_objects( int type, const string &path, const string &name, unsigned int n )
{
	assert((unsigned)type < COUNTOF(this->object));
	
	const string obs = ".OBS";
	
	return load_objects(type, l3_filename(path, name, n, obs));
}

bool Level::load_objects( int type, const string &filename )
{
	assert((unsigned)type < COUNTOF(this->object));
	
	object[type].clear();
	
	ifstream f(filename.c_str(), ios::binary);
	if (!f)
	{
		cerr << "failed to open '" << filename << "'" << endl;
		return false;
	}
	
	while (true)
	{
		Object o;
		
		f.read((char *)&o.id, sizeof(o.id));
		f.read((char *)&o.x,  sizeof(o.x));
		f.read((char *)&o.y,  sizeof(o.y));
		
		if (!f)
			break;
		
		object[type].push_back(o);
	}
	
	cout << "loaded " << object[type].size() << " objects from '" << filename << "'" << endl;
	f.close();
	return true;
}

bool Level::save(unsigned int n)
{
	const string path = "LEVELS/";
	const string level = "LEVEL";
	const string perm = "PERM", temp = "TEMP";

	enemies = 0;
	extra_lemmings = 0;

	return save_objects(PERM, path, perm, n) &&
		save_objects(TEMP, path, temp, n) &&
		save_level(path, level, n);
}

bool Level::save_level(const std::string &path, const std::string &name, unsigned int n)
{
	const string dat = ".DAT";

	return save_level(l3_filename(path, name, n, dat));
}

bool Level::save_level(const string &filename)
{
	ofstream f(filename.c_str(), ios::binary | ios::trunc);
	if (!f)
	{
		cerr << "failed to open '" << filename << "'" << endl;
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
	f.write((char *)&x, sizeof(x));
	f.write((char *)&y, sizeof(y));
	f.write((char *)&time, sizeof(time));
	f.write((char *)&extra_lemmings, sizeof(extra_lemmings));
	f.write((char *)&unknown, sizeof(unknown));
	f.write((char *)&release_rate, sizeof(release_rate));
	f.write((char *)&release_delay, sizeof(release_delay));
	f.write((char *)&enemies, sizeof(enemies));

	cout << "wrote level to '" << filename << "'" << endl;
	f.close();
	return true;
}

bool Level::save_objects(int type, const string &path, const string &name, unsigned int n)
{
	assert((unsigned)type < COUNTOF(this->object));

	const string obs = ".OBS";

	return save_objects(type, l3_filename(path, name, n, obs));
}

bool Level::save_objects(int type, const string &filename)
{
	assert((unsigned)type < COUNTOF(this->object));

	ofstream f(filename.c_str(), ios::binary | ios::trunc);
	if (!f)
	{
		cerr << "failed to open '" << filename << "'" << endl;
		return false;
	}

	for (vector<Object>::const_iterator i = object[type].begin(); i != object[type].end(); ++i)
	{
		const Object &o = *i;
		f.write((char *)&o.id, sizeof(o.id));
		f.write((char *)&o.x, sizeof(o.x));
		f.write((char *)&o.y, sizeof(o.y));
		if (o.id == 10006 || o.id == 10007)
			extra_lemmings++;
		if (o.id >= 10010 && o.id <= 10017)
			enemies++;
	}

	cout << "wrote " << object[type].size() << " objects to '" << filename << "'" << endl;
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
	}
}