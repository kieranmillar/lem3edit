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
 This file contains code for reading from Lemmings 3's RAW files
 */

#include "lem3edit.hpp"
#include "raw.hpp"
#include "style.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
using namespace std;

void Raw::blit(SDL_Surface *dest, signed int x, signed int y, unsigned int frame) const
{
	if (frame >= this->frame.size())
		return assert(false);

	Uint8 *f = this->frame[frame];

	for (unsigned int by = 0; by < height; ++by)
	{
		const int oy = y + by;
		if (oy >= 0 && oy < dest->h)
		{
			for (unsigned int bx = 0; bx < width; ++bx)
			{
				const int ox = x + bx;
				if (ox >= 0 && ox < dest->w)
					((Uint8 *)dest->pixels)[oy * dest->pitch + ox] = f[(by * width) + bx];
			}
		}
	}
}

bool Raw::load(string name)
{
	const string path = "GRAPHICS/";
	const string raw = ".RAW";

	return load_raw(l3_filename(path, name, raw));
}

bool Raw::load_raw(string raw_filename)
{
	destroy();

	ifstream raw_f(raw_filename.c_str(), ios::binary);
	if (!raw_f)
	{
		cerr << "failed to open '" << raw_filename << "'" << endl;
		return false;
	}

	int size = width * height;

	Uint8 *temp = new Uint8[size];

	while (true)
	{
		raw_f.read((char *)temp, size);

		if (!raw_f)
			break;

		Uint8 *f = new Uint8[size];

		Style::Block::decode(f, temp, size);

		frame.push_back(f);
	}

	delete[] temp;

	SDL_Log("Loaded %d images from '%s'\n", frame.size(), raw_filename.c_str());
	return true;
}

void Raw::destroy(void)
{
	for (vector<Uint8 *>::const_iterator f = frame.begin(); f != frame.end(); ++f)
		delete[] * f;

	frame.clear();
}
