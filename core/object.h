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

class EvaluationContext;
class Graph;
class Node;
class ParamCallback;
class Primitive;
class PrimitiveCollection;

class Object : public Persona {
	PrimitiveCollection *m_collection = nullptr;
	PrimitiveCache m_cache;

	glm::mat4 m_matrix = glm::mat4(0.0f);
	glm::mat4 m_inv_matrix = glm::mat4(0.0f);

	Graph *m_graph;

	std::string m_name;

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

	void name(const QString &name);
	const QString name() const;

	void updateMatrix();
	void clearCache();

	void addChild(Object *child);
	const std::vector<Object *> &children() const;

	Object *parent() const;
	void parent(Object *parent);
};
