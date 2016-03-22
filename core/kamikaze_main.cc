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

#include <kamikaze/object.h>
#include <kamikaze/modifiers.h>

#include "dynamiclibrary.h"
#include "filesystem.h"

namespace fs = filesystem;
using PluginVec = std::vector<fs::shared_library>;

typedef void (*register_func_t)(ObjectFactory *);
typedef void (*register_modifier_func_t)(ModifierFactory *);

PluginVec load_plugins(const std::string &path)
{
	PluginVec plugins;
	fs::dir dir(path);

	for (const auto &file : dir) {
		if (!fs::is_library(file)) {
			continue;
		}

		fs::shared_library lib(file);

		if (!lib) {
			std::cerr << lib.error() << '\n';
			continue;
		}

		plugins.push_back(std::move(lib));
	}

	return plugins;
}

Main::Main()
    : m_object_factory(new ObjectFactory)
    , m_modifier_factory(new ModifierFactory)
{}

Main::~Main()
{
	delete m_object_factory;
	delete m_modifier_factory;
}

void Main::loadPlugins()
{
	PluginVec plugins = load_plugins("/opt/kamikaze/");

	for (const auto &plugin : plugins) {
		auto register_figures = plugin.symbol<register_func_t>("new_kamikaze_objects");

		if (register_figures != nullptr) {
			register_figures(m_object_factory);
		}

		auto register_modifiers = plugin.symbol<register_modifier_func_t>("new_kamikaze_modifiers");

		if (register_modifiers != nullptr) {
			register_modifiers(m_modifier_factory);
		}
	}
}

ObjectFactory *Main::objectFactory() const
{
	return m_object_factory;
}

ModifierFactory *Main::modifierFactory() const
{
	return m_modifier_factory;
}
