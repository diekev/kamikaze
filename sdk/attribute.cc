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

#include "attribute.h"

template <typename Container>
void copy(const Container &from, Container &to)
{
	std::copy(from->begin(), from->end(), to->begin());
}

Attribute::Attribute(const std::string &name, AttributeType type, size_t size)
    : m_name(name)
    , m_type(type)
{
	switch (m_type) {
		case ATTR_TYPE_BYTE:
			m_data.char_list = new std::vector<char>(size);
			break;
		case ATTR_TYPE_INT:
			m_data.int_list = new std::vector<int>(size);
			break;
		case ATTR_TYPE_FLOAT:
			m_data.float_list = new std::vector<float>(size);
			break;
		case ATTR_TYPE_STRING:
			m_data.string_list = new std::vector<std::string>(size);
			break;
		case ATTR_TYPE_VEC2:
			m_data.vec2_list = new std::vector<glm::vec2>(size);
			break;
		case ATTR_TYPE_VEC3:
			m_data.vec3_list = new std::vector<glm::vec3>(size);
			break;
		case ATTR_TYPE_VEC4:
			m_data.vec4_list = new std::vector<glm::vec4>(size);
			break;
		case ATTR_TYPE_MAT3:
			m_data.mat3_list = new std::vector<glm::mat3>(size);
			break;
		case ATTR_TYPE_MAT4:
			m_data.mat4_list = new std::vector<glm::mat4>(size);
			break;
		default:
			break;
	}
}

Attribute::Attribute(const Attribute &rhs)
    : Attribute(rhs.name(), rhs.type(), rhs.size())
{
	switch (m_type) {
		case ATTR_TYPE_BYTE:
			copy(rhs.m_data.char_list, m_data.char_list);
			break;
		case ATTR_TYPE_INT:
			copy(rhs.m_data.int_list, m_data.int_list);
			break;
		case ATTR_TYPE_FLOAT:
			copy(rhs.m_data.float_list, m_data.float_list);
			break;
		case ATTR_TYPE_STRING:
			copy(rhs.m_data.string_list, m_data.string_list);
			break;
		case ATTR_TYPE_VEC2:
			copy(rhs.m_data.vec2_list, m_data.vec2_list);
			break;
		case ATTR_TYPE_VEC3:
			copy(rhs.m_data.vec3_list, m_data.vec3_list);
			break;
		case ATTR_TYPE_VEC4:
			copy(rhs.m_data.vec4_list, m_data.vec4_list);
			break;
		case ATTR_TYPE_MAT3:
			copy(rhs.m_data.mat3_list, m_data.mat3_list);
			break;
		case ATTR_TYPE_MAT4:
			copy(rhs.m_data.mat4_list, m_data.mat4_list);
			break;
		default:
			break;
	}
}

Attribute::~Attribute()
{
	switch (m_type) {
		case ATTR_TYPE_BYTE:
			delete m_data.char_list;
			break;
		case ATTR_TYPE_INT:
			delete m_data.int_list;
			break;
		case ATTR_TYPE_FLOAT:
			delete m_data.float_list;
			break;
		case ATTR_TYPE_STRING:
			delete m_data.string_list;
			break;
		case ATTR_TYPE_VEC2:
			delete m_data.vec2_list;
			break;
		case ATTR_TYPE_VEC3:
			delete m_data.vec3_list;
			break;
		case ATTR_TYPE_VEC4:
			delete m_data.vec4_list;
			break;
		case ATTR_TYPE_MAT3:
			delete m_data.mat3_list;
			break;
		case ATTR_TYPE_MAT4:
			delete m_data.mat4_list;
			break;
		default:
			break;
	}
}

AttributeType Attribute::type() const
{
	return m_type;
}

std::string Attribute::name() const
{
	return m_name;
}

void Attribute::reserve(size_t n)
{
	switch (m_type) {
		case ATTR_TYPE_BYTE:
			m_data.char_list->reserve(n);
			break;
		case ATTR_TYPE_INT:
			m_data.int_list->reserve(n);
			break;
		case ATTR_TYPE_FLOAT:
			m_data.float_list->reserve(n);
			break;
		case ATTR_TYPE_STRING:
			m_data.string_list->reserve(n);
			break;
		case ATTR_TYPE_VEC2:
			m_data.vec2_list->reserve(n);
			break;
		case ATTR_TYPE_VEC3:
			m_data.vec3_list->reserve(n);
			break;
		case ATTR_TYPE_VEC4:
			m_data.vec4_list->reserve(n);
			break;
		case ATTR_TYPE_MAT3:
			m_data.mat3_list->reserve(n);
			break;
		case ATTR_TYPE_MAT4:
			m_data.mat4_list->reserve(n);
			break;
		default:
			break;
	}
}

void Attribute::resize(size_t n)
{
	switch (m_type) {
		case ATTR_TYPE_BYTE:
			m_data.char_list->resize(n);
			break;
		case ATTR_TYPE_INT:
			m_data.int_list->resize(n);
			break;
		case ATTR_TYPE_FLOAT:
			m_data.float_list->resize(n);
			break;
		case ATTR_TYPE_STRING:
			m_data.string_list->resize(n);
			break;
		case ATTR_TYPE_VEC2:
			m_data.vec2_list->resize(n);
			break;
		case ATTR_TYPE_VEC3:
			m_data.vec3_list->resize(n);
			break;
		case ATTR_TYPE_VEC4:
			m_data.vec4_list->resize(n);
			break;
		case ATTR_TYPE_MAT3:
			m_data.mat3_list->resize(n);
			break;
		case ATTR_TYPE_MAT4:
			m_data.mat4_list->resize(n);
			break;
		default:
			break;
	}
}

size_t Attribute::size() const
{
	switch (m_type) {
		case ATTR_TYPE_BYTE:
			return m_data.char_list->size();
		case ATTR_TYPE_INT:
			return m_data.int_list->size();
		case ATTR_TYPE_FLOAT:
			return m_data.float_list->size();
		case ATTR_TYPE_STRING:
			return m_data.string_list->size();
		case ATTR_TYPE_VEC2:
			return m_data.vec2_list->size();
		case ATTR_TYPE_VEC3:
			return m_data.vec3_list->size();
		case ATTR_TYPE_VEC4:
			return m_data.vec4_list->size();
		case ATTR_TYPE_MAT3:
			return m_data.mat3_list->size();
		case ATTR_TYPE_MAT4:
			return m_data.mat4_list->size();
		default:
			return 0;
	}
}

void Attribute::clear()
{
	switch (m_type) {
		case ATTR_TYPE_BYTE:
			m_data.char_list->clear();
			break;
		case ATTR_TYPE_INT:
			m_data.int_list->clear();
			break;
		case ATTR_TYPE_FLOAT:
			m_data.float_list->clear();
			break;
		case ATTR_TYPE_STRING:
			m_data.string_list->clear();
			break;
		case ATTR_TYPE_VEC2:
			m_data.vec2_list->clear();
			break;
		case ATTR_TYPE_VEC3:
			m_data.vec3_list->clear();
			break;
		case ATTR_TYPE_VEC4:
			m_data.vec4_list->clear();
			break;
		case ATTR_TYPE_MAT3:
			m_data.mat3_list->clear();
			break;
		case ATTR_TYPE_MAT4:
			m_data.mat4_list->clear();
			break;
		default:
			break;
	}
}

const void *Attribute::data() const
{
	switch (m_type) {
		case ATTR_TYPE_BYTE:
			return &(*(m_data.char_list))[0];
		case ATTR_TYPE_INT:
			return &(*(m_data.int_list))[0];
		case ATTR_TYPE_FLOAT:
			return &(*(m_data.float_list))[0];
		case ATTR_TYPE_STRING:
			return &(*(m_data.string_list))[0];
		case ATTR_TYPE_VEC2:
			return &(*(m_data.vec2_list))[0][0];
		case ATTR_TYPE_VEC3:
			return &(*(m_data.vec3_list))[0][0];
		case ATTR_TYPE_VEC4:
			return &(*(m_data.vec4_list))[0][0];
		case ATTR_TYPE_MAT3:
			return &(*(m_data.mat3_list))[0][0];
		case ATTR_TYPE_MAT4:
			return &(*(m_data.mat4_list))[0][0];
		default:
			return nullptr;
	}
}

size_t Attribute::byte_size() const
{
	switch (m_type) {
		case ATTR_TYPE_BYTE:
			return m_data.char_list->size() * sizeof(char);
		case ATTR_TYPE_INT:
			return m_data.int_list->size() * sizeof(int);
		case ATTR_TYPE_FLOAT:
			return m_data.float_list->size() * sizeof(float);
		case ATTR_TYPE_STRING:
			return m_data.string_list->size() * sizeof(std::string);
		case ATTR_TYPE_VEC2:
			return m_data.vec2_list->size() * sizeof(glm::vec2);
		case ATTR_TYPE_VEC3:
			return m_data.vec3_list->size() * sizeof(glm::vec3);
		case ATTR_TYPE_VEC4:
			return m_data.vec4_list->size() * sizeof(glm::vec4);
		case ATTR_TYPE_MAT3:
			return m_data.mat3_list->size() * sizeof(glm::mat3);
		case ATTR_TYPE_MAT4:
			return m_data.mat4_list->size() * sizeof(glm::mat4);
		default:
			return 0;
	}
}

void Attribute::byte(size_t n, char b)
{
	(*(m_data.char_list))[n] = b;
}

char Attribute::byte(size_t n) const
{
	return (*(m_data.char_list))[n];
}

void Attribute::integer(size_t n, int i)
{
	(*(m_data.int_list))[n] = i;
}

int Attribute::integer(size_t n) const
{
	return (*(m_data.int_list))[n];
}

void Attribute::float_(size_t n, float f)
{
	(*(m_data.float_list))[n] = f;
}

int Attribute::float_(size_t n) const
{
	return (*(m_data.float_list))[n];
}

void Attribute::vec2(size_t n, const glm::vec2 &v)
{
	(*(m_data.vec2_list))[n] = v;
}

const glm::vec2 &Attribute::vec2(size_t n) const
{
	return (*(m_data.vec2_list))[n];
}

void Attribute::vec3(size_t n, const glm::vec3 &v)
{
	(*(m_data.vec3_list))[n] = v;
}

const glm::vec3 &Attribute::vec3(size_t n) const
{
	return (*(m_data.vec3_list))[n];
}

void Attribute::vec4(size_t n, const glm::vec4 &v)
{
	(*(m_data.vec4_list))[n] = v;
}

const glm::vec4 &Attribute::vec4(size_t n) const
{
	return (*(m_data.vec4_list))[n];
}

void Attribute::mat3(size_t n, const glm::mat3 &m)
{
	(*(m_data.mat3_list))[n] = m;
}

const glm::mat3 &Attribute::mat3(size_t n) const
{
	return (*(m_data.mat3_list))[n];
}

void Attribute::mat4(size_t n, const glm::mat4 &m)
{
	(*(m_data.mat4_list))[n] = m;
}

const glm::mat4 &Attribute::mat4(size_t n) const
{
	return (*(m_data.mat4_list))[n];
}

void Attribute::stdstring(size_t n, const std::string &str)
{
	(*(m_data.string_list))[n] = str;
}

const std::string &Attribute::stdstring(size_t n) const
{
	return (*(m_data.string_list))[n];
}
