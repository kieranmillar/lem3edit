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

/*
This file contains code for reading from Lemmings 3's DEL files
*/

#include "del.hpp"
#include "lem3edit.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
using namespace std;

void Del::Frame::copy( const Del::Frame &that )
{
	size = that.size;
	
	frame = new Uint8[size];
	memcpy(frame, that.frame, sizeof(Uint8) * size);
}

void Del::Frame::destroy( void )
{
	delete[] frame;
}

Del::Frame & Del::Frame::operator=( const Del::Frame &that )
{
	if (this == &that)
		return *this;
	
	destroy();
	copy(that);
	
	return *this;
}

void Del::Frame::blit( SDL_Surface *surface, signed int x, signed int y, unsigned int width, unsigned int height ) const
{
	assert(width * height == size);
	
	for (unsigned int by = 0; by < height; ++by)
	{
		const int oy = y + (height - by - 1);
		if (oy >= 0 && oy < surface->h)
		{
			for (unsigned int bx = 0; bx < width; ++bx)
			{
				const int ox = x + bx;
				if (ox >= 0 && ox < surface->w)
				{
					if (frame[(by * width) + bx])
						((Uint8 *)surface->pixels)[oy * surface->pitch + ox] = frame[(by * width) + bx];
				}
			}
		}
	}
}

void Del::blit( SDL_Surface *surface, signed int x, signed int y, unsigned int frame, unsigned int width, unsigned int height ) const
{
	if (frame >= this->frame.size())
		return assert(false);
	
	this->frame[frame].blit(surface, x, y, width, height);
}

void Del::blit_text( SDL_Surface *surface, signed int x, signed int y, const string &text ) const
{
	const int font_width = 8, font_height = 8;
	
	for (unsigned int i = 0; i < text.length(); ++i)
	{
		char temp = tolower(text[i]);
		if (temp >= '0' && temp <= '9')
			blit(surface, x + i * font_width, y, temp - '0' + 30, font_width, font_height);
		else if (temp >= 'a' && temp <= 'z')
			blit(surface, x + i * font_width, y, temp - 'a' + 40, font_width, font_height);
		else if (temp >= ' ' && temp <= '!')
			blit(surface, x + i * font_width, y, temp - ' ' + 66, font_width, font_height);
	}
}

bool Del::load( const string &name )
{
	const string path = "GRAPHICS/";
	const string din = ".DIN", del = ".DEL";
	
	return load(l3_filename(path, name, din), l3_filename(path, name, del));
}

bool Del::load( const std::string &path, const std::string &name, unsigned int n )
{
	const string din = ".DIN", del = ".DEL";
	
	return load(l3_filename(path, name, n, din), l3_filename(path, name, n, del));
}

bool Del::load( const string &din_filename, const string &del_filename )
{
	frame.clear();
	
	ifstream din_f(din_filename.c_str(), ios::binary);
	if (!din_f)
	{
		cerr << "failed to open '" << din_filename << "'" << endl;
		return false;
	}
	
	ifstream del_f(del_filename.c_str(), ios::binary);
	if (!del_f)
	{
		cerr << " failed to open '" << del_filename << "'" << endl;
		return false;
	}
	
	while (true)
	{
		Uint16 size;
		
		din_f.read((char *)&size, sizeof(size));
		
		if (!din_f)
			break;
		
		Frame f(size);
		
		del_f.read((char *)f.frame, f.size);
		
		if (!del_f)
			return false;
		
		frame.push_back(f);
	}
	
	cout << "loaded " << frame.size() << " images from '" << del_filename << "'" << endl;
	return true;
}