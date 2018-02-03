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
#ifndef RAW_HPP
#define RAW_HPP

#include "SDL.h"

#include <cassert>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::experimental::filesystem::v1;

class Raw
{
public:
	unsigned int width, height;

	std::vector<Uint8 *> frame;

	void blit(SDL_Surface *dest, signed int x, signed int y, unsigned int frame) const;

	bool load(fs::path basePath, std::string name);
	bool load_raw(fs::path raw_filename);

	void destroy();

	Raw(unsigned int width, unsigned int height) : width(width), height(height), frame() { assert(width % 4 == 0); }
	~Raw(void) { destroy(); }

	/*private:
		Raw(const Raw &);
		Raw & operator=(const Raw &);*/
};

#endif // RAW_HPP
