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

#include <cassert>
#include <unordered_map>
#include <vector>

class ParamCallback;
class Object;

class Modifier {
	std::string m_name;

public:
	virtual ~Modifier() = default;

	void setName(const std::string &name);
	std::string name() const;

	virtual void evaluate(Object *ob) = 0;
	virtual void setUIParams(ParamCallback *cb) = 0;
};

class ModifierFactory final {
public:
	typedef Modifier *(*factory_func)(void);

	void registerType(const std::string &name, factory_func func);

	Modifier *operator()(const std::string &name);

	size_t numEntries() const;

	std::vector<std::string> keys() const;

private:
	std::unordered_map<std::string, factory_func> m_map;
};
