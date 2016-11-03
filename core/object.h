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

#pragma once

#include <kamikaze/persona.h>

#include "graphs/object_graph.h"

class PrimitiveCollection;
class Node;

enum {
	SNODE_OL_EXPANDED = (1 << 0),  /* Is it expanded in the outliner? */
};

class SceneNode : public Persona {
	PrimitiveCollection *m_collection = nullptr;

	glm::mat4 m_matrix = glm::mat4(0.0f);
	glm::mat4 m_inv_matrix = glm::mat4(0.0f);

	Graph m_graph{};

	SceneNode *m_parent = nullptr;
	std::vector<SceneNode *> m_children;

	std::string m_name = "";
	int m_flags = 0;

	float m_xpos = 0.0f;
	float m_ypos = 0.0f;

public:
	SceneNode();
	~SceneNode();

	PrimitiveCollection *collection() const;
	void collection(PrimitiveCollection *coll);

	/* Return the object's matrix. */
	void matrix(const glm::mat4 &m);
	const glm::mat4 &matrix() const;

	/* Nodes */
	void addNode(Node *node);

	Graph *graph();
	const Graph *graph() const;

	void updateMatrix();

	void add_child(SceneNode *child);
	void remove_child(SceneNode *child);
	const std::vector<SceneNode *> &children() const;

	SceneNode *parent() const;
	void parent(SceneNode *parent);

	std::string get_dag_path() const;

	void name(const std::string &name);
	const std::string &name() const;

	float xpos() const;
	void xpos(float x);

	float ypos() const;
	void ypos(float y);

	inline int flags() const
	{
		return m_flags;
	}

	inline void set_flags(int flag)
	{
		m_flags |= flag;
	}

	inline void unset_flags(int flag)
	{
		m_flags &= ~flag;
	}

	inline bool has_flags(int flag) const
	{
		return (m_flags & flag) != 0;
	}
};
