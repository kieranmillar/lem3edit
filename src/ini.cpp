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

#include "SDL.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::experimental::filesystem::v1;

bool Ini::load(void)
{
	//set defaults
	setLem3cdPath("");
	setLem3installPath("");
	setDosBoxPath("");
	setLastLoadedPack("");

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
						setLem3cdPath(value);
					if (key == "INSTALL")
						setLem3installPath(value);
					if (key == "DOSBOX")
						setDosBoxPath(value);
					if (key == "LASTPACK")
						setLastLoadedPack(value);
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

bool Ini::save(void)
{
	fs::path iniPath = fs::current_path();
	iniPath /= "lem3edit.ini";

	std::ofstream iniFile(iniPath, std::ios::in | std::ios::trunc);
	if (iniFile.is_open())
	{
		iniFile << "CD=" << getLem3cdPath() << "\n";
		iniFile << "INSTALL=" << getLem3installPath() << "\n";
		iniFile << "DOSBOX=" << getDosBoxPath() << "\n";
		iniFile << "LASTPACK=" << getLastLoadedPack() << "\n";
		iniFile.close();
	}
	else
	{
		SDL_Log("Failed to save ini file.");
		return false;
	}
	return true;
}

fs::path Ini::getLem3cdPath(void) {
	return lem3cdPath;
}

fs::path Ini::getLem3installPath(void) {
	return lem3installPath;
}

fs::path Ini::getDosBoxPath(void) {
	return dosBoxPath;
}

fs::path Ini::getLastLoadedPack(void) {
	return lastLoadedPack;
}

void Ini::setLem3cdPath(fs::path p) {
	lem3cdPath = p;
}

void Ini::setLem3installPath(fs::path p) {
	lem3installPath = p;
}

void Ini::setDosBoxPath(fs::path p) {
	dosBoxPath = p;
}

void Ini::setLastLoadedPack(fs::path p) {
	lastLoadedPack = p;
}
