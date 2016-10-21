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

#include <memory>
#include <unordered_map>
#include <vector>

class DepsNode;
class Context;
class EvaluationContext;
class Graph;
class Object;
class SceneNode;
class TaskNotifier;

struct DepsInputSocket;
struct DepsOutputSocket;

/* ************************************************************************** */

struct DepsInputSocket {
	DepsNode *parent = nullptr;
	std::vector<DepsOutputSocket *> links{};

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
	virtual void pre_process() {}
	virtual void process(const Context &context, TaskNotifier *notifier) = 0;

	DepsInputSocket *input();
	const DepsInputSocket *input() const;

	DepsOutputSocket *output();

	bool is_linked() const;

	virtual const char *name() const = 0;
};

/* ************************************************************************** */

class DepsObjectNode : public DepsNode {
	Object *m_object;

public:
	DepsObjectNode() = delete;
	explicit DepsObjectNode(Object *object);

	~DepsObjectNode() = default;

	void pre_process() override;
	void process(const Context &context, TaskNotifier *notifier) override;

	Object *object();
	const Object *object() const;

	const char *name() const override;
};

/* ************************************************************************** */

class ObjectGraphDepsNode : public DepsNode {
	Graph *m_graph;

public:
	ObjectGraphDepsNode() = delete;
	explicit ObjectGraphDepsNode(Graph *graph);

	~ObjectGraphDepsNode() = default;

	void pre_process() override;
	void process(const Context &context, TaskNotifier *notifier) override;

	Graph *graph();
	const Graph *graph() const;

	const char *name() const override;
};

/* ************************************************************************** */

class TimeDepsNode : public DepsNode {
public:
	TimeDepsNode() = default;
	~TimeDepsNode() = default;

	void process(const Context &context, TaskNotifier *notifier) override;

	const char *name() const override;
};

/* ************************************************************************** */

enum {
	DEG_STATE_NONE   = -1,
	DEG_STATE_OBJECT = 0,
	DEG_STATE_TIME   = 1,
};

class Depsgraph {
	std::vector<std::unique_ptr<DepsNode>> m_nodes;
	std::vector<DepsNode *> m_stack;
	std::unordered_map<SceneNode *, DepsNode *> m_scene_node_map;
	std::unordered_map<const Graph *, DepsNode *> m_object_graph_map;

	int m_state = DEG_STATE_NONE;
	bool m_need_update = false;

	DepsNode *m_time_node = nullptr;

	friend class GraphEvalTask;

public:
	Depsgraph();
	~Depsgraph() = default;

	/* Disallow copy. */
	Depsgraph(const Depsgraph &other) = delete;
	Depsgraph &operator=(const Depsgraph &other) = delete;

	void connect(SceneNode *from, SceneNode *to);
	void disconnect(SceneNode *from, SceneNode *to);

	void connect(DepsOutputSocket *from, DepsInputSocket *to);
	void disconnect(DepsOutputSocket *from, DepsInputSocket *to);

	void create_node(SceneNode *scene_node);
	void remove_node(SceneNode *scene_node);

	void connect_to_time(SceneNode *scene_node);

	void evaluate(const Context &context, SceneNode *scene_node);
	void evaluate_for_time_change(const Context &context);

	const std::vector<std::unique_ptr<DepsNode> > &nodes() const;

private:
	void build(DepsNode *root);

	void evaluate_ex(const Context &context, DepsNode *root, TaskNotifier *notifier);
	DepsNode *find_node(SceneNode *scene_node, bool graph);
};
