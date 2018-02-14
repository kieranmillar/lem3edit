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

#ifndef PACKEDITOR_HPP
#define PACKEDITOR_HPP

#include "../lem3edit.hpp"

#include "SDL.h"

#include <string>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem::v1;

class Ini;
class Editor;

class PackEditor
{
public:
	void setReferences(Ini * i, Editor * e);

	bool create(void);
	bool load(const fs::path fileName);
	bool save(void);

	//void createLevel(const int n, const tribeName tribe);
	//void loadLevel(const int n, const tribeName tribe);
	//void editLevel(const int n, const tribeName tribe);

	bool levelExists(const int id);//returns if level files exist and all match expected id

private:
	Ini * ini_ptr;
	Editor * editor_ptr;

	fs::path packPath;

	class levelData
	{
	public:
		levelData(std::string s, int n);

		std::string name;
		int lems;
	};

	std::vector<levelData> levels[TRIBECOUNT];

	int totalLems[TRIBECOUNT];
	void refreshLemCounts(void);
};

#endif // PACKEDITOR_HPP
