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

#include <QString>

#include <kamikaze/util_render.h>

#include "context.h"
#include "object.h"
#include "graphs/depsgraph.h"

class Depsgraph;
class EvaluationContext;
class Node;
class Simulation;

using SceneNodePtr = std::unique_ptr<SceneNode>;

enum {
	SCENE_OL_EXPANDED = (1 << 0),  /* Is it expanded in the outliner? */
};

class Scene : public Listened {
	std::vector<SceneNodePtr> m_nodes = {};
	SceneNode *m_active_node = nullptr;
	int m_mode = 0;

	/* The root of the hierarchy, "/", it only has children node. */
	SceneNode *m_root;

	/* The node that is currently viewed/edited. */
	SceneNode *m_current_node;

	Depsgraph m_depsgraph{};

	int m_start_frame = 0;
	int m_end_frame = 250;
	int m_cur_frame = 0;
	float m_fps = 24.0f;

	int m_flags = 0;

public:
	Scene();
	~Scene() = default;

	SceneNode *active_node();
	void set_active_node(SceneNode *node);

	void add_node(SceneNode *node);
	void remove_node(SceneNode *node);

	void intersect(const Ray &ray);

	void selectObject(const glm::vec3 &pos);

	Depsgraph *depsgraph();

	/* Time/Frame */

	int startFrame() const;
	void startFrame(int value);

	int endFrame() const;
	void endFrame(int value);

	int currentFrame() const;
	void currentFrame(int value);

	float framesPerSecond() const;
	void framesPerSecond(float value);

	void updateForNewFrame(const Context &context);

	const std::vector<SceneNodePtr> &nodes() const;

	void tagObjectUpdate();

	void evalObjectDag(const Context &context, SceneNode *node);

	int flags() const;
	void set_flags(int flag);
	void unset_flags(int flag);
	bool has_flags(int flag);

	SceneNode *current_node();

private:
	bool ensureUniqueName(std::string &name) const;
};
