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

#include <glm/glm.hpp>
#include <string>
#include <vector>

enum AttributeType {
	ATTR_TYPE_INVALID = -1,
	ATTR_TYPE_BYTE = 0,
	ATTR_TYPE_INT,
	ATTR_TYPE_FLOAT,
	ATTR_TYPE_STRING,
	ATTR_TYPE_VEC2,
	ATTR_TYPE_VEC3,
	ATTR_TYPE_VEC4,
	ATTR_TYPE_MAT3,
	ATTR_TYPE_MAT4,
};

class Attribute {
	union {
		std::vector<char> *char_list;
		std::vector<int> *int_list;
		std::vector<float> *float_list;
		std::vector<std::string> *string_list;
		std::vector<glm::vec2> *vec2_list;
		std::vector<glm::vec3> *vec3_list;
		std::vector<glm::vec4> *vec4_list;
		std::vector<glm::mat3> *mat3_list;
		std::vector<glm::mat4> *mat4_list;
	} m_data;

	std::string m_name;
	AttributeType m_type;

public:
	Attribute(const Attribute &rhs);
	Attribute(const std::string &name, AttributeType type, size_t size = 0);
	~Attribute();

	AttributeType type() const;
	std::string name() const;

	void reserve(size_t n);
	void resize(size_t n);

	void clear();

	const void *data() const;

	size_t byte_size() const;
	size_t size() const;

	/**
	 * @brief byte Set a byte in the attribute list.
	 * @param n The position to write the byte in the list.
	 * @param b The byte to write in the list.
	 */
	void byte(size_t n, char b);

	/**
	 * @brief byte Get a byte from the attribute list.
	 * @param n The position of the byte in the list.
	 * @return The byte at position n.
	 */
	char byte(size_t n) const;

	void integer(size_t n, int i);
	int integer(size_t n) const;

	void float_(size_t n, float f);
	int float_(size_t n) const;

	void vec2(size_t n, const glm::vec2 &v);
	const glm::vec2 &vec2(size_t n) const;

	void vec3(size_t n, const glm::vec3 &v);
	const glm::vec3 &vec3(size_t n) const;

	void vec4(size_t n, const glm::vec4 &v);
	const glm::vec4 &vec4(size_t n) const;

	void mat3(size_t n, const glm::mat3 &m);
	const glm::mat3 &mat3(size_t n) const;

	void mat4(size_t n, const glm::mat4 &m);
	const glm::mat4 &mat4(size_t n) const;

	void stdstring(size_t n, const std::string &str);
	const std::string &stdstring(size_t n) const;
};
