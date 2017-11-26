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
#ifndef STYLE_HPP
#define STYLE_HPP

#include "cmp.hpp"
#include "lem3edit.hpp"
#include "tribe.hpp"
#include "window.hpp"

#include "SDL.h"

#include <string>
#include <vector>

class Style
{
public:
	class Object
	{
	public:
		Uint16 id;
		Uint8 width, height;
		Uint16 texX, texY;
		
		Uint16 frl, unknown[4];
		
		std::vector<Uint16 *> frame;
		
		Object( void ) { /* nothing to do */ }
		Object( const Object &that ) { copy(that); }
		~Object( void ) { destroy(); }
		
		void copy( const Object & );
		void destroy( void );
		Object & operator=( const Object & );
	};
	
	class Block
	{
	public:
		Uint8 data[16];
		
		void blit( SDL_Surface *dest, signed int x, signed int y ) const;
		
		Block( Uint8 encoded_data[16] );
		
		static void decode( Uint8 *dest, const Uint8 *src, unsigned int size );
	};
	
	std::vector<Object> object[3];
	
	std::vector<Block> block[2];
	
	Cmp skill;
	
	SDL_Color palette[209];

	SDL_Texture *megatex;

	Uint16 megatexAddX, megatexAddY;
	Uint16 megatexBiggestY;
	
	signed int object_by_id( int type, unsigned int id ) const;
	signed int object_next_id(int type, unsigned int id) const;
	signed int object_prev_id(int type, unsigned int id) const;
	
	void blit_object( SDL_Surface * surface, signed int x, signed int y, int type, unsigned int object, unsigned int frame ) const;
	void draw_object_texture(Window * window, signed int x, signed int y, int type, unsigned int object, int zoom, int maxSize) const;

	bool load(unsigned int n, Window * window, SDL_Color *pal2);
	bool load_palette( std::string path, std::string name, unsigned int n );
	bool load_palette( std::string pal_filename );
	bool load_objects( int type, const std::string &path, const std::string &name, unsigned int n);
	bool load_objects( int type, const std::string &obj_filename, const std::string &frl_filename );
	bool load_blocks( int type, const std::string &path, const std::string &name, unsigned int n );
	bool load_blocks( int type, const std::string &blk_filename );
	bool create_object_textures( int type, Window * window, SDL_Color *pal2 );
	bool destroy_all_objects( int type );
	
	Style( void ) { /* nothing to do */ }
	
private:
	Style( const Style & );
	Style & operator=( const Style & );
};

#endif // STYLE_HPP