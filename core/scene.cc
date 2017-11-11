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

#include <kamikaze/primitive.h>
#include <kamikaze/outils/chaîne_caractère.h>

void Scene::removeObject(SceneNode *node)
{
	auto iter = std::find_if(m_nodes.begin(), m_nodes.end(),
	                         [node](const SceneNodePtr &node_ptr)
	{
		return node_ptr.get() == node;
	});

	assert(iter != m_nodes.end());

	notify_listeners(event_type::object | event_type::removed);

	m_depsgraph.remove_node(node);

	if (node == m_active_node) {
		m_active_node = nullptr;
	}

	m_nodes.erase(iter);
}

void Scene::addObject(SceneNode *node)
{
	auto name = node->name();

	if (ensureUniqueName(name)) {
		node->name(name);
	}

	m_nodes.push_back(SceneNodePtr(node));
	m_active_node = node;

	m_depsgraph.create_node(node);

	notify_listeners(event_type::object | event_type::added);
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
		auto object = static_cast<Object *>(node.get());

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

	if (selected_object != -1 && m_active_node != (m_nodes[selected_object]).get()) {
		m_active_node = (m_nodes[selected_object]).get();
		notify_listeners(event_type::object | event_type::selected);
	}
}

Depsgraph *Scene::depsgraph()
{
	return &m_depsgraph;
}

SceneNode *Scene::active_node()
{
	if (!m_nodes.empty()) {
		return m_active_node;
	}

	return nullptr;
}

void Scene::tagObjectUpdate()
{
	if (!m_active_node) {
		return;
	}

	auto object = static_cast<Object *>(m_active_node);

	object->updateMatrix();

	if (object->collection()) {
		for (auto &prim : object->collection()->primitives()) {
			prim->tagUpdate();
		}
	}

	notify_listeners(event_type::object | event_type::modified);
}

bool Scene::ensureUniqueName(std::string &name) const
{
	return ensure_unique_name(name, [&](const std::string &str)
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
	notify_listeners(event_type::object | event_type::selected);
}

void Scene::updateForNewFrame(const Context &context)
{
	m_depsgraph.evaluate_for_time_change(context);
}

void Scene::evalObjectDag(const Context &context, SceneNode *node)
{
	m_depsgraph.evaluate(context, node);
}

void Scene::connect(const Context &context, SceneNode *node_from, SceneNode *node_to)
{
	auto from_ob = static_cast<Object *>(node_from);
	auto to_ob = static_cast<Object *>(node_to);

	from_ob->addChild(to_ob);

	node_to->inputs()[0]->link = node_from->outputs()[0].get();
	node_from->outputs()[0]->links.push_back(node_to->inputs()[0].get());

	m_depsgraph.connect(node_from, node_to);
	m_depsgraph.evaluate(context, node_from);
}

void Scene::disconnect(const Context &context, SceneNode *node_from, SceneNode *node_to)
{
	auto from_ob = static_cast<Object *>(node_from);
	auto to_ob = static_cast<Object *>(node_to);

	from_ob->removeChild(to_ob);

	node_to->inputs()[0]->link = nullptr;

	auto from = node_from->outputs()[0].get();
	auto iter = std::find(from->links.begin(), from->links.end(), node_to->inputs()[0].get());

	if (iter == from->links.end()) {
		std::cerr << "Scene::disconnect, cannot find output!\n";
		return;
	}

	from->links.erase(iter);

	m_depsgraph.disconnect(node_from, node_to);
	m_depsgraph.evaluate(context, node_to);
}

int Scene::flags() const
{
	return m_flags;
}

void Scene::set_flags(int flag)
{
	m_flags |= flag;
}

void Scene::unset_flags(int flag)
{
	m_flags &= ~flag;
}

bool Scene::has_flags(int flag)
{
	return (m_flags & flag) != 0;
}

void Scene::supprime_tout()
{
	for (const auto &node : m_nodes) {
		m_depsgraph.remove_node(node.get());
	}

	m_nodes.clear();
}

int Scene::startFrame() const
{
	return m_start_frame;
}

void Scene::startFrame(int value)
{
	m_start_frame = value;
	notify_listeners(event_type::time | event_type::modified);
}

int Scene::endFrame() const
{
	return m_end_frame;
}

void Scene::endFrame(int value)
{
	m_end_frame = value;
	notify_listeners(event_type::time | event_type::modified);
}

int Scene::currentFrame() const
{
	return m_cur_frame;
}

void Scene::currentFrame(int value)
{
	m_cur_frame = value;
	notify_listeners(event_type::time | event_type::modified);
}

float Scene::framesPerSecond() const
{
	return m_fps;
}

void Scene::framesPerSecond(float value)
{
	m_fps = value;
	notify_listeners(event_type::time | event_type::modified);
}

const std::vector<SceneNodePtr> &Scene::nodes() const
{
	return m_nodes;
}
