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

#include "modifiers.h"

void Modifier::setName(const std::string &name)
{
	m_name = name;
}

std::string Modifier::name() const
{
	return m_name;
}

void ModifierFactory::registerType(const std::string &name, ModifierFactory::factory_func func)
{
	const auto iter = m_map.find(name);
	assert(iter == m_map.end());

	m_map[name] = func;
}

Modifier *ModifierFactory::operator()(const std::string &name)
{
	const auto iter = m_map.find(name);
	assert(iter != m_map.end());

	return iter->second();
}

size_t ModifierFactory::numEntries() const
{
	return m_map.size();
}

std::vector<std::string> ModifierFactory::keys() const
{
	std::vector<std::string> v;

	for (const auto &entry : m_map) {
		v.push_back(entry.first);
	}

	return v;
}
