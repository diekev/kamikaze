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

#include <QString>
#include <vector>

#include <glm/glm.hpp>
#include <memory>

class Graph;
class Node;
class ParamCallback;
class Primitive;
class MainWindow;

#include "ui/mainwindow.h"

/**
 * This class is used to gather and release the primitives created inside of an
 * object's node graph.
 */
class PrimitiveCache {
	std::vector<Primitive *> m_primitives;

public:
	void add(Primitive *prim);
	void clear();
};

class Object {
	Primitive *m_primitive = nullptr;
	PrimitiveCache m_cache;

	glm::vec3 m_scale = glm::vec3(1.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f);
	glm::vec3 m_pos = glm::vec3(0.0f);

	glm::mat4 m_matrix = glm::mat4(0.0f);
	glm::mat4 m_inv_matrix = glm::mat4(0.0f);

	Graph *m_graph;

	std::string m_name;

public:
	Object();
	~Object();

	Primitive *primitive() const;
	void primitive(Primitive *prim);

	/* Return the object's matrix. */
	void matrix(const glm::mat4 &m);
	const glm::mat4 &matrix() const;

	/* Nodes */
	void addNode(Node *node);

	Graph *graph() const;

	void name(const QString &name);
	const QString name() const;

	void setUIParams(ParamCallback *cb);

	void updateMatrix();
	void clearCache();
};

void eval_graph(MainWindow *window, Object *ob, bool force);
