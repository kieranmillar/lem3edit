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
#include "lem3edit.hpp"
#include "level.hpp"
#include "style.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
using namespace std;

Style::Object & Style::Object::operator=( const Style::Object &that )
{
	if (this == &that)
		return *this;
	
	destroy();
	copy(that);
	
	return *this;
}

void Style::Object::copy( const Style::Object &that )
{
	id = that.id;
	width = that.width;
	height = that.height;
	frl = that.frl;
	texX = that.texX;
	texY = that.texY;
	
	for (unsigned int i = 0; i < COUNTOF(unknown); ++i)
		unknown[i] = that.unknown[i];
	
	for (vector<Uint16 *>::const_iterator f = that.frame.begin(); f != that.frame.end(); ++f)
	{
		Uint16 *temp = new Uint16[width * height];
		memcpy(temp, *f, sizeof(Uint16) * width * height);
		frame.push_back(temp);
	}
}

void Style::Object::destroy( void )
{
	for (vector<Uint16 *>::const_iterator f = frame.begin(); f != frame.end(); ++f)
		delete[] *f;
}

void Style::Block::blit( SDL_Surface *dest, signed int x, signed int y ) const
{
	for (int by = 0; by < 2; ++by)
	{
		const int oy = y + by;
		if (oy >= 0 && oy < dest->h)
		{
			for (int bx = 0; bx < 8; ++bx)
			{
				const int ox = x + bx;
				if (ox >= 0 && ox < dest->w)
					((Uint8 *)dest->pixels)[oy * dest->pitch + ox] = data[(by * 8) + bx];	
			}
		}
	}
}

Style::Block::Block( Uint8 encoded_data[16] )
{
	decode(data, encoded_data, sizeof(data));
}

void Style::Block::decode( Uint8 *dest, const Uint8 *src, unsigned int size )
{
	int o = 0;
	for (int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < size; j += 4)
		{
			dest[i + j] = src[o++];
		}
	}
}

int Style::object_by_id( int type, unsigned int id ) const
{
	assert((unsigned)type < COUNTOF(this->object));
	
	for (vector<Object>::const_iterator i = object[type].begin(); i != object[type].end(); ++i)
	{
		if (i->id == id)
			return i - object[type].begin(); // index
	}
	
	return -1;
}

int Style::object_next_id(int type, unsigned int id) const
{
	assert((unsigned)type < COUNTOF(this->object));

	for (vector<Object>::const_iterator i = object[type].begin(); i != object[type].end(); ++i)
	{
		if (i->id == id) {
			i++;
			if (i == object[type].end())
				i = object[type].begin();
			return i->id;
		}	
	}
	return -1;
}

int Style::object_prev_id(int type, unsigned int id) const
{
	assert((unsigned)type < COUNTOF(this->object));

	for (vector<Object>::const_iterator i = object[type].begin(); i != object[type].end(); ++i)
	{
		if (i->id == id)
		{
			if (i == object[type].begin())
			{
				i = object[type].end();
			}
			i--;
			return i->id;
		}
	}
	return -1;
}

void Style::blit_object( SDL_Surface *surface, signed int x, signed int y, int type, unsigned int object, unsigned int frame ) const
{
	assert((unsigned)type < COUNTOF(this->object));
	
	if (object >= this->object[type].size())
		return assert(false);
	if (frame >= this->object[type][object].frame.size())
		return assert(false);
	
	int i = 0;
	
	for (int by = 0; by < this->object[type][object].height; ++by)
	{
		for (int bx = 0; bx < this->object[type][object].width; ++bx)
		{
			int b = this->object[type][object].frame[frame][i++];
			if (b != (Uint16)-1)
				block[type][b].blit(surface, x + bx * 8, y + by * 2);
		}
	}
}

void Style::draw_object_texture(Window * window, signed int x, signed int y, int type, unsigned int object, int zoom ) const
{
	assert((unsigned)type < COUNTOF(this->object));

	if (object >= this->object[type].size())
		return assert(false);

	SDL_Rect rdest;
	rdest.x = x*zoom;
	rdest.y = y*zoom;
	rdest.w = this->object[type][object].width * 8 * zoom;
	rdest.h = this->object[type][object].height * 2 * zoom;

	if (rdest.x > window->width || rdest.y > window->height || rdest.x + rdest.w < 0 || rdest.y + rdest.h < 0)
	{
		return;
	}

	SDL_Rect rsource;
	rsource.x = this->object[type][object].texX;
	rsource.y = this->object[type][object].texY;
	rsource.w = this->object[type][object].width * 8;
	rsource.h = this->object[type][object].height * 2;

	SDL_RenderCopy(window->screen_renderer, megatex, &rsource, &rdest);
}

bool Style::load(unsigned int n, Window * window, SDL_Color *pal2)
{
	const string path = "STYLES/";
	const string data = "DATA";
	const string perm = "PERM", temp = "TEMP";
	const string objec = "OBJEC";
	
	//The megatex is a big texture with all the graphics info.
	//So we just draw objects to the screen by drawing the section of the megatexture.
	//This should be faster than a separate texture for each object

	megatex = SDL_CreateTexture(window->screen_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, 1024, 1024);
	SDL_SetTextureBlendMode(megatex, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(megatex, 0);
	SDL_RenderCopy(window->screen_renderer, megatex, NULL, NULL);
	SDL_SetTextureAlphaMod(megatex, 255);
	megatexAddX = 0;
	megatexAddY = 0;
	megatexBiggestY = 0;
	
	return load_palette(path, data, n) &&
	       load_objects(PERM, path, perm, n) &&
	       load_blocks(PERM, path, perm, n) &&
	       load_objects(TEMP, path, temp, n) &&
	       load_blocks(TEMP, path, temp, n) &&
	       skill.load(path, objec, n) &&
		   create_object_textures (PERM, window, pal2) &&
		   create_object_textures(TEMP, window, pal2);
}

bool Style::load_palette( string path, string name, unsigned int n )
{
	const string pal = ".PAL";
	
	return load_palette(l3_filename(path, name, n, pal));
}

bool Style::load_palette( string pal_filename )
{
	ifstream pal_f(pal_filename.c_str(), ios::binary);
	if (!pal_f)
	{
		cerr << " failed to open '" << pal_filename << "'" << endl;
		return false;
	}
	
	for (int i = 0; i < 208; ++i)
	{
		Uint8 r, g, b;
		
		pal_f.read((char *)&r, sizeof(r));
		pal_f.read((char *)&g, sizeof(g));
		pal_f.read((char *)&b, sizeof(b));
		
		palette[i].r = (255.0f / 63.0f) * r;
		palette[i].g = (255.0f / 63.0f) * g;
		palette[i].b = (255.0f / 63.0f) * b;
	}
	palette[208].r = 255;
	palette[208].g = 0;
	palette[208].b = 255;//Delibertely add a colour to the end of the palette to represent transparency
	
	return true;
}

bool Style::load_objects( int type, const string &path, const string &name, unsigned int n)
{
	assert((unsigned)type < COUNTOF(this->object));
	
	const string obj = ".OBJ", frl = ".FRL";
	
	return load_objects(type, l3_filename(path, name, n, obj), l3_filename(path, name, n, frl));
}

bool Style::load_objects( int type, const string &obj_filename, const string &frl_filename )
{
	assert((unsigned)type < COUNTOF(this->object));
	
	object[type].clear();
	
	ifstream obj_f(obj_filename.c_str(), ios::binary);
	if (!obj_f)
	{
		cerr << "failed to open '" << obj_filename << "'" << endl;
		return false;
	}
	
	ifstream frl_f(frl_filename.c_str(), ios::binary);
	if (!frl_f)
	{
		cerr << "failed to open '" << frl_filename << "'" << endl;
		return false;
	}
	
	while (true)
	{
		Object o;
		
		Uint8 frames;
		
		obj_f.read((char *)&o.id,         sizeof(o.id));
		obj_f.read((char *)&o.unknown[0], sizeof(o.unknown[0]));
		obj_f.read((char *)&o.frl,        sizeof(o.frl));
		obj_f.read((char *)&o.unknown[1], sizeof(o.unknown[1]));
		obj_f.read((char *)&o.width,      sizeof(o.width));
		obj_f.read((char *)&o.height,     sizeof(o.height));
		obj_f.read((char *)&frames,       sizeof(frames));
		obj_f.read((char *)&o.unknown[2], sizeof(o.unknown[2]));
		obj_f.read((char *)&o.unknown[3], sizeof(o.unknown[3]));

		if (!obj_f)
			break;

		if (o.id == 10008 || o.id == 10009) //Don't load game-crashing unimplemented monster
		{
			continue;
		}
		
		for (int j = 0; j < frames; ++j)
		{
			Uint16 seek = o.frl + j * sizeof(seek);
			frl_f.seekg(seek, ios::beg);
			
			frl_f.read((char *)&seek, sizeof(seek));
			frl_f.seekg(seek, ios::beg);
			
			Uint8 type = -1;
			Uint16 blocks = 0;
			
			frl_f.read((char *)&type, sizeof(type));
			frl_f.read((char *)&blocks, sizeof(blocks));
			
			if (type == 0)
			{
				frl_f.read((char *)&seek, sizeof(seek));
				frl_f.seekg(seek, ios::beg);
			}
			
			Uint16 *frame = new Uint16[o.width * o.height];
			memset(frame, -1, o.width * o.height * sizeof(*frame));
			
			for (int k = 0; k < blocks; ++k)
			{
				int b;
				
				switch (type)
				{
				case 0:
					Uint8 x, y;
					frl_f.read((char *)&x, sizeof(x));
					frl_f.read((char *)&y, sizeof(y));
					
					b = x + y * o.width;
					break;
				case 1:
					b = k;
					break;
				default:
					assert(false);
					break;
				}
				
				frl_f.read((char *)&frame[b], sizeof(*frame));
			}
			
			if (!frl_f)
			{
				delete[] frame;
				
				cerr << "unexpected end-of-file '" << frl_filename << "'" << endl;
				return false;
			}
			
			o.frame.push_back(frame);
		}

		o.texX = NULL;
		o.texY = NULL;

		object[type].push_back(o);

	}
	
	cout << "loaded " << object[type].size() << " objects from '" << obj_filename << "'" << endl;
	return true;
}

bool Style::load_blocks( int type, const string &path, const string &name, unsigned int n)
{
	assert((unsigned)type < COUNTOF(this->object));
	
	const string blk = ".BLK";
	
	return load_blocks(type, l3_filename(path, name, n, blk));
}

bool Style::load_blocks( int type, const string &blk_filename )
{
	assert((unsigned)type < COUNTOF(this->object));
	
	block[type].clear();
	
	ifstream blk_f(blk_filename.c_str(), ios::binary);
	if (!blk_f)
	{
		cerr << "failed to open '" << blk_filename << "'" << endl;
		return false;
	}
	
	while (true)
	{
		Uint8 temp[16];
		
		blk_f.read((char *)&temp, sizeof(temp));
		
		if (!blk_f)
			break;
		
		Block b(temp);
		
		block[type].push_back(b);
	}
	
	cout << "loaded " << block[type].size() << " blocks from '" << blk_filename << "'" << endl;
	return true;
}

bool Style::create_object_textures (int type, Window * window, SDL_Color *pal2)
{
	assert((unsigned)type < COUNTOF(this->object));

	for (vector<Object>::iterator i = object[type].begin(); i != object[type].end(); ++i)
	{
		SDL_Surface *tempSurface;
		tempSurface = SDL_CreateRGBSurface(0, i->width*8, i->height*2, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors(tempSurface->format->palette, pal2, 0, 32);
		SDL_SetPaletteColors(tempSurface->format->palette, palette, 32, 209);
		SDL_FillRect(tempSurface, NULL, SDL_MapRGB(tempSurface->format, 255, 0, 255));//fill surface with dummy magenta colour to represent transparency
		int so = object_by_id(type, i->id);
		if (so == -1)
			continue;
		blit_object(tempSurface, 0, 0, type, so, 0);
		if (object[type][so].frame.size() > 1)
			blit_object(tempSurface, 0, 0, type, so, 1);

		//tempSurface now contains image in 8 bit colour depth format, and magenta for transparency

		SDL_Surface *tempSurface2;
		tempSurface2 = SDL_ConvertSurfaceFormat(tempSurface, SDL_PIXELFORMAT_RGB888, 0);//We need a surface format with higgher colour depth that can handle transparency
		SDL_SetColorKey(tempSurface2, SDL_TRUE, SDL_MapRGB(tempSurface2->format, 255, 0, 255));//Convert dummy magenta into transparency
		
		SDL_Texture *tempTexture;//Need to turn surface into a texture so can draw it with transparency onto our megatexture
		tempTexture = SDL_CreateTextureFromSurface(window->screen_renderer, tempSurface2);
		
		if (megatexAddX + (i->width*8) > 1024) {
			megatexAddX = 0;
			megatexAddY += megatexBiggestY;
			megatexBiggestY = 0;
		}

		SDL_Rect r;
		r.x = megatexAddX;
		r.y = megatexAddY;
		r.w = i->width * 8;
		r.h = i->height * 2;

		SDL_SetRenderTarget(window->screen_renderer, megatex);
		SDL_RenderCopy(window->screen_renderer, tempTexture, NULL, &r);
		SDL_SetRenderTarget(window->screen_renderer, NULL);

		i->texX = megatexAddX;
		i->texY = megatexAddY;
		megatexAddX += r.w;
		if ((i->height * 2) > megatexBiggestY)
			megatexBiggestY = i->height * 2;
		SDL_FreeSurface(tempSurface);
		SDL_FreeSurface(tempSurface2);
		SDL_DestroyTexture(tempTexture);
	}
	return true;
}

bool Style::destroy_all_objects(int type)
{
	assert((unsigned)type < COUNTOF(this->object));

	/*for (vector<Object>::const_iterator i = object[type].begin(); i != object[type].end(); ++i)
	{
		delete &i;
	}*/

	SDL_DestroyTexture( megatex );
	return true;
}