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

#include <kamikaze/nodes.h>
#include <kamikaze/primitive.h>

#include <utils/filesystem.h>

#include "scene.h"
#include "simulation.h"

class Main final {
	std::vector<filesystem::shared_library> m_plugins;

	std::unique_ptr<PrimitiveFactory> m_primitive_factory;
	std::unique_ptr<NodeFactory> m_node_factory;
	std::unique_ptr<SolverFactory> m_solver_factory;
	std::unique_ptr<Scene> m_scene;

public:
	Main();

	/* Disallow copy. */
	Main(const Main &other) = delete;
	Main &operator=(const Main &other) = delete;

	void initialize();
	void loadPlugins();

	/* Factories. */
	PrimitiveFactory *primitive_factory() const;
	NodeFactory *node_factory() const;
	SolverFactory *solver_factory() const;
	Scene *scene() const;
};
