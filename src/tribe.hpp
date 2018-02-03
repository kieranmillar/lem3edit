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
#ifndef TRIBE_HPP
#define TRIBE_HPP

#include "cmp.hpp"
#include "del.hpp"

#include "SDL.h"

#include <string>
#include <filesystem>

namespace fs = std::experimental::filesystem::v1;

class Tribe
{
public:
	Cmp cmp;

	Del panel;

	SDL_Color palette[32];

	bool load(unsigned int n, fs::path basePath);
	bool load_palette(fs::path basePath, std::string folder, std::string name, unsigned int n);
	bool load_palette(fs::path pal_filename);
};

#endif // TRIBE_HPP
