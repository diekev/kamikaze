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
	PointList m_point_list;
	PolygonList m_poly_list;

	std::vector<Attribute *> m_attributes;

	RenderBuffer *m_renderbuffer;

	bool m_need_data_update;

public:
	Mesh();
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

	/**
	 * @brief attribute Return an attribute from this primitive's attibute list.
	 * @param name The name of the attribute to look up.
	 * @param type The type of the attribute to look up.
	 *
	 * @return The attribute corresponding to the given name and type, nullptr
	 *         if no such attribute exists.
	 */
	Attribute *attribute(const std::string &name, AttributeType type);

	/**
	 * @brief attribute Add an attribute to this primitive's attibute list.
	 * @param attr The attribute to add.
	 */
	void addAttribute(Attribute *attr);

	/**
	 * @brief attribute Add an attribute to this primitive's attibute list.
	 * @param name The name of the attribute to add.
	 * @param type The type of the attribute to add.
	 *
	 * @return The newly added attribute. If there is already an attribute with
	 *         the given name and type in this primitive's attibute list,  it is
	 *         returned, and no new attribute is created.
	 */
	Attribute *addAttribute(const std::string &name, AttributeType type, size_t size = 0);

	void update() override;

	void render(const ViewerContext * const context, const bool for_outline) override;

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
