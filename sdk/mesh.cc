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

#ifndef UNUSED
#define UNUSED(x) static_cast<void>(x);
#endif

Mesh::Mesh()
    : Primitive()
    , m_need_data_update(true)
{
	loadShader();
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
		computeBBox();
		updateMatrix();

		m_bbox.reset(new Cube(m_min, m_max));
		m_need_update = false;
	}
}

void Mesh::tag_update()
{
	m_need_update = true;
	m_need_data_update = true;
}

Primitive *Mesh::copy() const
{
	Mesh *mesh = new Mesh;

	PointList *points = mesh->points();
	points->resize(this->points()->size());

	for (size_t i = 0; i < points->size(); ++i) {
		(*points)[i] = m_point_list[i];
	}

	PolygonList *polys = mesh->polys();
	polys->resize(this->polys()->size());

	for (size_t i = 0; i < polys->size(); ++i) {
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

	mesh->update();
	mesh->generateGPUData();

	return mesh;
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

void Mesh::render(ViewerContext *context, const bool for_outline)
{
	if (m_need_data_update) {
		generateGPUData();
		m_need_data_update = false;
	}

	if (m_program.isValid()) {
		m_program.enable();
		m_buffer_data->bind();

		glUniformMatrix4fv(m_program("matrix"), 1, GL_FALSE, glm::value_ptr(m_matrix));
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

void Mesh::generateGPUData()
{
	Attribute *normals = this->attribute("normal", ATTR_TYPE_VEC3);

	if (normals->size() != this->points()->size()) {
		computeNormals();
	}

	for (size_t i = 0, ie = this->points()->size(); i < ie; ++i) {
		m_point_list[i] = m_point_list[i] * glm::mat3(m_inv_matrix);
	}

	std::vector<unsigned int> indices;
	indices.reserve(this->polys()->size());

	PolygonList *polys = this->polys();

	for (size_t i = 0, ie = polys->size(); i < ie; ++i) {
		const auto &quad = (*polys)[i];

		indices.push_back(quad[0]);
		indices.push_back(quad[1]);
		indices.push_back(quad[2]);

		if (quad[3] != std::numeric_limits<int>::max()) {
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
}

void Mesh::computeBBox()
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
	Attribute *normals = this->attribute("normal", ATTR_TYPE_VEC3);
	normals->resize(this->points()->size());

	PolygonList *polys = this->polys();

	for (size_t i = 0, ie = polys->size(); i < ie; ++i) {
		const auto &quad = (*polys)[i];

		const auto v0 = m_point_list[quad[0]];
		const auto v1 = m_point_list[quad[1]];
		const auto v2 = m_point_list[quad[2]];

		const auto normal = get_normal(v0, v1, v2);

		normals->vec3(quad[0], normals->vec3(quad[0]) + normal);
		normals->vec3(quad[1], normals->vec3(quad[1]) + normal);
		normals->vec3(quad[2], normals->vec3(quad[2]) + normal);

		if (quad[3] != std::numeric_limits<unsigned int>::max()) {
			normals->vec3(quad[3], normals->vec3(quad[3]) + normal);
		}
	}

	for (size_t i = 0, ie = this->points()->size(); i < ie ; ++i) {
		normals->vec3(i, -glm::normalize(normals->vec3(i)));
	}
}

static Primitive *create_cube()
{
	Mesh *mesh = new Mesh;

	PointList *points = mesh->points();
	points->reserve(8);

	points->push_back(glm::vec3(-0.5f, -0.5f, -0.5f));
	points->push_back(glm::vec3( 0.5f, -0.5f, -0.5f));
	points->push_back(glm::vec3( 0.5f, -0.5f,  0.5f));
	points->push_back(glm::vec3(-0.5f, -0.5f,  0.5f));
	points->push_back(glm::vec3(-0.5f,  0.5f, -0.5f));
	points->push_back(glm::vec3( 0.5f,  0.5f, -0.5f));
	points->push_back(glm::vec3( 0.5f,  0.5f,  0.5f));
	points->push_back(glm::vec3(-0.5f,  0.5f,  0.5f));

	PolygonList *polys = mesh->polys();
	polys->resize(6);
	polys->push_back(glm::ivec4(1, 0, 4, 5));
	polys->push_back(glm::ivec4(2, 1, 5, 6));
	polys->push_back(glm::ivec4(3, 2, 6, 7));
	polys->push_back(glm::ivec4(0, 3, 7, 4));
	polys->push_back(glm::ivec4(2, 3, 0, 1));
	polys->push_back(glm::ivec4(5, 4, 7, 6));

	mesh->tag_update();

	return mesh;
}

static Primitive *create_plane()
{
	Mesh *mesh = new Mesh;

	PointList *points = mesh->points();
	points->reserve(4);

	points->push_back(glm::vec3(-1.0f,  0.0f, -1.0f));
	points->push_back(glm::vec3( 1.0f,  0.0f, -1.0f));
	points->push_back(glm::vec3( 1.0f,  0.0f,  1.0f));
	points->push_back(glm::vec3(-1.0f,  0.0f,  1.0f));

	PolygonList *polys = mesh->polys();
	polys->push_back(glm::ivec4(0, 1, 2, 3));

	mesh->tag_update();

	return mesh;
}

static Primitive *create_torus()
{
	Mesh *mesh = new Mesh;
	PointList *points = mesh->points();
	PolygonList *polys = mesh->polys();

	const auto major_segment = 48;
	const auto minor_segment = 24;
	const auto major_radius = 1.0f;
	const auto minor_radius = 0.25f;

	constexpr auto tau = static_cast<float>(M_PI) * 2.0f;

	const auto vertical_angle_stride = tau / static_cast<float>(major_segment);
	const auto horizontal_angle_stride = tau / static_cast<float>(minor_segment);

	int f1 = 0, f2, f3, f4;
	const auto tot_verts = major_segment * minor_segment;

	points->reserve(tot_verts);

	for (int i = 0; i < major_segment; ++i) {
		auto theta = vertical_angle_stride * i;

		for (int j = 0; j < minor_segment; ++j) {
			auto phi = horizontal_angle_stride * j;

			auto x = glm::cos(theta) * (major_radius + minor_radius * glm::cos(phi));
			auto y = minor_radius * glm::sin(phi);
			auto z = glm::sin(theta) * (major_radius + minor_radius * glm::cos(phi));

			points->push_back(glm::vec3(x, y, z));

			if (j + 1 == minor_segment) {
				f2 = i * minor_segment;
				f3 = f1 + minor_segment;
				f4 = f2 + minor_segment;
			}
			else {
				f2 = f1 + 1;
				f3 = f1 + minor_segment;
				f4 = f3 + 1;
			}

			if (f2 >= tot_verts) {
				f2 -= tot_verts;
			}
			if (f3 >= tot_verts) {
				f3 -= tot_verts;
			}
			if (f4 >= tot_verts) {
				f4 -= tot_verts;
			}

			if (f2 > 0) {
				polys->push_back(glm::ivec4(f1, f3, f4, f2));
			}
			else {
				polys->push_back(glm::ivec4(f2, f1, f3, f4));
			}

			++f1;
		}
	}

	mesh->tag_update();

	return mesh;
}

void Mesh::registerSelf(PrimitiveFactory *factory)
{
	factory->registerType("Box Mesh", create_cube);
	factory->registerType("Plane Mesh", create_plane);
	factory->registerType("Torus Mesh", create_torus);
}
