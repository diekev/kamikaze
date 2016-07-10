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

	Q_EMIT(objectChanged());
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

	Q_EMIT(objectAdded(object));
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
		Q_EMIT(objectChanged());
	}
}

Object *Scene::currentObject()
{
	if (!m_objects.empty()) {
		return m_active_object;
	}

	return nullptr;
}

void Scene::setObjectName(const QString &name)
{
	/* Need to make a copy of the string, since the slot signature has to match
     * the signal signature (const QString &) */
	QString copy(name);

	bool name_changed = ensureUniqueName(copy);

	if (name_changed) {
		m_active_object->name(copy);
		Q_EMIT(objectChanged()); // XXX - hack to update the tab and outliner
	}
	else {
		m_active_object->name(name);
	}
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
}

void Scene::emitNodeAdded(Object *ob, Node *node)
{
	Q_EMIT(nodeAdded(ob, node));
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
	Q_EMIT(objectChanged());
}

void Scene::updateForNewFrame(const EvaluationContext * const context)
{
	m_depsgraph->evaluate(context);
}

const std::vector<Object *> &Scene::objects() const
{
	return m_objects;
}
