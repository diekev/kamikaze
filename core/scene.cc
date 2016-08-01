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
#include "simulation.h"

#include "graphs/depsgraph.h"

#include "util/util_string.h"

Scene::Scene()
    : m_depsgraph(new Depsgraph)
{}

Scene::~Scene()
{
	for (auto &node : m_nodes) {
		delete node;
	}

	delete m_depsgraph;
}

void Scene::removeObject(SceneNode *node)
{
	auto iter = std::find(m_nodes.begin(), m_nodes.end(), node);

	assert(iter != m_nodes.end());

	notify_listeners(OBJECT_REMOVED);

	m_nodes.erase(iter);
	m_depsgraph->remove_node(node);
	delete node;

	if (node == m_active_node) {
		m_active_node = nullptr;
	}
}

void Scene::addObject(SceneNode *node)
{
	QString name = node->name();
	if (ensureUniqueName(name)) {
		node->name(name);
	}

	m_nodes.push_back(node);
	m_active_node = node;

	m_depsgraph->create_node(node);

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

	for (auto &node : m_nodes) {
		if (node->type() != SCE_NODE_OBJECT) {
			continue;
		}

		auto object = static_cast<Object *>(node);

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

	if (selected_object != -1 && m_active_node != m_nodes[selected_object]) {
		m_active_node = m_nodes[selected_object];
		notify_listeners(OBJECT_SELECTED);
	}
}

Depsgraph *Scene::depsgraph()
{
	return m_depsgraph;
}

SceneNode *Scene::current_node()
{
	if (!m_nodes.empty()) {
		return m_active_node;
	}

	return nullptr;
}

void Scene::tagObjectUpdate()
{
	if (!m_active_node || m_active_node->type() != SCE_NODE_OBJECT) {
		return;
	}

	auto object = static_cast<Object *>(m_active_node);

	object->updateMatrix();

	if (object->collection()) {
		for (auto &prim : object->collection()->primitives()) {
			prim->tagUpdate();
		}
	}

	notify_listeners(OBJECT_MODIFIED);
}

bool Scene::ensureUniqueName(QString &name) const
{
	return ensure_unique_name(name, [&](const QString &str)
	{
		for (const auto &object : m_nodes) {
			if (object->name() == str) {
				return false;
			}
		}

		return true;
	});
}

void Scene::set_active_node(SceneNode *node)
{
	m_active_node = node;
	notify_listeners(OBJECT_SELECTED);
}

void Scene::updateForNewFrame(const EvaluationContext * const context)
{
	m_depsgraph->evaluate_for_time_change(context);
}

void Scene::evalObjectDag(const EvaluationContext * const context, SceneNode *node)
{
	m_depsgraph->evaluate(context, node);
}

void Scene::connect(const EvaluationContext * const context, SceneNode *node_from, SceneNode *node_to)
{
	auto from_type = node_from->type();
	auto to_type = node_to->type();

	bool do_connect = false;

	if (from_type == to_type) {
		if (from_type == SCE_NODE_OBJECT) {
			auto from_ob = static_cast<Object *>(node_from);
			auto to_ob = static_cast<Object *>(node_to);

			from_ob->addChild(to_ob);

			do_connect = true;
		}
		else {
			std::cerr << "Cannot connect two simulations together!\n";
		}
	}
	else {
		if (from_type == SCE_NODE_OBJECT) {
			do_connect = true;
		}
		else {
			std::cerr << "Cannot connect a simulation to an object!\n";
		}
	}

	if (do_connect) {
		node_to->inputs()[0]->link = node_from->outputs()[0];
		node_from->outputs()[0]->links.push_back(node_to->inputs()[0]);

		m_depsgraph->connect(node_from, node_to);
		m_depsgraph->evaluate(context, node_from);
	}
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

const std::vector<SceneNode *> &Scene::nodes() const
{
	return m_nodes;
}
