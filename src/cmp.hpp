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
#ifndef CMP_HPP
#define CMP_HPP

#include "SDL.h"

#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::experimental::filesystem::v1;

class Cmp
{
public:
	class Animation
	{
	public:
		Uint16 width, height;

		std::vector< std::pair<std::streampos, Uint8 *> > frame;

		Animation(void) { /* nothing to do */ }
		Animation(const Animation &that) { copy(that); }
		~Animation(void) { destroy(); }

		void copy(const Animation &);
		void destroy(void);
		Animation & operator=(const Animation &);
	};

	std::vector<Animation> animation;

	void blit(SDL_Surface *dest, signed int x, signed int y, unsigned int animation, unsigned int frame) const;

	bool load(fs::path basePath, std::string folder, const std::string name, unsigned int n);
	bool load(const fs::path ind_filename, const fs::path cmp_filename);

	Cmp() {}

	/*private:
		Cmp(const Cmp &);
		Cmp & operator=(const Cmp &);*/
};

#endif // CMP_HPP
