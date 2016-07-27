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

#include "kamikaze_main.h"

#include <kamikaze/nodes.h>
#include <kamikaze/mesh.h>
#include <kamikaze/primitive.h>
#include <kamikaze/prim_points.h>

#include <utils/filesystem.h>

#include <dlfcn.h>

#include "graphs/object_nodes.h"
#include "scene.h"

namespace fs = filesystem;

static std::vector<fs::shared_library> load_plugins(const fs::path &path)
{
	std::vector<fs::shared_library> plugins;

	std::error_code ec;
	for (const auto &entry : fs::directory_iterator(path)) {
		if (!fs::is_library(entry)) {
			continue;
		}

		fs::shared_library lib(entry, ec);

		if (!lib) {
			std::cerr << "Invalid library object: " << entry.path_() << '\n';
			std::cerr << dlerror() << '\n';
			continue;
		}

		plugins.push_back(std::move(lib));
	}

	return plugins;
}

Main::Main()
    : m_primitive_factory(new PrimitiveFactory)
    , m_node_factory(new NodeFactory)
    , m_scene(new Scene)
{}

Main::~Main()
{
	delete m_primitive_factory;
	delete m_node_factory;
	delete m_scene;
}

void Main::loadPlugins()
{
	m_plugins = load_plugins("plugins");

	std::error_code ec;
	for (auto &plugin : m_plugins) {
		if (!plugin) {
			std::cerr << "Invalid library object\n";
			continue;
		}

		auto symbol = plugin("new_kamikaze_prims", ec);
		auto register_figures = fs::dso_function<void(PrimitiveFactory *)>(symbol);

		if (register_figures) {
			register_figures(m_primitive_factory);
		}

		symbol = plugin("new_kamikaze_node", ec);
		auto register_node = fs::dso_function<void(NodeFactory *)>(symbol);

		if (register_node) {
			register_node(m_node_factory);
		}
	}
}

void Main::initTypes()
{
	register_builtin_nodes(m_node_factory);

	/* primitive types */

	{
		auto factory = m_primitive_factory;

		Mesh::id = REGISTER_PRIMITIVE("Mesh", Mesh);
		PrimPoints::id = REGISTER_PRIMITIVE("PrimPoints", PrimPoints);
	}
}

PrimitiveFactory *Main::primitiveFactory() const
{
	return m_primitive_factory;
}

NodeFactory *Main::nodeFactory() const
{
	return m_node_factory;
}

Scene *Main::scene() const
{
	return m_scene;
}
