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

#include "scene.h"

#include <glm/gtc/matrix_transform.hpp>

#include <kamikaze/nodes.h>
#include <kamikaze/primitive.h>

#include "object.h"

#include "graphs/depsgraph.h"

#include "util/util_string.h"

Scene::Scene()
    : m_active_object(nullptr)
    , m_depsgraph(new Depsgraph)
    , m_start_frame(0)
    , m_end_frame(250)
    , m_cur_frame(0)
{}

Scene::~Scene()
{
	for (auto &object : m_objects) {
		delete object;
	}

	delete m_depsgraph;
}

void Scene::removeObject(Object *object)
{
	auto iter = std::find(m_objects.begin(), m_objects.end(), object);

	assert(iter != m_objects.end());

	m_objects.erase(iter);
	m_depsgraph->remove_node(object);
	delete object;

	if (object == m_active_object) {
		m_active_object = nullptr;
	}

	notify_listeners(OBJECT_ADDED);
}

void Scene::addObject(Object *object)
{
	QString name = object->name();
	if (ensureUniqueName(name)) {
		object->name(name);
	}

	m_objects.push_back(object);
	m_active_object = object;

	m_depsgraph->create_node(object);

	notify_listeners(OBJECT_ADDED);
}

void Scene::intersect(const Ray &/*ray*/)
{
}

/* Select the object which is closest to pos. */
void Scene::selectObject(const glm::vec3 &pos)
{
	float min = 1000.0f;
	int selected_object = -1, index = 0;

	for (auto &object : m_objects) {
		if (!object || !object->collection()) {
			continue;
		}

		for (const auto &prim : object->collection()->primitives()) {
			float dist = glm::distance(prim->pos(), pos);

			if (/*dist < 1.0f &&*/ dist < min) {
				selected_object = index;
				min = dist;
			}
		}

		++index;
	}

	if (selected_object != -1 && m_active_object != m_objects[selected_object]) {
		m_active_object = m_objects[selected_object];
		notify_listeners(OBJECT_SELECTED);
	}
}

Object *Scene::currentObject()
{
	if (!m_objects.empty()) {
		return m_active_object;
	}

	return nullptr;
}

void Scene::tagObjectUpdate()
{
	if (!m_active_object) {
		return;
	}

	m_active_object->updateMatrix();

	if (m_active_object->collection()) {
		for (auto &prim : m_active_object->collection()->primitives()) {
			prim->tagUpdate();
		}
	}

	notify_listeners(OBJECT_MODIFIED);
}

bool Scene::ensureUniqueName(QString &name) const
{
	return ensure_unique_name(name, [&](const QString &str)
	{
		for (const auto &object : m_objects) {
			if (object->name() == str) {
				return false;
			}
		}

		return true;
	});
}

void Scene::setActiveObject(Object *object)
{
	m_active_object = object;
	notify_listeners(OBJECT_SELECTED);
}

void Scene::updateForNewFrame(const EvaluationContext * const context)
{
	m_depsgraph->evaluate_for_time_change(context);
}

void Scene::evalObjectDag(const EvaluationContext * const context, Object *object)
{
	m_depsgraph->evaluate(context, object);
}

int Scene::startFrame() const
{
	return m_start_frame;
}

void Scene::startFrame(int value)
{
	m_start_frame = value;
	notify_listeners(TIME_CHANGED);
}

int Scene::endFrame() const
{
	return m_end_frame;
}

void Scene::endFrame(int value)
{
	m_end_frame = value;
	notify_listeners(TIME_CHANGED);
}

int Scene::currentFrame() const
{
	return m_cur_frame;
}

void Scene::currentFrame(int value)
{
	m_cur_frame = value;
	notify_listeners(TIME_CHANGED);
}

float Scene::framesPerSecond() const
{
	return m_fps;
}

void Scene::framesPerSecond(float value)
{
	m_fps = value;
	notify_listeners(TIME_CHANGED);
}

const std::vector<Object *> &Scene::objects() const
{
	return m_objects;
}
