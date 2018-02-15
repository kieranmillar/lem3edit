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
This file contains code for reading from and writing to the ini file
*/

#include "ini.hpp"
#include "lem3edit.hpp"

#include "SDL.h"

#include <experimental/filesystem>
#include <fstream>
#include <string>

namespace fs = std::experimental::filesystem::v1;

bool Ini::load(void)
{
	//set defaults
	lem3cdPath = "";
	lem3installPath = "";
	dosBoxPath = "";
	lastLoadedPack = "";

	fs::path iniPath = fs::current_path();
	iniPath /= "lem3edit.ini";
	if (fs::exists(iniPath))
	{
		std::ifstream iniFile(iniPath, std::ios::out);

		if (iniFile.is_open())
		{
			std::string line;
			std::string key;
			std::string value;

			while (getline(iniFile, line))
			{
				std::string::size_type pos = line.find('=');
				if (pos != std::string::npos)
				{
					key = line.substr(0, pos);
					value = line.substr(pos + 1);
					if (key == "CD")
						lem3cdPath = value;
					if (key == "INSTALL")
						lem3installPath = value;
					if (key == "DOSBOX")
						dosBoxPath = value;
					if (key == "LASTPACK")
						lastLoadedPack = value;
				}
			}

			iniFile.close();
		}
		else
		{
			SDL_Log("Failed to open ini file.");
			return false;
		}
	}
	else
	{
		SDL_Log("ini file doesn't exist.");
		return false;
	}
	return true;
}

bool Ini::validateData(void) {
	return validateData(lem3cdPath.parent_path());
}

//check that every data file the program might need exists and return if successful
bool Ini::validateData(const fs::path parentPath) {
	bool success = true;

	fs::path l3cdPath = parentPath;
	l3cdPath /= "L3CD.EXE";
	if (!fs::exists(l3cdPath))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TRIBE", 4, "PAL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TRIBE", 5, "PAL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TRIBE", 10, "PAL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TRIBE", 4, "IND")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TRIBE", 5, "IND")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TRIBE", 10, "IND")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TRIBE", 4, "CMP")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TRIBE", 5, "CMP")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TRIBE", 10, "CMP")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TPANL", 4, "DIN")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TPANL", 5, "DIN")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TPANL", 10, "DIN")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TPANL", 4, "DEL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TPANL", 5, "DEL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "GRAPHICS", "TPANL", 10, "DEL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "DATA", 1, "PAL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "DATA", 2, "PAL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "DATA", 3, "PAL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "DATA", 1, "PAL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "DATA", 2, "PAL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "DATA", 3, "PAL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "PERM", 1, "OBJ")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "PERM", 2, "OBJ")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "PERM", 3, "OBJ")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "PERM", 1, "BLK")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "PERM", 2, "BLK")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "PERM", 3, "BLK")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "PERM", 1, "FRL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "PERM", 2, "FRL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "PERM", 3, "FRL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "TEMP", 1, "OBJ")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "TEMP", 2, "OBJ")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "TEMP", 3, "OBJ")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "TEMP", 1, "BLK")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "TEMP", 2, "BLK")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "TEMP", 3, "BLK")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "TEMP", 1, "FRL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "TEMP", 2, "FRL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "TEMP", 3, "FRL")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "OBJEC", 1, "CMP")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "OBJEC", 2, "CMP")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "OBJEC", 3, "CMP")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "OBJEC", 1, "IND")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "OBJEC", 2, "IND")))
		success = false;
	if (!fs::exists(l3_filename_data(parentPath, "STYLES", "OBJEC", 3, "IND")))
		success = false;

	return success;
}

void Ini::saveLem3cdPath(fs::path p) {
	lem3cdPath = p;
	save();
}

void Ini::saveLem3installPath(fs::path p) {
	lem3installPath = p;
	save();
}

void Ini::saveDosBoxPath(fs::path p) {
	dosBoxPath = p;
	save();
}

void Ini::saveLastLoadedPack(fs::path p) {
	lastLoadedPack = p;
	save();
}

bool Ini::save(void)
{
	fs::path iniPath = fs::current_path();
	iniPath /= "lem3edit.ini";

	std::ofstream iniFile(iniPath, std::ios::in | std::ios::trunc);
	if (iniFile.is_open())
	{
		iniFile << "CD=" << lem3cdPath.generic_string() << "\n";
		iniFile << "INSTALL=" << lem3installPath.generic_string() << "\n";
		iniFile << "DOSBOX=" << dosBoxPath.generic_string() << "\n";
		iniFile << "LASTPACK=" << lastLoadedPack.generic_string() << "\n";
		iniFile.close();
	}
	else
	{
		SDL_Log("Failed to save ini file.");
		return false;
	}
	return true;
}
