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

#include "attribute.h"
#include "geomlists.h"
#include "primitive.h"

class RenderBuffer;

class Mesh : public Primitive {
	PointList m_point_list = {};
	PolygonList m_poly_list = {};

	RenderBuffer *m_renderbuffer = nullptr;

public:
	Mesh();
	Mesh(const Mesh &other);
	~Mesh();

	/**
	 * @brief points The points (or vertices) of this mesh.
	 * @return A pointer to the list of points contained in this mesh.
	 */
	PointList *points();

	/**
	 * @brief points The points (or vertices) of this mesh.
	 * @return A pointer to the const list of points contained in this mesh.
	 */
	const PointList *points() const;

	/**
	 * @brief polys The polys of this mesh.
	 * @return A pointer to the list of polys contained in this mesh.
	 */
	PolygonList *polys();

	/**
	 * @brief polys The polys of this mesh.
	 * @return A pointer to the const list of polys contained in this mesh.
	 */
	const PolygonList *polys() const;

	void update() override;

	void render(const ViewerContext &context) override;

	void prepareRenderData() override;

	void computeBBox(glm::vec3 &min, glm::vec3 &max) override;

	Primitive *copy() const override;

	static size_t id;
	size_t typeID() const override;

private:
	void computeNormals();
};

template <typename CharT, typename CharTraits>
std::basic_ostream<CharT, CharTraits> &operator<<(std::basic_ostream<CharT, CharTraits> &os, const Mesh &mesh)
{
	os << "Mesh: " << mesh.name() << '\n'
	   << "\tvertices: " << mesh.points()->size() << '\n'
	   << "\tpolys: " << mesh.polys()->size() << '\n';

	return os;
}
