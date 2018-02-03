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
 This file contains code for reading from Lemmings 3's DEL files. These contain game fonts
 */

#include "del.hpp"
#include "lem3edit.hpp"
#include "style.hpp"
#include "window.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = std::experimental::filesystem::v1;

void Del::setReferences(Window * w, Style * s)
{
	window_ptr = w;
	style_ptr = s;
}

void Del::createFont(void)
{
	fontTex = SDL_CreateTexture(window_ptr->screen_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, 64, 64);
	SDL_SetTextureBlendMode(fontTex, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(fontTex, 255);
	fontTexAddX = 0;
	fontTexAddY = 0;

	for (unsigned int i = 30; i < frame.size(); i++)
	{
		SDL_Surface *tempSurface;
		tempSurface = SDL_CreateRGBSurface(0, 8, 8, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors(tempSurface->format->palette, style_ptr->palette, 0, 209);
		SDL_FillRect(tempSurface, NULL, SDL_MapRGB(tempSurface->format, 0, 0, 0));//fill surface with dummy white colour to represent transparency
		this->frame[i].blit(tempSurface, 0, 0, 8, 8);

		//tempSurface now contains image in 8 bit colour depth format, and magenta for transparency

		SDL_Surface *tempSurface2;
		tempSurface2 = SDL_ConvertSurfaceFormat(tempSurface, SDL_PIXELFORMAT_RGB888, 0);//We need a surface format with higher colour depth that can handle transparency
		SDL_SetColorKey(tempSurface2, SDL_TRUE, SDL_MapRGB(tempSurface2->format, 0, 0, 0));//Convert dummy white into transparency

		SDL_Texture *tempTexture;//Need to turn surface into a texture so can draw it with transparency onto our font texture
		tempTexture = SDL_CreateTextureFromSurface(window_ptr->screen_renderer, tempSurface2);

		if (fontTexAddX + 8 > 64) {
			fontTexAddX = 0;
			fontTexAddY += 8;
		}

		SDL_Rect r;
		r.x = fontTexAddX;
		r.y = fontTexAddY;
		r.w = 8;
		r.h = 8;

		SDL_SetRenderTarget(window_ptr->screen_renderer, fontTex);
		SDL_RenderCopy(window_ptr->screen_renderer, tempTexture, NULL, &r);
		SDL_SetRenderTarget(window_ptr->screen_renderer, NULL);

		fontTexAddX += 8;
		SDL_FreeSurface(tempSurface);
		SDL_FreeSurface(tempSurface2);
		SDL_DestroyTexture(tempTexture);
	}
}

void Del::Frame::copy(const Del::Frame &that)
{
	size = that.size;

	frame = new Uint8[size];
	memcpy(frame, that.frame, sizeof(Uint8) * size);
}

void Del::Frame::destroy(void)
{
	delete[] frame;
}

Del::Frame & Del::Frame::operator=(const Del::Frame &that)
{
	if (this == &that)
		return *this;

	destroy();
	copy(that);

	return *this;
}

void Del::Frame::blit(SDL_Surface *surface, signed int x, signed int y, unsigned int width, unsigned int height) const
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

void Del::blit(SDL_Surface *surface, signed int x, signed int y, unsigned int frame, unsigned int width, unsigned int height) const
{
	if (frame >= this->frame.size())
		return assert(false);

	this->frame[frame].blit(surface, x, y, width, height);
}

void Del::blit_text(SDL_Surface *surface, signed int x, signed int y, const string &text) const
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

bool Del::load(const fs::path basePath, const std::string name)
{
	const string folder = "GRAPHICS";
	const string din = ".DIN", del = ".DEL";

	return load(l3_filename_data(basePath, folder, name, din), l3_filename_data(basePath, folder, name, del));
}

bool Del::load(const fs::path basePath, const std::string &folder, const std::string &name, unsigned int n)
{
	const string din = ".DIN", del = ".DEL";

	return load(l3_filename_data(basePath, folder, name, n, din), l3_filename_data(basePath, folder, name, n, del));
}

bool Del::load(const fs::path din_filename, const fs::path del_filename)
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

	SDL_Log("Loaded %d images from '%s'\n", frame.size(), del_filename);

	return true;
}
