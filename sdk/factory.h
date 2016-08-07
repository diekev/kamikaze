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
#include <string>
#include <unordered_map>
#include <vector>

template <typename Base, typename Key = std::string>
class Factory final {
public:
	typedef Base *(*factory_func)(void);

	/**
	 * @brief register_type Register a new element in this factory.
	 *
	 * @param key The key associate @ func to.
	 * @param func A function pointer with signature 'Base *(void)'.
	 *
	 * @return The number of entries after registering the new element.
	 */
	size_t register_type(const Key &name, factory_func func)
	{
		const auto iter = m_map.find(name);
		assert(iter == m_map.end());

		m_map[name] = func;
		return num_entries();
	}

	/**
	 * @brief operator() Create a Base based on the given key.
	 *
	 * @param key The key to lookup.
	 * @return A new Base object corresponding to the given key.
	 */
	Base *operator()(const Key &name)
	{
		const auto iter = m_map.find(name);
		assert(iter != m_map.end());

		return iter->second();
	}

	/**
	 * @brief num_entries The number of entries registered in this factory.
	 *
	 * @return The number of entries registered in this factory, 0 if empty.
	 */
	inline size_t num_entries() const
	{
		return m_map.size();
	}

	/**
	 * @brief keys Keys registered in this factory.
	 *
	 * @return An unsorted vector containing the keys registered in this factory.
	 */
	std::vector<std::string> keys() const
	{
		std::vector<std::string> v;
		v.reserve(num_entries());

		for (const auto &entry : m_map) {
			v.push_back(entry.first);
		}

		return v;
	}

	/**
	 * @brief registered Check whether or not a key has been registered in this
	 *                   factory.
	 *
	 * @param key The key to lookup.
	 * @return True if the key is found, false otherwise.
	 */
	bool registered(const std::string &key) const
	{
		return (m_map.find(key) != m_map.end());
	}

private:
	std::unordered_map<Key, factory_func> m_map;
};

#define REGISTER_TYPE(factory, key, base, type) \
	factory->register_type(key, []() -> base* { return new type(); })
