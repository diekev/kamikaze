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
#include <vector>

class PointList {
	std::vector<glm::vec3> m_points{};

public:
	PointList() = default;

	void push_back(const glm::vec3 &point);
	void push_back(glm::vec3 &&point);

	void reserve(size_t n);

	void resize(size_t n);

	size_t size() const;

	size_t byte_size() const;

	const void *data() const;

	glm::vec3 &operator[](size_t i);
	const glm::vec3 &operator[](size_t i) const;
};

/**
 * @brief INVALID_INDEX Marker used to indicate that a polygon is a triangle,
 *                      e.g. poly[3] = INVALID_INDEX.
 */
static constexpr auto INVALID_INDEX = std::numeric_limits<unsigned int>::max();

class PolygonList {
	std::vector<glm::uvec4> m_polys{};

public:
	PolygonList() = default;

	void push_back(const glm::uvec4 &poly);

	void push_back(glm::uvec4 &&poly);

	void reserve(size_t n);

	void resize(size_t n);

	size_t size() const;

	size_t byte_size() const;

	const void *data() const;

	glm::uvec4 &operator[](size_t i);

	const glm::uvec4 &operator[](size_t i) const;
};
