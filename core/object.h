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

#include <glm/glm.hpp>
#include <memory>

#include <QString>

#include <kamikaze/primitive.h>
#include <kamikaze/persona.h>

struct SceneNode;
struct SceneInputSocket;

struct SceneOutputSocket {
	SceneNode *parent = nullptr;
	std::vector<SceneInputSocket *> links{};
	std::string name = "";

	explicit SceneOutputSocket(const std::string &sname)
	    : parent(nullptr)
	    , name(sname)
	{}
};

struct SceneInputSocket {
	SceneNode *parent = nullptr;
	SceneOutputSocket *link = nullptr;
	std::string name = "";

	explicit SceneInputSocket(const std::string &sname)
	    : parent(nullptr)
	    , link(nullptr)
	    , name(sname)
	{}
};

enum {
	SNODE_OL_EXPANDED = (1 << 0),  /* Is it expanded in the outliner? */
};

class SceneNode : public Persona {
protected:
	std::vector<SceneInputSocket *> m_inputs = {};
	std::vector<SceneOutputSocket *> m_outputs = {};

	std::string m_name = "";
	int m_flags = 0;

public:
	virtual ~SceneNode()
	{
		for (auto &input : m_inputs) {
			delete input;
		}

		for (auto &output : m_outputs) {
			delete output;
		}
	}

	void add_input(const std::string &name)
	{
		auto socket = new SceneInputSocket(name);
		socket->parent = this;

		this->m_inputs.push_back(socket);
	}

	void add_output(const std::string &name)
	{
		auto socket = new SceneOutputSocket(name);
		socket->parent = this;

		this->m_outputs.push_back(socket);
	}

	const std::vector<SceneInputSocket *> &inputs()
	{
		return m_inputs;
	}

	const std::vector<SceneOutputSocket *> &outputs()
	{
		return m_outputs;
	}

	void name(const std::string &name)
	{
		m_name = name;
	}

	const std::string &name() const
	{
		return m_name;
	}

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

class EvaluationContext;
class Graph;
class Node;
class ParamCallback;
class Primitive;
class PrimitiveCollection;

class Object : public SceneNode {
	PrimitiveCollection *m_collection = nullptr;

	glm::mat4 m_matrix = glm::mat4(0.0f);
	glm::mat4 m_inv_matrix = glm::mat4(0.0f);

	Graph *m_graph;

	Object *m_parent = nullptr;
	std::vector<Object *> m_children;

public:
	Object();
	~Object();

	PrimitiveCollection *collection() const;
	void collection(PrimitiveCollection *coll);

	/* Return the object's matrix. */
	void matrix(const glm::mat4 &m);
	const glm::mat4 &matrix() const;

	/* Nodes */
	void addNode(Node *node);

	Graph *graph() const;

	void updateMatrix();

	void addChild(Object *child);
	void removeChild(Object *child);
	const std::vector<Object *> &children() const;

	Object *parent() const;
	void parent(Object *parent);
};
