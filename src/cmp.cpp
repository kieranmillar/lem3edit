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
 This file contains code for reading from Lemmings 3's CMP files
 */

#include "cmp.hpp"
#include "lem3edit.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <experimental/filesystem>

using namespace std;
namespace fs = std::experimental::filesystem::v1;

void Cmp::Animation::copy(const Cmp::Animation &that)
{
	width = that.width;
	height = that.height;

	for (vector< pair<streampos, Uint8 *> >::const_iterator i = that.frame.begin(); i != that.frame.end(); ++i)
	{
		Uint8 *temp = new Uint8[i->first];
		memcpy(temp, i->second, i->first);
		frame.push_back(pair<streampos, Uint8 *>(i->first, temp));
	}
}

void Cmp::Animation::destroy(void)
{
	for (vector< pair<streampos, Uint8 *> >::const_iterator i = frame.begin(); i != frame.end(); ++i)
		delete[] i->second;
}

Cmp::Animation & Cmp::Animation::operator=(const Cmp::Animation &that)
{
	if (this == &that)
		return *this;

	destroy();
	copy(that);

	return *this;
}

void Cmp::blit(SDL_Surface *surface, signed int x, signed int y, unsigned int animation, unsigned int frame) const
{
	if (animation >= this->animation.size())
		return assert(false);
	if (frame >= this->animation[animation].frame.size())
		return assert(false);

	signed int sx = 0, sy = 0;

	int i = 0, o = 0;
	while (i < 4)
	{
		Uint8 cmp_temp = this->animation[animation].frame[frame].second[o++];

		switch (cmp_temp)
		{
		case 0x00:
			sx = 0;
			++sy;
			break;
		case 0xff:
			sy = 0;
			++i;
			break;
		default:
			for (int j = 0; j < 2; ++j)
			{
				switch (cmp_temp & 0xf0)
				{
				case 0x00:
					sx = 0;
					++sy;
					break;
				case 0x10:
				case 0x20:
				case 0x30:
				case 0x40:
				case 0x50:
				case 0x60:
				case 0x70:
				case 0x80:
					for (int k = 0; k < ((cmp_temp & 0xf0) >> 4); ++k)
					{
						if (y + sy >= 0 && y + sy < surface->h)
							if (x + sx >= 0 && x + sx < surface->w)
								((Uint8 *)surface->pixels)[(y + sy) * surface->pitch + x + i + (sx * 4)] = this->animation[animation].frame[frame].second[o];
						++o;
						++sx;
					}
					break;
				case 0x90:
				case 0xa0:
				case 0xb0:
				case 0xc0:
				case 0xd0:
				case 0xe0:
					sx += ((cmp_temp & 0xf0) >> 4) - 8;
					break;
				case 0xf0:
					//cerr << __PRETTY_FUNCTION__ << " encountered invalid data" << endl;
					break;
				}

				cmp_temp <<= 4;
			}
			break;
		}
	}
}

bool Cmp::load(fs::path basePath, std::string folder, const std::string name, unsigned int n)
{
	const string ind = "IND", cmp = "CMP";

	return load(l3_filename_data(basePath, folder, name, n, ind), l3_filename_data(basePath, folder, name, n, cmp));
}

bool Cmp::load(const fs::path ind_filename, const fs::path cmp_filename)
{
	animation.clear();
	ifstream ind_f(ind_filename, ios::binary);
	if (!ind_f)
	{
		cerr << "failed to open '" << ind_filename << "'" << endl;
		return false;
	}

	ifstream cmp_f(cmp_filename, ios::binary);
	if (!cmp_f)
	{
		cerr << " failed to open '" << cmp_filename << "'" << endl;
		return false;
	}

	while (true)
	{
		Animation a;

		Uint16 frames;

		ind_f.read((char *)&a.width, sizeof(a.width));
		ind_f.read((char *)&a.height, sizeof(a.height));
		ind_f.read((char *)&frames, sizeof(frames));

		if (!ind_f)
			break;

		for (; frames > 0; --frames)
		{
			streampos cmp_o = cmp_f.tellg(), cmp_s;

			for (int k = 0; k < 4;)
			{
				Uint8 cmp_temp;
				cmp_f.read((char *)&cmp_temp, sizeof(cmp_temp));

				if (!cmp_f)
				{
					cerr << "unexpected end-of-file '" << cmp_filename << "'" << endl;
					return false;
				}

				if (cmp_temp == 0xff)
				{
					++k;
				}
				else
				{
					for (int i = 0; i < 2; ++i)
					{
						if ((cmp_temp & 0xf0) <= 0x10 && (cmp_temp & 0xf0) >= 0x80)
							cmp_f.seekg((cmp_temp & 0xf0) >> 4, ios::cur);

						cmp_temp <<= 4;
					}
				}
			}

			cmp_s = cmp_f.tellg() - cmp_o;

			Uint8 *frame = new Uint8[cmp_s];

			cmp_f.seekg(cmp_o, ios::beg);
			cmp_f.read((char *)frame, cmp_s);

			a.frame.push_back(pair<streampos, Uint8 *>(cmp_s, frame));
		}

		animation.push_back(a);
	}

	SDL_Log("Loaded %d animations from '%s'\n", animation.size(), cmp_filename.generic_string().c_str());
	return true;
}
