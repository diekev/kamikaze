/*
 * ***** BEGIN GPL LICENSE BLOCK *****
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
 * along with this program; if not, write to the Free Software  Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#pragma once

#include <glm/glm.hpp>

class LevelSet;
class Volume;

class Scene {
	std::vector<Volume *> m_volumes;
	std::vector<LevelSet *> m_level_sets;
	Volume *m_volume;
	LevelSet *m_level_set;

public:
	Scene();
	~Scene();

	void keyboardEvent(int key);

	void add_volume(Volume *volume);
	void add_level_set(LevelSet *level_set);
	void render(const glm::vec3 &view_dir, const glm::mat4 &MV, const glm::mat4 &P);
};
