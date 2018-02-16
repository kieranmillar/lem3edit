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
This file handles the level pack editor
*/

#include "mainmenu.hpp"
#include "packeditor.hpp"
#include "../font.hpp"
#include "../ini.hpp"
#include "../tinyfiledialogs.h"

#include "SDL.h"
#include "SDL_ttf.h"

#include <fstream>
#include <string>
#include <vector>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem::v1;

PackEditor::PackEditor(void)
{
	TTF_Font * bigFont = TTF_OpenFont("./gfx/DejaVuSansMono.ttf", 30);
	classicTabTex = Font::createTextureFromString(bigFont, "CLASSIC");
	shadowTabTex = Font::createTextureFromString(bigFont, "SHADOW");
	egyptTabTex = Font::createTextureFromString(bigFont, "EGYPT");
	TTF_CloseFont(bigFont);

	TTF_Font * smallFont = TTF_OpenFont("./gfx/DejaVuSansMono.ttf", 20);
	for (int i = 0; i < 10; i++)
	{
		char digit[2] = "0";
		digit[0] += i;
		numbers[i] = Font::createTextureFromString(smallFont, digit);
	}
	addNewLevelTex = Font::createTextureFromString(smallFont, "Add New Level");
	loadLevelTex = Font::createTextureFromString(smallFont, "Load Level");
	totalLemsTex = Font::createTextureFromString(smallFont, "Total Lemmings:");
	quitTex = Font::createTextureFromString(smallFont, "Quit");
	TTF_CloseFont(smallFont);
}

void PackEditor::refreshTitleTexture(void)
{
	if (packTitleTex != NULL)
		SDL_DestroyTexture(packTitleTex);
	if (packPath.has_filename())
	{
		TTF_Font * bigFont = TTF_OpenFont("./gfx/DejaVuSansMono.ttf", 30);
		packTitleTex = Font::createTextureFromString(bigFont, packPath.stem().generic_string());
		TTF_CloseFont(bigFont);
	}
}

void PackEditor::setReferences(Ini * i, Editor * e)
{
	ini_ptr = i;
	editor_ptr = e;
}

void PackEditor::handlePackEditorEvents(SDL_Event event)
{
	Sint32 mouse_x_window, mouse_y_window;
	Uint8 mouse_state = SDL_GetMouseState(&mouse_x_window, &mouse_y_window);

	switch (event.type)
	{
	case SDL_WINDOWEVENT:
	{
		SDL_WindowEvent &e = event.window;

		if (e.event == SDL_WINDOWEVENT_RESIZED)
		{
			g_window.resize(e.data1, e.data2);
			redraw = true;
		}
		break;
	}
	case SDL_MOUSEMOTION:
	{
		break;
	}
	case SDL_MOUSEBUTTONDOWN://when pressed
	{
		SDL_MouseButtonEvent &e = event.button;

		if (e.button == SDL_BUTTON_LEFT)
		{
			int windowThird = g_window.width / 3;

			//swap tabs
			if (mouse_y_window > 36 && mouse_y_window < 76)
			{
				if (mouse_x_window > 4 && mouse_x_window < (4 + windowThird))
				{
					tribeTab = CLASSIC;
					redraw = true;
				}
				else if (mouse_x_window > (3 + windowThird) && mouse_x_window < (3 + (windowThird * 2)))
				{
					tribeTab = SHADOW;
					redraw = true;
				}
				else if (mouse_x_window > (g_window.width - windowThird - 2) && mouse_x_window < (g_window.width - 4))
				{
					tribeTab = EGYPT;
					redraw = true;
				}
			}

			if (mouse_y_window > 88 + (levels[tribeTab].size() * 30) && mouse_y_window < 114 + (levels[tribeTab].size() * 30))
			{
				//add new level button
				if (mouse_x_window > (windowThird - 100) && mouse_x_window < (windowThird + 100))
				{
					createLevel((levels[tribeTab].size() + 1) + (tribeTab * 100));
					redraw = true;
				}
			}

			//quit button
			if (mouse_x_window > 20 &&
				mouse_x_window < 120 &&
				mouse_y_window > g_window.height - 30 &&
				mouse_y_window < g_window.height - 4)
			{
				save();
				g_currentMode = MAINMENUMODE;
			}
		}
		break;
	}
	case SDL_KEYDOWN:
	{
		SDL_KeyboardEvent &e = event.key;

		switch (e.keysym.sym)
		{
		case SDLK_ESCAPE:
		case SDLK_q:
			save();
			g_currentMode = MAINMENUMODE;
			break;
		default:
			break;
		}
		break;
	}
	case SDL_USEREVENT:// stuff here happens every frame. Watch out, timer produces events on a separate thread to rest of program!
	{
		Uint32 ticksSinceLastFrame = SDL_GetTicks() - lastFrameTick;
		lastFrameTick = SDL_GetTicks();
		if (ticksSinceLastFrame <= 36 && ticksSinceLastFrame >= 30)
		{
			draw();
		}
		break;
	}
	default:
	{
		break;
	}
	}
}

bool PackEditor::create(void)
{
	char const * fileToSaveTo = NULL;
	fs::path defaultPath = fs::current_path();
	defaultPath /= "NewPack.l3pack";
	std::string defaultString = defaultPath.generic_string();
	char const * defaultChar = defaultString.c_str();
	char const * filterPatterns[1] = { "*.l3pack" };
	fileToSaveTo = tinyfd_saveFileDialog("Save New Level Pack", defaultChar, 1, filterPatterns, "Lem3Edit Level Pack (*.l3pack)");

	if (!fileToSaveTo)
		return false;

	packPath = fileToSaveTo;
	if (!packPath.has_extension())
		packPath += ".l3pack";

	//iterate through the folder looking for level or pack files
	for (auto& iter : fs::directory_iterator(packPath.parent_path()))
	{
		std::string s = iter.path().extension().generic_string();
		if (s == ".l3pack" || s == ".DAT" || s == ".OBS")
		{
			tinyfd_messageBox("Oh No!", "Sorry, but you can't make a new level pack in a folder already containing a pack, or any level files.\n\nIt is reccommended that you make a new diectory to store your level pack.\n\nTrust me, it's for your own good.", "ok", "error", 1);
			return false;
		}
	}

	std::ofstream packFile(packPath);
	packFile.close();

	clearLevels();
	refreshTitleTexture();

	version = CURRENTPACKFILEVERSION;
	tribeTab = CLASSIC;

	levels[0].emplace_back(levelData("Blah1", 2));
	levels[0].emplace_back(levelData("Blah2", 2));
	levels[0].emplace_back(levelData("Blah3", 2));

	g_currentMode = LEVELPACKMODE;
	redraw = true;
	ini_ptr->saveLastLoadedPack(packPath);
	return true;
}

bool PackEditor::load(const fs::path fileName)
{
	clearLevels();

	if (!fs::exists(fileName))
	{
		SDL_Log("Couldn't find level pack %s\n.", fileName.generic_string().c_str());
		return false;
	}

	packPath = fileName;
	fs::path packParent = packPath.parent_path();
	std::ifstream packFile(packPath, std::ios::out);

	if (!packFile.is_open())
	{
		SDL_Log("Couldn't open level pack %s\n.", fileName.generic_string().c_str());
		return false;
	}
	std::string line;
	std::string key;
	std::string value;

	int count = 1;

	while (getline(packFile, line))
	{
		std::string::size_type pos = line.find('=');
		if (pos != std::string::npos)
		{
			key = line.substr(0, pos);

			if (key == "VERSION")
			{
				//load pack version so we can handle backwards compatability if the format ever changes
				version = atoi(value.c_str());
				continue;
			}

			int id = atoi(key.c_str());
			value = line.substr(pos + 1);

			tribeName loadingTribe;
			if (id >= 1 && id <= 30)
				loadingTribe = CLASSIC;
			else if (id >= 101 && id <= 130)
				loadingTribe = SHADOW;
			else if (id >= 201 && id <= 230)
				loadingTribe = EGYPT;
			else
			{
				packFile.close();
				SDL_Log("Invalid pack file entry - Invalid id");
				//TODO: handle invalid pack file entry
				return false;
			}

			count++;
			if (id % 100 == 1)
				count = 1 + (loadingTribe * 100);

			if (id != count)
			{
				packFile.close();
				SDL_Log("Invalid pack file entry - Level id out of order");
				//TODO: handle invalid pack file entry
				return false;
			}

			if (!levelExists(id))
			{
				packFile.close();
				SDL_Log("Invalid pack file entry - Level does not have consistent file IDs");
				//TODO: handle level fies not matching id

				return false;
			}
			//TODO: Properly load lemming count instead of 0
			levels[loadingTribe].emplace_back(levelData(value, 1));
		}
	}

	packFile.close();

	refreshLemCounts();
	refreshTitleTexture();
	tribeTab = CLASSIC;

	g_currentMode = LEVELPACKMODE;
	redraw = true;
	ini_ptr->saveLastLoadedPack(packPath);
	return true;
}

bool PackEditor::save(void)
{
	std::ofstream packFile(packPath, std::ios::in | std::ios::trunc);
	if (!packFile.is_open())
	{
		SDL_Log("Failed to save pack file  %s\n.", packPath.generic_string().c_str());
		return false;
	}

	//always save as latest version at top of file
	packFile << "VERSION=" << CURRENTPACKFILEVERSION << std::endl;

	for (int i = 0; i < TRIBECOUNT; i++)
	{
		int count = 0;
		for (std::vector<levelData>::const_iterator iter = levels[i].begin(); iter != levels[i].end(); ++iter)
		{
			const levelData &data = *iter;
			count++;
			int id = (i * 100) + count;
			packFile << id << "=" << data.name << std::endl;
		}
	}
	packFile.close();
	return true;
}

void PackEditor::createLevel(const int n)
{
	fs::path parentPath = packPath.parent_path();
	//create blank LEVEL###.DAT file
	fs::path datPath = parentPath;
	datPath /= "LEVEL";
	datPath += l3_filename_number(n);
	datPath += ".DAT";

	Uint16 tribe;
	Uint16 cave_map, cave_raw;
	Uint16 temp, perm;
	Uint16 style;
	Uint16 width, height;
	Sint16 cameraX, cameraY;
	Uint16 time;
	Uint8  extra_lemmings;
	Uint8  unknown;
	Uint16 release_rate, release_delay;
	Uint16 enemies;

	if (n >= 1 && n <= 30) //CLASSIC
	{
		tribe = 4;
		style = 1;
	}
	else if (n >= 101 && n <= 130) //SHADOW
	{
		tribe = 10;
		style = 2;
	}
	else if (n >= 201 && n <= 230) //EGYPT
	{
		tribe = 5;
		style = 3;
	}
	else
	{
		SDL_Log("Invalid level id for making a level via pack editor");
		return;
	}

	cave_map = 0;
	cave_raw = 0;
	temp = n;
	perm = n;
	width = 320;
	height = 160;
	cameraX = 0;
	cameraY = 0;
	time = 420;
	extra_lemmings = 0;
	unknown = 2;
	release_rate = 23;
	release_delay = 46;
	enemies = 0;

	std::ofstream f(datPath, std::ios::binary | std::ios::trunc);
	if (!f)
	{
		SDL_Log("Failed to open '%s'\n", datPath.generic_string().c_str());
		return;
	}

	f.write((char *)&tribe, sizeof(tribe));
	f.write((char *)&cave_map, sizeof(cave_map));
	f.write((char *)&cave_raw, sizeof(cave_raw));
	f.write((char *)&temp, sizeof(temp));
	f.write((char *)&perm, sizeof(perm));
	f.write((char *)&style, sizeof(style));
	f.write((char *)&width, sizeof(width));
	f.write((char *)&height, sizeof(height));
	f.write((char *)&cameraX, sizeof(cameraX));
	f.write((char *)&cameraY, sizeof(cameraY));
	f.write((char *)&time, sizeof(time));
	f.write((char *)&extra_lemmings, sizeof(extra_lemmings));
	f.write((char *)&unknown, sizeof(unknown));
	f.write((char *)&release_rate, sizeof(release_rate));
	f.write((char *)&release_delay, sizeof(release_delay));
	f.write((char *)&enemies, sizeof(enemies));

	f.close();

	//create blank TEMP###.OBS file
	fs::path tempPath = parentPath;
	tempPath /= "TEMP";
	tempPath += l3_filename_number(n);
	tempPath += ".OBS";

	{
		std::ofstream f(tempPath, std::ios_base::binary | std::ios_base::in | std::ios_base::trunc);
		if (!f)
		{
			SDL_Log("Failed to open '%s'\n", tempPath.generic_string().c_str());
			return;
		}
		f.close();
	}

	//create blank PERM###.OBS file
	fs::path permPath = parentPath;
	permPath /= "PERM";
	permPath += l3_filename_number(n);
	permPath += ".OBS";

	{
		std::ofstream f(permPath, std::ios_base::binary | std::ios_base::in | std::ios_base::trunc);
		if (!f)
		{
			SDL_Log("Failed to open '%s'\n", permPath.generic_string().c_str());
			return;
		}
		f.close();
	}

	std::string name;
	name = "New Level ";
	name += std::to_string(n);
	levels[(n / 100)].emplace_back(levelData(name, 0));
}

bool PackEditor::levelExists(const int id)
{
	fs::path levelPath = packPath.parent_path();
	levelPath /= "LEVEL";
	levelPath += l3_filename_number(id);
	levelPath += ".DAT";
	bool datExists = fs::exists(levelPath);

	Mainmenu::OBSValues levelOBS = Mainmenu::loadOBSValues(levelPath);
	if (levelOBS.temp != id || levelOBS.perm != id)
	{
		return false;
	}

	fs::path tempPath = packPath.parent_path();
	tempPath /= "TEMP";
	tempPath += l3_filename_number(id);
	tempPath += ".OBS";
	bool tempExists = fs::exists(tempPath);

	fs::path permPath = packPath.parent_path();
	permPath /= "PERM";
	permPath += l3_filename_number(id);
	permPath += ".OBS";
	bool permExists = fs::exists(permPath);

	return datExists && tempExists && permExists;
}

PackEditor::levelData::levelData(const std::string s, const int n)
{
	name = s;
	lems = n;
	tex = NULL;
	refreshTexture();
}

void PackEditor::levelData::refreshTexture(void)
{
	if (tex != NULL)
		SDL_DestroyTexture(tex);
	TTF_Font * smallFont = TTF_OpenFont("./gfx/DejaVuSansMono.ttf", 20);
	tex = Font::createTextureFromString(smallFont, name);
	TTF_CloseFont(smallFont);
}

void PackEditor::clearLevels(void)
{
	for (int i = 0; i < TRIBECOUNT; i++)
	{
		for (std::vector<levelData>::iterator iter = levels[i].begin(); iter != levels[i].end(); ++iter)
		{
			levelData &data = *iter;
			if (data.tex != NULL)
				SDL_DestroyTexture(data.tex);
		}
		levels[i].clear();
		totalLems[i] = 20;
	}
}

void PackEditor::refreshLemCounts(void)
{
	for (int i = 0; i < TRIBECOUNT; i++)
	{
		int count = 20;
		for (std::vector<levelData>::const_iterator iter = levels[i].begin(); iter != levels[i].end(); ++iter)
		{
			const levelData &data = *iter;

			count += data.lems;
		}
		totalLems[i] = count;
	}
}

void PackEditor::draw(void)
{
	if (redraw == false)
		return;

	int centreX = g_window.width / 2;
	int windowThird = g_window.width / 3;
	SDL_SetRenderDrawBlendMode(g_window.screen_renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(g_window.screen_renderer, g_window.screen_texture);
	SDL_SetRenderDrawColor(g_window.screen_renderer, 240, 240, 240, 255);
	SDL_RenderClear(g_window.screen_renderer);

	//title and tabs
	{
		renderText(packTitleTex, centreX, 4, CENTRE, 0);
		SDL_Rect r;

		r.x = 4;
		r.y = 36;
		r.w = windowThird;
		r.h = 40;
		SDL_SetRenderDrawColor(g_window.screen_renderer, 80, 200, 70, 255);
		SDL_RenderFillRect(g_window.screen_renderer, &r);
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		if (tribeTab == CLASSIC)
		{
			SDL_SetRenderDrawColor(g_window.screen_renderer, 80, 200, 70, 255);
			SDL_RenderDrawLine(g_window.screen_renderer, 5, 75, (2 + windowThird), 75);
		}
		renderText(classicTabTex, (g_window.width / 6), 40, CENTRE, 0);

		r.x = 3 + windowThird;
		r.y = 36;
		r.w = windowThird;
		r.h = 40;
		SDL_SetRenderDrawColor(g_window.screen_renderer, 130, 140, 200, 255);
		SDL_RenderFillRect(g_window.screen_renderer, &r);
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		if (tribeTab == SHADOW)
		{
			SDL_SetRenderDrawColor(g_window.screen_renderer, 130, 140, 200, 255);
			SDL_RenderDrawLine(g_window.screen_renderer, 4 + windowThird, 75, (1 + (windowThird * 2)), 75);
		}
		renderText(shadowTabTex, centreX, 40, CENTRE, 0);

		r.x = (g_window.width - windowThird - 2);
		r.y = 36;
		r.w = windowThird;
		r.h = 40;
		SDL_SetRenderDrawColor(g_window.screen_renderer, 250, 230, 120, 255);
		SDL_RenderFillRect(g_window.screen_renderer, &r);
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		if (tribeTab == EGYPT)
		{
			SDL_SetRenderDrawColor(g_window.screen_renderer, 250, 230, 120, 255);
			SDL_RenderDrawLine(g_window.screen_renderer, g_window.width - windowThird - 1, 75, (g_window.width - 4), 75);
		}
		renderText(egyptTabTex, (g_window.width / 6) * 5, 40, CENTRE, 0);

		r.x = 4;
		r.y = 76;
		r.w = g_window.width - 7;
		r.h = g_window.height - 111;
		if (tribeTab == CLASSIC)
			SDL_SetRenderDrawColor(g_window.screen_renderer, 80, 200, 70, 255);
		if (tribeTab == SHADOW)
			SDL_SetRenderDrawColor(g_window.screen_renderer, 130, 140, 200, 255);
		if (tribeTab == EGYPT)
			SDL_SetRenderDrawColor(g_window.screen_renderer, 250, 230, 120, 255);
		SDL_RenderFillRect(g_window.screen_renderer, &r);
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawLine(g_window.screen_renderer, r.x, r.y, r.x, r.y + r.h); //left border
		SDL_RenderDrawLine(g_window.screen_renderer, r.x, r.y + r.h, r.x + r.w, r.y + r.h); //bottom border
		SDL_RenderDrawLine(g_window.screen_renderer, r.x + r.w, r.y, r.x + r.w, r.y + r.h); // right border
	}

	//levels
	{
		int yPos = 90;
		int count = 1;
		for (std::vector<levelData>::const_iterator iter = levels[tribeTab].begin(); iter != levels[tribeTab].end(); ++iter)
		{
			const levelData &d = *iter;

			SDL_Rect r;
			r.x = 8;
			r.y = (yPos - 2);
			r.w = (g_window.width - 300);
			r.h = 26;
			SDL_SetRenderDrawColor(g_window.screen_renderer, 230, 230, 230, 255);
			SDL_RenderFillRect(g_window.screen_renderer, &r);
			SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
			SDL_RenderDrawRect(g_window.screen_renderer, &r);
			SDL_RenderDrawLine(g_window.screen_renderer, g_window.width - 340, r.y, g_window.width - 340, r.y + r.h - 1);

			renderNumbers(count, 35, yPos);
			renderText(d.tex, 45, yPos, LEFT, g_window.width - 390);
			renderNumbers(d.lems, g_window.width - 305, yPos);

			yPos += 30;
			count++;
		}
		if (count < 30)
		{
			SDL_Rect r;
			r.x = windowThird - 100;
			r.y = (yPos - 2);
			r.w = 200;
			r.h = 26;
			SDL_SetRenderDrawColor(g_window.screen_renderer, 230, 230, 230, 255);
			SDL_RenderFillRect(g_window.screen_renderer, &r);
			SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
			SDL_RenderDrawRect(g_window.screen_renderer, &r);
			renderText(addNewLevelTex, windowThird, yPos, CENTRE, 0);

			r.x = (windowThird * 2) - 100;
			r.y = (yPos - 2);
			r.w = 200;
			r.h = 26;
			SDL_SetRenderDrawColor(g_window.screen_renderer, 230, 230, 230, 255);
			SDL_RenderFillRect(g_window.screen_renderer, &r);
			SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
			SDL_RenderDrawRect(g_window.screen_renderer, &r);
			renderText(loadLevelTex, windowThird * 2, yPos, CENTRE, 0);
		}
	}

	//bottom bar
	{
		SDL_Rect r;
		r.x = 20;
		r.y = g_window.height - 30;
		r.w = 100;
		r.h = 26;
		SDL_SetRenderDrawColor(g_window.screen_renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(g_window.screen_renderer, &r);
		renderText(quitTex, 70, g_window.height - 30, CENTRE, 0);

		renderText(totalLemsTex, g_window.width - 530, g_window.height - 30, LEFT, 0);
		renderNumbers(totalLems[tribeTab], g_window.width - 305, g_window.height - 30);
	}

	SDL_SetRenderTarget(g_window.screen_renderer, NULL);
	SDL_RenderCopy(g_window.screen_renderer, g_window.screen_texture, NULL, NULL);
	SDL_RenderPresent(g_window.screen_renderer);

	redraw = false;
}

void PackEditor::renderText(SDL_Texture * tex, const int x, const int topY, renderAlign align, const int restrictWidth)
{
	int textW, textH;
	SDL_Rect sourceRect;
	SDL_Rect destinationRect;
	SDL_QueryTexture(tex, NULL, NULL, &textW, &textH);

	sourceRect.x = 0;
	sourceRect.y = 0;
	sourceRect.w = textW;
	sourceRect.h = textH;

	if (align == CENTRE)
		destinationRect.x = x - (textW / 2);
	else
		destinationRect.x = x;
	destinationRect.y = topY;
	destinationRect.w = textW;
	destinationRect.h = textH;

	if (restrictWidth > 0)
	{
		if (textW > restrictWidth)
			sourceRect.w = destinationRect.w = restrictWidth;
	}
	SDL_RenderCopy(g_window.screen_renderer, tex, &sourceRect, &destinationRect);
}

void PackEditor::renderNumbers(int num, const int rightX, const int y)
{
	int numChars[3];
	int size = 0;
	if (num == 0)
	{
		size = 1;
		numChars[0] = 0;
	}
	else
	{
		while (num != 0)
		{
			//load number characters into array in reverse order
			numChars[size] = num % 10;
			num /= 10;
			size++;
		}
	}

	int textW, textH, drawX;
	SDL_Rect textRect;
	drawX = rightX;
	for (int i = 0; i < size; i++)
	{
		SDL_QueryTexture(numbers[numChars[i]], NULL, NULL, &textW, &textH);
		drawX -= textW;
		textRect.x = drawX;
		textRect.y = y;
		textRect.w = textW;
		textRect.h = textH;
		SDL_RenderCopy(g_window.screen_renderer, numbers[numChars[i]], NULL, &textRect);
	}
}
