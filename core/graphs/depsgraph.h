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

#include <unordered_map>
#include <vector>

class DepsNode;
class EvaluationContext;
class Object;

struct DepsInputSocket;
struct DepsOutputSocket;

/* ************************************************************************** */

struct DepsInputSocket {
	DepsNode *parent = nullptr;
	DepsOutputSocket *link = nullptr;

	DepsInputSocket() = default;
};

struct DepsOutputSocket {
	DepsNode *parent = nullptr;
	std::vector<DepsInputSocket *> links{};

	DepsOutputSocket() = default;
};

/* ************************************************************************** */

class DepsNode {
	DepsInputSocket m_input;
	DepsOutputSocket m_output;

public:
	DepsNode();

	virtual ~DepsNode() = default;
	virtual void process() = 0;

	DepsInputSocket *input();
	const DepsInputSocket *input() const;

	DepsOutputSocket *output();

	bool is_linked() const;
};

/* ************************************************************************** */

class DepsObjectNode : public DepsNode {
	Object *m_object;

public:
	DepsObjectNode() = delete;
	DepsObjectNode(Object *object);

	~DepsObjectNode() = default;

	void process() override;

	Object *object();
	const Object *object() const;
};

/* ************************************************************************** */

class Depsgraph {
	std::vector<DepsNode *> m_nodes;
	std::vector<DepsNode *> m_stack;
	std::unordered_map<Object *, DepsNode *> m_object_map;

	bool m_need_update = false;

public:
	~Depsgraph();

	void connect(DepsOutputSocket *from, DepsInputSocket *to);
	void disconnect(DepsOutputSocket *from, DepsInputSocket *to);

	void create_node(Object *object);
	void remove_node(Object *object);

	void evaluate(const EvaluationContext * const context);

	const std::vector<DepsNode *> &nodes() const;

private:
	void topology_sort();
};
