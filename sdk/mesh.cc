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

#include "mesh.h"

#include <algorithm>
#include <ego/utils.h>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "context.h"
#include "renderbuffer.h"
#include "util_parallel.h"

/* ************************************************************************** */

static RenderBuffer *create_surface_buffer()
{
	RenderBuffer *renderbuffer = new RenderBuffer;

	renderbuffer->set_shader_source(ego::VERTEX_SHADER, ego::util::str_from_file("shaders/object.vert"));
	renderbuffer->set_shader_source(ego::FRAGMENT_SHADER, ego::util::str_from_file("shaders/object.frag"));
	renderbuffer->finalize_shader();

	ProgramParams params;
	params.add_attribute("vertex");
	params.add_attribute("normal");
	params.add_attribute("vertex_color");
	params.add_uniform("matrix");
	params.add_uniform("MVP");
	params.add_uniform("N");
	params.add_uniform("for_outline");
	params.add_uniform("has_vcolors");

	renderbuffer->set_shader_params(params);

	return renderbuffer;
}

/* ************************************************************************** */

size_t Mesh::id = -1;

Mesh::Mesh()
    : Primitive()
    , m_renderbuffer(nullptr)
{
	add_attribute("normal", ATTR_TYPE_VEC3, 0);
	m_need_update = true;
}

Mesh::Mesh(const Mesh &other)
    : Primitive(other)
{
	/* Copy points. */
	auto points = other.points();
	m_point_list.resize(points->size());

	for (auto i = 0ul; i < points->size(); ++i) {
		m_point_list[i] = (*points)[i];
	}

	/* Copy points. */
	auto polys = other.polys();
	m_poly_list.resize(polys->size());

	for (auto i = 0ul; i < polys->size(); ++i) {
		m_poly_list[i] = (*polys)[i];
	}
}

Mesh::~Mesh()
{
	free_renderbuffer(m_renderbuffer);
}

PointList *Mesh::points()
{
	return &m_point_list;
}

const PointList *Mesh::points() const
{
	return &m_point_list;
}

PolygonList *Mesh::polys()
{
	return &m_poly_list;
}

const PolygonList *Mesh::polys() const
{
	return &m_poly_list;
}

void Mesh::update()
{
	if (m_need_update) {
		computeBBox(m_min, m_max);

		m_bbox.reset(new Cube(m_min, m_max));
		m_need_update = false;
	}
}

Primitive *Mesh::copy() const
{
	auto mesh = new Mesh(*this);
	mesh->tagUpdate();

	return mesh;
}

size_t Mesh::typeID() const
{
	return Mesh::id;
}

void Mesh::render(const ViewerContext &context)
{
	m_renderbuffer->render(context);
}

void Mesh::prepareRenderData()
{
	if (!m_need_data_update) {
		return;
	}

	if (!m_renderbuffer) {
		m_renderbuffer = create_surface_buffer();
	}

	auto normals = this->attribute("normal", ATTR_TYPE_VEC3);

	if (normals->size() != this->points()->size()) {
		computeNormals();
	}

	for (size_t i = 0, ie = this->points()->size(); i < ie; ++i) {
		m_point_list[i] = m_point_list[i] * glm::mat3(m_inv_matrix);
	}

	auto indices = std::vector<unsigned int>{};
	indices.reserve(this->polys()->size());

	auto polys = this->polys();

	for (auto i = 0ul, ie = polys->size(); i < ie; ++i) {
		const auto &quad = (*polys)[i];

		indices.push_back(quad[0]);
		indices.push_back(quad[1]);
		indices.push_back(quad[2]);

		if (quad[3] != INVALID_INDEX) {
			indices.push_back(quad[0]);
			indices.push_back(quad[2]);
			indices.push_back(quad[3]);
		}
	}

	m_renderbuffer->can_outline(true);

	m_renderbuffer->set_vertex_buffer("vertex",
	                                  m_point_list.data(),
	                                  m_point_list.byte_size(),
	                                  &indices[0],
	                                  indices.size() * sizeof(GLuint),
	                                  indices.size());

	m_renderbuffer->set_normal_buffer("normal", normals->data(), normals->byte_size());

	auto colors = this->attribute("color", ATTR_TYPE_VEC3);

	if (colors != nullptr) {
		m_renderbuffer->set_color_buffer("vertex_color", colors->data(), colors->byte_size());
	}

	m_need_data_update = false;
}

void Mesh::computeBBox(glm::vec3 &min, glm::vec3 &max)
{
	for (size_t i = 0, ie = m_point_list.size(); i < ie; ++i) {
		const auto &vert = m_point_list[i];

		if (vert.x < m_min.x) {
			m_min.x = vert.x;
		}
		else if (vert.x > m_max.x) {
			m_max.x = vert.x;
		}

		if (vert.y < m_min.y) {
			m_min.y = vert.y;
		}
		else if (vert.y > m_max.y) {
			m_max.y = vert.y;
		}

		if (vert.z < m_min.z) {
			m_min.z = vert.z;
		}
		else if (vert.z > m_max.z) {
			m_max.z = vert.z;
		}
	}

	min = m_min;
	max = m_max;
	m_dimensions = m_max - m_min;
}

static inline glm::vec3 get_normal(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2)
{
	const auto n0 = v0 - v1;
	const auto n1 = v2 - v1;

	return glm::cross(n1, n0);
}

void Mesh::computeNormals()
{
	auto normals = this->attribute("normal", ATTR_TYPE_VEC3);
	normals->resize(this->points()->size());

	auto polys = this->polys();

	parallel_for(tbb::blocked_range<size_t>(0, polys->size()),
	             [&](const tbb::blocked_range<size_t> &r)
	{
		for (auto i = r.begin(), ie = r.end(); i < ie ; ++i) {
			const auto &quad = (*polys)[i];

			const auto v0 = m_point_list[quad[0]];
			const auto v1 = m_point_list[quad[1]];
			const auto v2 = m_point_list[quad[2]];

			const auto normal = get_normal(v0, v1, v2);

			normals->vec3(quad[0], normals->vec3(quad[0]) + normal);
			normals->vec3(quad[1], normals->vec3(quad[1]) + normal);
			normals->vec3(quad[2], normals->vec3(quad[2]) + normal);

			if (quad[3] != INVALID_INDEX) {
				normals->vec3(quad[3], normals->vec3(quad[3]) + normal);
			}
		}
	});

	for (size_t i = 0, ie = this->points()->size(); i < ie ; ++i) {
		normals->vec3(i, -glm::normalize(normals->vec3(i)));
	}
}
