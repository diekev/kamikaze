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

#include "geomlists.h"

void PointList::push_back(const glm::vec3 &point)
{
	m_points.push_back(point);
}

void PointList::push_back(glm::vec3 &&point)
{
	m_points.emplace_back(std::move(point));
}

void PointList::reserve(size_t n)
{
	m_points.reserve(n);
}

void PointList::resize(size_t n)
{
	m_points.resize(n);
}

size_t PointList::size() const
{
	return m_points.size();
}

size_t PointList::byte_size() const
{
	return m_points.size() * sizeof(glm::vec3);
}

const void *PointList::data() const
{
	if (m_points.empty()) {
		return nullptr;
	}

	return (&m_points[0][0]);
}

glm::vec3 &PointList::operator[](size_t i)
{
	return m_points[i];
}

const glm::vec3 &PointList::operator[](size_t i) const
{
	return m_points[i];
}

void PolygonList::push_back(const glm::uvec4 &poly)
{
	m_polys.push_back(poly);
}

void PolygonList::push_back(glm::uvec4 &&poly)
{
	m_polys.emplace_back(std::move(poly));
}

void PolygonList::reserve(size_t n)
{
	m_polys.reserve(n);
}

void PolygonList::resize(size_t n)
{
	m_polys.resize(n);
}

size_t PolygonList::size() const
{
	return m_polys.size();
}

size_t PolygonList::byte_size() const
{
	return m_polys.size() * sizeof(glm::ivec4);
}

const void *PolygonList::data() const
{
	if (m_polys.empty()) {
		return nullptr;
	}

	return (&m_polys[0][0]);
}

glm::uvec4 &PolygonList::operator[](size_t i)
{
	return m_polys[i];
}

const glm::uvec4 &PolygonList::operator[](size_t i) const
{
	return m_polys[i];
}
