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
#include "lem3edit.hpp"
#include "level.hpp"
#include "style.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <experimental/filesystem>

using namespace std;
namespace fs = std::experimental::filesystem::v1;

Style::Object & Style::Object::operator=(const Style::Object &that)
{
	if (this == &that)
		return *this;

	destroy();
	copy(that);

	return *this;
}

void Style::Object::copy(const Style::Object &that)
{
	id = that.id;
	width = that.width;
	height = that.height;
	objTex = that.objTex;
	frl = that.frl;

	for (unsigned int i = 0; i < COUNTOF(unknown); ++i)
		unknown[i] = that.unknown[i];

	for (vector<Uint16 *>::const_iterator f = that.frame.begin(); f != that.frame.end(); ++f)
	{
		Uint16 *temp = new Uint16[width * height];
		memcpy(temp, *f, sizeof(Uint16) * width * height);
		frame.push_back(temp);
	}
}

void Style::Object::destroy(void)
{
	SDL_DestroyTexture(objTex);
	for (vector<Uint16 *>::const_iterator f = frame.begin(); f != frame.end(); ++f)
		delete[] * f;
}

void Style::Block::blit(SDL_Surface *dest, signed int x, signed int y) const
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

Style::Block::Block(Uint8 encoded_data[16])
{
	decode(data, encoded_data, sizeof(data));
}

void Style::Block::decode(Uint8 *dest, const Uint8 *src, unsigned int size)
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

int Style::object_by_id(int type, unsigned int id) const
{
	assert((unsigned)type < COUNTOF(this->object));

	vector<Object>::const_iterator i = std::find_if(object[type].begin(), object[type].end(), [&id](const Object& obj) {return obj.id == id; });

	if (i != object[type].end())
		return i - object[type].begin(); // index
	else
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

void Style::blit_object(SDL_Surface *surface, signed int x, signed int y, int type, unsigned int object, unsigned int frame) const
{
	assert((unsigned)type < COUNTOF(this->object));

	// There are 3 object arrays, but they were loaded from two files
	// We split up the PERM objects to separate out the tools and creatures
	// But the blocks that hold the grpahics info are all still in a single PERM list
	// So we need to redirect TOOL objects back to the PERM block list
	int blockType = type;
	if (type == TOOL)
		blockType = PERM;

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
				block[blockType][b].blit(surface, x + bx * 8, y + by * 2);
		}
	}
}

// NOTE TO SELF: Have this return the rectangle, not do the drawing itself! Put Get in the title
void Style::draw_object_texture(signed int x, signed int y, int type, unsigned int object, int zoom, int maxSize) const
{
	if (x > g_window.width || y > g_window.height)
		return;

	assert((unsigned)type < COUNTOF(this->object));

	if (object >= this->object[type].size())
		return assert(false);

	const Object *o = &this->object[type][object];

	SDL_Rect rdest;
	rdest.x = x;
	rdest.y = y;
	rdest.w = o->width * 8 * zoom;
	rdest.h = o->height * 2 * zoom;
	if (maxSize != 0) {
		if (max(rdest.w, rdest.h) > maxSize)
		{
			int factor = (max(rdest.w, rdest.h) * 100) / maxSize;
			rdest.w = (rdest.w * 100) / (factor);
			rdest.h = (rdest.h * 100) / (factor);
		}
	}

	if (x + rdest.w < 0 || y + rdest.h < 0)
		return;

	SDL_RenderCopy(g_window.screen_renderer, o->objTex, NULL, &rdest);
}

bool Style::load(unsigned int n, SDL_Color *pal2, fs::path basePath)
{
	const string folder = "STYLES";
	const string data = "DATA";
	const string perm = "PERM", temp = "TEMP";
	const string objec = "OBJEC";

	return load_palette(basePath, folder, data, n) &&
		load_objects(PERM, basePath, folder, perm, n) &&
		load_blocks(PERM, basePath, folder, perm, n) &&
		load_objects(TEMP, basePath, folder, temp, n) &&
		load_blocks(TEMP, basePath, folder, temp, n) &&
		skill.load(basePath, folder, objec, n) &&
		create_object_textures(PERM, pal2) &&
		create_object_textures(TEMP, pal2) &&
		create_object_textures(TOOL, pal2);
}

bool Style::load_palette(fs::path basePath, string folder, string name, unsigned int n)
{
	const string pal = "PAL";

	return load_palette(l3_filename_data(basePath.generic_string(), folder, name, n, pal));
}

bool Style::load_palette(fs::path pal_filename)
{
	ifstream pal_f(pal_filename, ios::binary);
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

bool Style::load_objects(int type, fs::path basePath, const string &folder, const string &name, unsigned int n)
{
	assert((unsigned)type < COUNTOF(this->object));

	const string obj = "OBJ", frl = "FRL";

	return load_objects(type, l3_filename_data(basePath.generic_string(), folder, name, n, obj), l3_filename_data(basePath.generic_string(), folder, name, n, frl));
}

bool Style::load_objects(int type, const fs::path obj_filename, const fs::path frl_filename)
{
	assert((unsigned)type < COUNTOF(this->object));

	object[type].clear();
	if (type == PERM)
		object[TOOL].clear();

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

		obj_f.read((char *)&o.id, sizeof(o.id));
		obj_f.read((char *)&o.unknown[0], sizeof(o.unknown[0]));
		obj_f.read((char *)&o.frl, sizeof(o.frl));
		obj_f.read((char *)&o.unknown[1], sizeof(o.unknown[1]));
		obj_f.read((char *)&o.width, sizeof(o.width));
		obj_f.read((char *)&o.height, sizeof(o.height));
		obj_f.read((char *)&frames, sizeof(frames));
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

			if (type == PERM)
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
				case PERM:
					Uint8 x, y;
					frl_f.read((char *)&x, sizeof(x));
					frl_f.read((char *)&y, sizeof(y));

					b = x + y * o.width;
					break;
				case TEMP:
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

		if (o.id < 5000)
		{
			object[type].push_back(o);
		}
		else
		{
			object[TOOL].push_back(o);
		}
	}
	if (type == PERM)
		SDL_Log("Loaded %d + %d objects from '%s'\n", object[type].size(), object[TOOL].size(), obj_filename.generic_string().c_str());
	if (type == TEMP)
		SDL_Log("Loaded %d objects from '%s'\n", object[type].size(), obj_filename.generic_string().c_str());
	return true;
}

bool Style::load_blocks(int type, fs::path basePath, const string &folder, const string &name, unsigned int n)
{
	assert((unsigned)type < COUNTOF(this->object));

	const string blk = "BLK";

	return load_blocks(type, l3_filename_data(basePath.generic_string(), folder, name, n, blk));
}

bool Style::load_blocks(int type, fs::path blk_filename)
{
	assert((unsigned)type < COUNTOF(this->object));

	block[type].clear();

	ifstream blk_f(blk_filename.c_str(), ios::binary);
	if (!blk_f)
	{
		SDL_Log("Failed to open '%s'\n", blk_filename.generic_string().c_str());
		return false;
	}

	int count = 0;
	while (true)
	{
		Uint8 temp[16];

		blk_f.read((char *)&temp, sizeof(temp));

		if (!blk_f)
			break;

		Block b(temp);

		block[type].push_back(b);
	}
	SDL_Log("Loaded %d blocks from '%s'\n", block[type].size(), blk_filename.generic_string().c_str());
	return true;
}

bool Style::create_object_textures(int type, SDL_Color *pal2)
{
	assert((unsigned)type < COUNTOF(this->object));

	for (vector<Object>::iterator i = object[type].begin(); i != object[type].end(); ++i)
	{
		SDL_Surface *tempSurface;
		tempSurface = SDL_CreateRGBSurface(0, i->width * 8, i->height * 2, 8, 0, 0, 0, 0);
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
		tempSurface2 = SDL_ConvertSurfaceFormat(tempSurface, SDL_PIXELFORMAT_RGB888, 0);//We need a surface format with higher colour depth that can handle transparency
		SDL_SetColorKey(tempSurface2, SDL_TRUE, SDL_MapRGB(tempSurface2->format, 255, 0, 255));//Convert dummy magenta into transparency

		object[type][so].objTex = SDL_CreateTextureFromSurface(g_window.screen_renderer, tempSurface2);

		SDL_FreeSurface(tempSurface);
		SDL_FreeSurface(tempSurface2);
	}
	return true;
}

bool Style::destroy_all_objects(int type)
{
	assert((unsigned)type < COUNTOF(this->object));

	if (object[type].empty())
		return true;

	for (vector<Object>::const_reverse_iterator i = object[type].rbegin(); i != object[type].rend(); ++i)
	{
		int j = object[type].rend() - i - 1;
		if (object[type][j].objTex != NULL)
			SDL_DestroyTexture(object[type][j].objTex);
	}

	object[type].clear();

	return true;
}
