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
 * The Original Code is Copyright (C) 2016 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include "undo.h"

#include <kamikaze/context.h>
#include <kamikaze/nodes.h>
#include <kamikaze/primitive.h>

#include "graphs/object_graph.h"
#include "graphs/object_nodes.h"

#include "object.h"
#include "scene.h"

#include "util/util_input.h"

class AddObjectCmd : public Command {
	Object *m_object = nullptr;
	Scene *m_scene = nullptr;

public:
	AddObjectCmd() = default;
	~AddObjectCmd() = default;

	void execute(const Context &context) override
	{
		m_scene = context.scene;

		m_object = new Object;
		m_object->name(m_name.c_str());

		assert(m_scene != nullptr);
		m_scene->addObject(m_object);
	}
};

class AddNodeCmd : public Command {
	Object *m_object = nullptr;
	Scene *m_scene = nullptr;

public:
	AddNodeCmd() = default;
	~AddNodeCmd() = default;

	void execute(const Context &context) override
	{
		m_scene = context.scene;
		auto scene_node = m_scene->active_node();

		if (scene_node == nullptr) {
			return;
		}

		m_object = static_cast<Object *>(scene_node);

		assert(m_object != nullptr);

		auto node = (*context.node_factory)(m_name);
		m_object->addNode(node);

		m_scene->notify_listeners(event_type::node | event_type::added);
	}
};

class AddPresetObjectCmd : public Command {
	Object *m_object = nullptr;
	Scene *m_scene = nullptr;

public:
	AddPresetObjectCmd() = default;
	~AddPresetObjectCmd() = default;

	void execute(const Context &context) override
	{
		m_scene = context.scene;

		auto node = (*context.node_factory)(m_name);

		if (context.eval_ctx->edit_mode) {
			auto scene_node = m_scene->active_node();

			/* Sanity check. */
			if (scene_node == nullptr) {
				return;
			}

			m_object = static_cast<Object *>(scene_node);
		}
		else {
			m_object = new Object;
			m_object->name(m_name.c_str());
		}

		assert(m_object != nullptr);

		m_object->addNode(node);

		if (!context.eval_ctx->edit_mode) {
			m_scene->addObject(m_object);
		}
		else {
			m_scene->notify_listeners(event_type::node | event_type::added);
		}
	}
};

class DeleteObjectCommand : public Command {
public:
	DeleteObjectCommand() = default;
	~DeleteObjectCommand() = default;

	void execute(const Context &context) override
	{
		auto scene = context.scene;
		scene->removeObject(scene->active_node());
	}
};

void register_object_key_mappings(std::vector<KeyData> &keys)
{
	keys.emplace_back(0, 0, "add node");
	keys.emplace_back(0, 0, "add object");
	keys.emplace_back(0, 0, "add preset");
	keys.emplace_back(MOD_KEY_NONE, 0x01000007, "DeleteObjectCommand");
}

void register_object_commands(CommandFactory *factory)
{
	REGISTER_COMMAND(factory, "add node", AddNodeCmd);
	REGISTER_COMMAND(factory, "add object", AddObjectCmd);
	REGISTER_COMMAND(factory, "add preset", AddPresetObjectCmd);
	REGISTER_COMMAND(factory, "DeleteObjectCommand", DeleteObjectCommand);
}
