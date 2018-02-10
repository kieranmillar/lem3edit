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
#ifndef INI_HPP
#define INI_HPP

#include <string>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem::v1;

class Ini
{
public:
	bool load(void);
	bool save(void);

	bool validateData(void);

	fs::path getLem3cdPath(void);
	fs::path getLem3installPath(void);
	fs::path getDosBoxPath(void);
	fs::path getLastLoadedPack(void);

	void setLem3cdPath(fs::path p);
	void setLem3installPath(fs::path p);
	void setDosBoxPath(fs::path p);
	void setLastLoadedPack(fs::path p);

private:
	fs::path lem3cdPath;
	fs::path lem3installPath;
	fs::path dosBoxPath;
	fs::path lastLoadedPack;
};

#endif // INI_HPP
