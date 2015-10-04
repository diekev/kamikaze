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
    : m_active_object(-1)
{}

Scene::~Scene()
{
	for (auto &object : m_objects) {
		delete object;
	}
}

void Scene::keyboardEvent(int key)
{
	if (m_objects.size() == 0) {
		return;
	}

	Object *ob = m_objects[m_active_object];

	switch (key) {
//		case Qt::Key_Minus:
//			m_volume->changeNumSlicesBy(-1);
//			break;
//		case Qt::Key_Plus:
//			m_volume->changeNumSlicesBy(1);
//			break;
//		case Qt::Key_L:
//			m_volume->toggleUseLUT();
//			break;
		case Qt::Key_B:
			ob->toggleBBoxDrawing();
			break;
		case Qt::Key_T:
			ob->toggleTopologyDrawing();
			break;
	}
}

void Scene::add_object(Object *object)
{
	m_objects.push_back(object);
	m_active_object = m_objects.size() - 1;
}

void Scene::render(const glm::vec3 &view_dir, const glm::mat4 &MV, const glm::mat4 &P)
{
	const auto &MVP = P * MV;
	const auto &N = glm::inverseTranspose(glm::mat3(MV));

	for (auto &object : m_objects) {
		object->render(MVP, N, view_dir);
	}
}

void Scene::intersect(const Ray &ray)
{
	float min = std::numeric_limits<float>::max();
	int selected_volume = -1, index = 0;

	for (auto &object : m_objects) {
		if (object->intersect(ray, min)) {
			selected_volume = index;
		}

		++index;
	}

	if (selected_volume != -1) {
		m_active_object = selected_volume;
	}
}
