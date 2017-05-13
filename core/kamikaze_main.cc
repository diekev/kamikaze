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

#include <girafeenfeu/systeme_fichier/utilitaires.h>

#include <kamikaze/nodes.h>
#include <kamikaze/mesh.h>
#include <kamikaze/primitive.h>
#include <kamikaze/prim_points.h>
#include <kamikaze/segmentprim.h>

#include <dlfcn.h>

#include "graphs/object_nodes.h"
#include "scene.h"

namespace fs = std::experimental::filesystem;
namespace sf = systeme_fichier;

static std::vector<sf::shared_library> load_plugins(const fs::path &path)
{
	std::vector<sf::shared_library> plugins;

	std::error_code ec;
	for (const auto &entry : fs::directory_iterator(path)) {
		if (!sf::est_bibilotheque(entry)) {
			continue;
		}

		sf::shared_library lib(entry, ec);

		if (!lib) {
			std::cerr << "Invalid library object: " << entry.path() << '\n';
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
		auto register_figures = sf::dso_function<void(PrimitiveFactory *)>(symbol);

		if (register_figures) {
			register_figures(this->primitive_factory());
		}

		symbol = plugin("new_kamikaze_node", ec);
		auto register_node = sf::dso_function<void(NodeFactory *)>(symbol);

		if (register_node) {
			register_node(this->node_factory());
		}
	}
}

void Main::initialize()
{
	register_builtin_nodes(this->node_factory());

	/* primitive types */

	{
		auto factory = this->primitive_factory();

		Mesh::id = REGISTER_PRIMITIVE("Mesh", Mesh);
		PrimPoints::id = REGISTER_PRIMITIVE("PrimPoints", PrimPoints);
		SegmentPrim::id = REGISTER_PRIMITIVE("SegmentPrim", SegmentPrim);
	}
}

PrimitiveFactory *Main::primitive_factory() const
{
	return m_primitive_factory.get();
}

NodeFactory *Main::node_factory() const
{
	return m_node_factory.get();
}

Scene *Main::scene() const
{
	return m_scene.get();
}
