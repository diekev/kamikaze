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
{
	loadShader();
	m_need_update = true;
}

std::vector<glm::ivec4> &Mesh::quads()
{
	return m_quads;
}

const std::vector<glm::ivec4> &Mesh::quads() const
{
	return m_quads;
}

std::vector<glm::ivec3> &Mesh::tris()
{
	return m_tris;
}

const std::vector<glm::ivec3> &Mesh::tris() const
{
	return m_tris;
}

std::vector<glm::vec3> &Mesh::verts()
{
	return m_verts;
}

const std::vector<glm::vec3> &Mesh::verts() const
{
	return m_verts;
}

std::vector<glm::vec3> &Mesh::normals()
{
	return m_normals;
}

const std::vector<glm::vec3> &Mesh::normals() const
{
	return m_normals;
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

Primitive *Mesh::copy() const
{
	Mesh *mesh = new Mesh;

	mesh->verts().resize(this->verts().size());
	auto &verts = mesh->verts();

	for (size_t i = 0; i < verts.size(); ++i) {
		verts[i] = this->verts()[i];
	}

	mesh->tris().resize(this->tris().size());
	auto &tris = mesh->tris();

	for (size_t i = 0; i < tris.size(); ++i) {
		tris[i] = this->tris()[i];
	}

	mesh->quads().resize(this->quads().size());
	auto &quads = mesh->quads();

	for (size_t i = 0; i < quads.size(); ++i) {
		quads[i] = this->quads()[i];
	}

	mesh->normals().resize(this->normals().size());
	auto &normals = mesh->normals();

	for (size_t i = 0; i < normals.size(); ++i) {
		normals[i] = this->normals()[i];
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
	computeNormals();

	std::vector<unsigned int> indices;
	indices.reserve(m_quads.size() * 6 + m_tris.size() * 3);

	for (const auto &quad : m_quads) {
		indices.push_back(quad[0]);
		indices.push_back(quad[1]);
		indices.push_back(quad[2]);
		indices.push_back(quad[0]);
		indices.push_back(quad[2]);
		indices.push_back(quad[3]);
	}

	for (const auto &tri : m_tris) {
		indices.push_back(tri[0]);
		indices.push_back(tri[1]);
		indices.push_back(tri[2]);
	}

	m_elements = indices.size();

	m_buffer_data.reset(new ego::BufferObject());
	m_buffer_data->bind();
	m_buffer_data->generateVertexBuffer(&m_verts[0][0], m_verts.size() * sizeof(glm::vec3));
	m_buffer_data->generateIndexBuffer(&indices[0], m_elements * sizeof(GLuint));
	m_buffer_data->attribPointer(m_program["vertex"], 3);
	m_buffer_data->generateNormalBuffer(&m_normals[0], m_normals.size() * sizeof(glm::vec3));
	m_buffer_data->attribPointer(m_program["normal"], 3);
	m_buffer_data->unbind();

	ego::util::GPU_check_errors("Unable to create level set buffer");
}

void Mesh::computeBBox()
{
	for (const auto &vert : verts()) {
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
	m_normals.resize(m_verts.size());

	for (const auto &quad : m_quads) {
		const auto v0 = m_verts[quad[0]];
		const auto v1 = m_verts[quad[1]];
		const auto v2 = m_verts[quad[2]];

		const auto normal = get_normal(v0, v1, v2);

		m_normals[quad[0]] += normal;
		m_normals[quad[1]] += normal;
		m_normals[quad[2]] += normal;
		m_normals[quad[3]] += normal;
	}

	for (const auto &tri : m_tris) {
		const auto v0 = m_verts[tri[0]];
		const auto v1 = m_verts[tri[1]];
		const auto v2 = m_verts[tri[2]];

		const auto normal = get_normal(v0, v1, v2);

		m_normals[tri[0]] += normal;
		m_normals[tri[1]] += normal;
		m_normals[tri[2]] += normal;
	}

	for (auto &normal : m_normals) {
		normal = glm::normalize(normal);
	}
}

static Primitive *create_cube()
{
	Mesh *mesh = new Mesh;

	mesh->verts().resize(8);
	mesh->verts()[0] = glm::vec3(-0.5f, -0.5f, -0.5f);
	mesh->verts()[1] = glm::vec3( 0.5f, -0.5f, -0.5f);
	mesh->verts()[2] = glm::vec3( 0.5f, -0.5f,  0.5f);
	mesh->verts()[3] = glm::vec3(-0.5f, -0.5f,  0.5f);
	mesh->verts()[4] = glm::vec3(-0.5f,  0.5f, -0.5f);
	mesh->verts()[5] = glm::vec3( 0.5f,  0.5f, -0.5f);
	mesh->verts()[6] = glm::vec3( 0.5f,  0.5f,  0.5f);
	mesh->verts()[7] = glm::vec3(-0.5f,  0.5f,  0.5f);

	mesh->quads().resize(6);
	mesh->quads()[0] = glm::ivec4(1, 0, 4, 5);
	mesh->quads()[1] = glm::ivec4(2, 1, 5, 6);
	mesh->quads()[2] = glm::ivec4(3, 2, 6, 7);
	mesh->quads()[3] = glm::ivec4(0, 3, 7, 4);
	mesh->quads()[4] = glm::ivec4(2, 3, 0, 1);
	mesh->quads()[5] = glm::ivec4(5, 4, 7, 6);

	mesh->update();
	mesh->generateGPUData();

	return mesh;
}

static Primitive *create_plane()
{
	Mesh *mesh = new Mesh;

	mesh->verts().resize(4);
	mesh->verts()[0] = glm::vec3(-1.0f,  0.0f, -1.0f);
	mesh->verts()[1] = glm::vec3( 1.0f,  0.0f, -1.0f);
	mesh->verts()[2] = glm::vec3( 1.0f,  0.0f,  1.0f);
	mesh->verts()[3] = glm::vec3(-1.0f,  0.0f,  1.0f);

	mesh->quads().resize(1);
	mesh->quads()[0] = glm::ivec4(0, 1, 2, 3);

	mesh->update();
	mesh->generateGPUData();

	return mesh;
}

static Primitive *create_torus()
{
	Mesh *mesh = new Mesh;

	const auto major_segment = 48;
	const auto minor_segment = 24;
	const auto major_radius = 1.0f;
	const auto minor_radius = 0.25f;

	constexpr auto tau = static_cast<float>(M_PI) * 2.0f;

	const auto vertical_angle_stride = tau / static_cast<float>(major_segment);
	const auto horizontal_angle_stride = tau / static_cast<float>(minor_segment);

	int f1 = 0, f2, f3, f4;
	const auto tot_verts = major_segment * minor_segment;
	mesh->verts().reserve(tot_verts);

	for (int i = 0; i < major_segment; ++i) {
		auto theta = vertical_angle_stride * i;

		for (int j = 0; j < minor_segment; ++j) {
			auto phi = horizontal_angle_stride * j;

			auto x = glm::cos(theta) * (major_radius + minor_radius * glm::cos(phi));
			auto y = minor_radius * glm::sin(phi);
			auto z = glm::sin(theta) * (major_radius + minor_radius * glm::cos(phi));

			auto vec = glm::vec3(x, y, z);

			mesh->verts().push_back(vec);

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
				mesh->quads().emplace_back(f1, f3, f4, f2);
			}
			else {
				mesh->quads().emplace_back(f2, f1, f3, f4);
			}

			++f1;
		}
	}

	mesh->update();
	mesh->generateGPUData();

	return mesh;
}

void Mesh::registerSelf(ObjectFactory *factory)
{
	factory->registerType("Box Mesh", create_cube);
	factory->registerType("Plane Mesh", create_plane);
	factory->registerType("Torus Mesh", create_torus);
}
