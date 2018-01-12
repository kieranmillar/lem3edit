/*
 * lem3edit
 * Copyright (C) 2008-2009 Carl Reinke
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
#ifndef LEVEL_HPP
#define LEVEL_HPP

#include "SDL.h"

#include <vector>
#include <string>

class Window;
class Canvas;
class Style;

class Level
{
public:

	Window * window_ptr;
	Canvas * canvas_ptr;
	Style * style_ptr;

	int level_id;

	Uint16 tribe;
	Uint16 cave_map, cave_raw;
	Uint16 temp, perm;
	Uint16 style;
	Uint16 width, height;
	Sint16 cameraX, cameraY;
	Uint16 time;
	Uint8  extra_lemmings;
	Uint8  unknown;
	Uint16 release_rate, release_delay;
	Uint16 enemies;

	class Object
	{
	public:
		class Index
		{
		public:
			int type;
			signed int i;

			Index(int type, signed int i) : type(type), i(i) { }

			inline bool operator<(const Index &that) const { return this->type != that.type ? this->type < that.type : this->i < that.i; }
		};

		Uint16 id;
		Sint16 x, y;
	};

	std::vector<Object> object[3];

	void setReferences(Window * w, Canvas * c, Style * s);

	void draw(signed int x, signed int xOffset, signed int y, signed int yOffset, int zoom) const;
	void draw_objects(signed int x, signed int xOffset, signed int y, signed int yOffset, int type, int zoom) const;

	Object::Index get_object_by_position(signed int x, signed int y) const;
	signed int get_object_by_position(signed int x, signed int y, int type) const;

	std::vector<int> get_objects_in_area(int areaX, int areaY, int areaW, int areaH, int type) const;

	bool load(unsigned int n);
	bool load_level(const std::string &path, const std::string &name, unsigned int n);
	bool load_level(const std::string &filename);
	bool load_objects(int type, const std::string &path, const std::string &name, unsigned int n);
	bool load_objects(int type, const std::string &filename);

	bool validate(const Object * o, const int type);

	bool save(unsigned int n);
	bool save_level(const std::string &path, const std::string &name, unsigned int n);
	bool save_level(const std::string &filename);
	bool save_objects(int type, const std::string &path, const std::string &name, unsigned int n);
	bool save_objects(int type, const std::string &filename);

	void resizeLevel(int delta_x, int delta_y, bool shiftLevel);

	Level(void) { /* nothing to do */ }

private:
	Level(const Level &);
	Level & operator=(const Level &);
};

#endif // LEVEL_HPP
