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

#include <ego/bufferobject.h>
#include <ego/program.h>

#include <glm/glm.hpp>

#include <vector>

class ViewerContext;

class ProgramParams {
	std::vector<std::string> m_attributes = {};
	std::vector<std::string> m_uniforms = {};

public:
	ProgramParams() = default;

	void add_attribute(const std::string &attribute);
	void add_uniform(const std::string &uniforms);

	const std::vector<std::string> &attributes() const;
	const std::vector<std::string> &uniforms() const;
};

class DrawParams {
	unsigned int m_draw_type = 0x0004; /* GL_TRIANGLES */
	unsigned int m_data_type = 0x1405; /* GL_UNSIGNED_INT */
	float m_line_size = 1.0f;
	float m_point_size = 1.0f;

public:
	DrawParams() = default;

	void set_draw_type(unsigned int draw_type);
	unsigned int draw_type() const;

	void set_data_type(unsigned int data_type);
	unsigned int data_type() const;

	void set_line_size(float size);
	float line_size() const;

	void set_point_size(float size);
	float point_size() const;
};

class RenderBuffer {
	numero7::ego::BufferObject::Ptr m_buffer_data = nullptr;
	numero7::ego::Program m_program;
	size_t m_elements = 0;

	DrawParams m_params;

	bool m_require_normal = false;
	bool m_require_color = false;
	bool m_can_outline = false;
	bool m_index_drawing = false;

public:
	void set_shader_source(int shader_type, const std::string &source, std::ostream &os = std::cerr);

	void set_shader_params(const ProgramParams &params);

	void set_draw_params(const DrawParams &params);

	void finalize_shader(std::ostream &os = std::cerr);

	void can_outline(bool yesno);

	void set_vertex_buffer(const std::string &attribute,
	                       const std::vector<glm::vec3> &vertices,
	                       const std::vector<unsigned int> &indices);

	void set_vertex_buffer(const std::string &attribute,
	                       const void *vertices_ptr,
	                       const size_t vertices_size,
	                       const void *indices_ptr,
	                       const size_t indices_size,
	                       const size_t elements);

	void set_extra_buffer(const std::string &attribute,
	                      const std::vector<glm::vec3> &values);

	void set_extra_buffer(const std::string &attribute,
	                      const void *data,
	                      const size_t data_size);

	void set_normal_buffer(const std::string &attribute,
	                       const std::vector<glm::vec3> &normals);

	void set_normal_buffer(const std::string &attribute,
	                       const void *normals,
	                       const size_t normals_size);

	void set_color_buffer(const std::string &attribute,
	                      const std::vector<glm::vec3> &normals);

	void set_color_buffer(const std::string &attribute,
	                      const void *normals,
	                      const size_t normals_size);

	void render(const ViewerContext &context);

	numero7::ego::Program *program();

private:
	void init();
};

void free_renderbuffer(RenderBuffer *buffer);
void purge_all_buffers();
