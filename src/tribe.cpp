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
This file contains code for reading from Lemmings 3's TRIBE and TPANL files
These contain palette and animation information
*/

#include "lem3edit.hpp"
#include "tribe.hpp"

#include <fstream>
#include <iostream>
using namespace std;

bool Tribe::load(unsigned int n)
{
	const string path = "GRAPHICS/";
	const string tribe = "TRIBE";
	const string tpanl = "TPANL";

	return load_palette(path, tribe, n) &&
		cmp.load(path, tribe, n) &&
		panel.load(path, tpanl, n);
}

bool Tribe::load_palette(string path, string name, unsigned int n)
{
	const string pal = ".PAL";

	return load_palette(l3_filename(path, name, n, pal));
}

bool Tribe::load_palette(string pal_filename)
{
	ifstream pal_f(pal_filename.c_str(), ios::binary);
	if (!pal_f)
	{
		cerr << "failed to open '" << pal_filename << "'" << endl;
		return false;
	}

	for (int i = 0; i < 32; ++i)
	{
		Uint8 r, g, b;

		pal_f.read((char *)&r, sizeof(r));
		pal_f.read((char *)&g, sizeof(g));
		pal_f.read((char *)&b, sizeof(b));

		palette[i].r = (255.0f / 63.0f) * r;
		palette[i].g = (255.0f / 63.0f) * g;
		palette[i].b = (255.0f / 63.0f) * b;
	}

	return true;
}
