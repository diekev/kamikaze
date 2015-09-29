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

#include <QKeyEvent>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "scene.h"

#include "render/GPUProgram.h"
#include "objects/levelset.h"
#include "objects/volume.h"

Scene::Scene()
    : m_volume(nullptr)
    , m_level_set(nullptr)
{}

Scene::~Scene()
{
	delete m_level_set;
	delete m_volume;
}

void Scene::keyboardEvent(int key)
{
	switch (key) {
		case Qt::Key_Minus:
			m_volume->changeNumSlicesBy(-1);
			break;
		case Qt::Key_Plus:
			m_volume->changeNumSlicesBy(1);
			break;
		case Qt::Key_L:
			m_volume->toggleUseLUT();
			break;
		case Qt::Key_B:
			m_volume->toggleBBoxDrawing();
			break;
		case Qt::Key_T:
			m_volume->toggleTopologyDrawing();
			break;
	}
}

void Scene::add_volume(Volume *volume)
{
	m_volume = volume;
}

void Scene::add_level_set(LevelSet *level_set)
{
	m_level_set = level_set;
}

void Scene::render(const glm::vec3 &view_dir, const glm::mat4 &MV, const glm::mat4 &P)
{
	const auto &MVP = P * MV;

	if (m_volume != nullptr) {
		m_volume->render(view_dir, MVP);
	}

	if (m_level_set != nullptr) {
		m_level_set->render(MVP, glm::inverseTranspose(glm::mat3(MV)));
	}
}
