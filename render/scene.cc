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

#include <glm/gtc/matrix_inverse.hpp>

#include "scene.h"

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

//	Object *ob = m_objects[m_active_object];

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
	}
}

void Scene::addObject(Object *object)
{
	m_objects.push_back(object);
	m_active_object = m_objects.size() - 1;
	Q_EMIT objectChanged();
}

void Scene::render(const glm::mat4 &MV, const glm::mat4 &P, const glm::vec3 &view_dir)
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
	int selected_object = -1, index = 0;

	for (auto &object : m_objects) {
		if (object->intersect(ray, min)) {
			selected_object = index;
		}

		++index;
	}

	if (selected_object != -1 && selected_object != m_active_object) {
		m_active_object = selected_object;
		Q_EMIT objectChanged();
	}
}

Object *Scene::currentObject()
{
	if (!m_objects.empty()) {
		return m_objects[m_active_object];
	}

	return nullptr;
}

void Scene::moveObjectX(double value)
{
	Object *ob = m_objects[m_active_object];
	glm::vec3 pos = ob->pos();
	pos.x = value;
	ob->setPos(pos);
	Q_EMIT updateViewport();
}

void Scene::moveObjectY(double value)
{
	Object *ob = m_objects[m_active_object];
	glm::vec3 pos = ob->pos();
	pos.y = value;
	ob->setPos(pos);
	Q_EMIT updateViewport();
}

void Scene::moveObjectZ(double value)
{
	Object *ob = m_objects[m_active_object];
	glm::vec3 pos = ob->pos();
	pos.z = value;
	ob->setPos(pos);
	Q_EMIT updateViewport();
}

void Scene::scaleObjectX(double value)
{
	Object *ob = m_objects[m_active_object];
	glm::vec3 scale = ob->scale();
	scale.x = value;
	ob->setScale(scale);
	Q_EMIT updateViewport();
}

void Scene::scaleObjectY(double value)
{
	Object *ob = m_objects[m_active_object];
	glm::vec3 scale = ob->scale();
	scale.y = value;
	ob->setScale(scale);
	Q_EMIT updateViewport();
}

void Scene::scaleObjectZ(double value)
{
	Object *ob = m_objects[m_active_object];
	glm::vec3 scale = ob->scale();
	scale.z = value;
	ob->setScale(scale);
	Q_EMIT updateViewport();
}

void Scene::rotateObjectX(double value)
{
	Object *ob = m_objects[m_active_object];
	glm::vec3 rot = ob->rotation();
	rot.x = value;
	ob->setRotation(rot);
	Q_EMIT updateViewport();
}

void Scene::rotateObjectY(double value)
{
	Object *ob = m_objects[m_active_object];
	glm::vec3 rot = ob->rotation();
	rot.y = value;
	ob->setRotation(rot);
	Q_EMIT updateViewport();
}

void Scene::rotateObjectZ(double value)
{
	Object *ob = m_objects[m_active_object];
	glm::vec3 rot = ob->rotation();
	rot.z = value;
	ob->setRotation(rot);
	Q_EMIT updateViewport();
}
