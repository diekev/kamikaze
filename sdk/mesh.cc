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

#include <ego/utils.h>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "context.h"
#include "util_parallel.h"

#ifndef UNUSED
#define UNUSED(x) static_cast<void>(x);
#endif

size_t Mesh::id = -1;

Mesh::Mesh()
    : Primitive()
    , m_need_data_update(true)
{
	addAttribute("normal", ATTR_TYPE_VEC3);
	m_need_update = true;
}

Mesh::~Mesh()
{
	for (auto &attr : m_attributes) {
		delete attr;
	}
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

Attribute *Mesh::attribute(const std::string &name, AttributeType type)
{
	auto iter = std::find_if(m_attributes.begin(), m_attributes.end(),
	                         [&](Attribute *attr)
	{
		return (attr->type() == type) && (attr->name() == name);
	});

	if (iter == m_attributes.end()) {
		return nullptr;
	}

	return *iter;
}

void Mesh::addAttribute(Attribute *attr)
{
	if (attribute(attr->name(), attr->type()) == nullptr) {
		m_attributes.push_back(attr);
	}
}

Attribute *Mesh::addAttribute(const std::string &name, AttributeType type, size_t size)
{
	auto attr = attribute(name, type);

	if (attr == nullptr) {
		attr = new Attribute(name, type, size);
		m_attributes.push_back(attr);
	}

	return attr;
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
	auto mesh = new Mesh;

	auto points = mesh->points();
	points->resize(this->points()->size());

	for (auto i = 0ul; i < points->size(); ++i) {
		(*points)[i] = m_point_list[i];
	}

	auto polys = mesh->polys();
	polys->resize(this->polys()->size());

	for (auto i = 0ul; i < polys->size(); ++i) {
		(*polys)[i] = m_poly_list[i];
	}

	/* XXX TODO: we need a better way to copy/update default attributes
	 * (e.g. normals) */
	for (auto &attr : mesh->m_attributes) {
		delete attr;
	}

	for (const auto &attr : m_attributes) {
		mesh->addAttribute(new Attribute(*attr));
	}

	mesh->tagUpdate();

	return mesh;
}

size_t Mesh::typeID() const
{
	return Mesh::id;
}

void Mesh::loadShader()
{
	m_program.load(ego::VERTEX_SHADER, ego::util::str_from_file("shaders/object.vert"));
	m_program.load(ego::FRAGMENT_SHADER, ego::util::str_from_file("shaders/object.frag"));
	m_program.createAndLinkProgram();

	m_program.enable();
	{
		m_program.addAttribute("vertex");
		m_program.addAttribute("normal");
		m_program.addUniform("matrix");
		m_program.addUniform("MVP");
		m_program.addUniform("N");
		m_program.addUniform("for_outline");
	}
	m_program.disable();
}

void Mesh::render(const ViewerContext * const context, const bool for_outline)
{
	if (m_program.isValid()) {
		m_program.enable();
		m_buffer_data->bind();

		glUniformMatrix4fv(m_program("matrix"), 1, GL_FALSE, glm::value_ptr(context->matrix()));
		glUniformMatrix4fv(m_program("MVP"), 1, GL_FALSE, glm::value_ptr(context->MVP()));
		glUniformMatrix3fv(m_program("N"), 1, GL_FALSE, glm::value_ptr(context->normal()));
		glUniform1i(m_program("for_outline"), for_outline);
		glDrawElements(m_draw_type, m_elements, GL_UNSIGNED_INT, nullptr);

		m_buffer_data->unbind();
		m_program.disable();
	}
}

void Mesh::setCustomUIParams(ParamCallback *cb)
{
	UNUSED(cb);
}

void Mesh::prepareRenderData()
{
	if (!m_need_data_update) {
		return;
	}

	if (!m_program.isValid()) {
		loadShader();
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

	m_elements = indices.size();

	m_buffer_data.reset(new ego::BufferObject());
	m_buffer_data->bind();
	m_buffer_data->generateVertexBuffer(m_point_list.data(), m_point_list.byte_size());
	m_buffer_data->generateIndexBuffer(&indices[0], m_elements * sizeof(GLuint));
	m_buffer_data->attribPointer(m_program["vertex"], 3);
	m_buffer_data->generateNormalBuffer(normals->data(), normals->byte_size());
	m_buffer_data->attribPointer(m_program["normal"], 3);
	m_buffer_data->unbind();

	ego::util::GPU_check_errors("Unable to create level set buffer");

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
