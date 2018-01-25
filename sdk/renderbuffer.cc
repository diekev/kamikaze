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

#include "renderbuffer.h"

#include <ego/utils.h>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <mutex>
#include <tbb/concurrent_vector.h>

#include "context.h"

/* ************************************************************************** */

void ProgramParams::add_attribute(const std::string &attribute)
{
	m_attributes.push_back(attribute);
}

void ProgramParams::add_uniform(const std::string &uniforms)
{
	m_uniforms.push_back(uniforms);
}

const std::vector<std::string> &ProgramParams::attributes() const
{
	return m_attributes;
}

const std::vector<std::string> &ProgramParams::uniforms() const
{
	return m_uniforms;
}

/* ************************************************************************** */

void DrawParams::set_draw_type(unsigned int draw_type)
{
	m_draw_type = draw_type;
}

unsigned int DrawParams::draw_type() const
{
	return m_draw_type;
}

void DrawParams::set_data_type(unsigned int data_type)
{
	m_data_type = data_type;
}

unsigned int DrawParams::data_type() const
{
	return m_data_type;
}

float DrawParams::line_size() const
{
	return m_line_size;
}

void DrawParams::set_line_size(float size)
{
	m_line_size = size;
}

float DrawParams::point_size() const
{
	return m_point_size;
}

void DrawParams::set_point_size(float size)
{
	m_point_size = size;
}

/* ************************************************************************** */

void RenderBuffer::set_shader_source(int shader_type, const std::string &source, std::ostream &os)
{
	m_program.load(shader_type, source, os);
}

void RenderBuffer::set_shader_params(const ProgramParams &params)
{
	m_program.enable();

	for (const auto &attribute : params.attributes()) {
		m_program.addAttribute(attribute);
	}

	for (const auto &uniform : params.uniforms()) {
		m_program.addUniform(uniform);
	}

	m_program.disable();
}

void RenderBuffer::set_draw_params(const DrawParams &params)
{
	m_params = params;
}

void RenderBuffer::finalize_shader(std::ostream &os)
{
	m_program.createAndLinkProgram(os);
}

void RenderBuffer::can_outline(bool yesno)
{
	m_can_outline = yesno;
}

void RenderBuffer::init()
{
	if (m_buffer_data == nullptr) {
		m_buffer_data = numero7::ego::BufferObject::create();
	}
}

void RenderBuffer::set_vertex_buffer(const std::string &attribute,
                                     const std::vector<glm::vec3> &vertices,
                                     const std::vector<unsigned int> &indices)
{
	init();

	m_elements = indices.size();

	m_buffer_data->bind();
	m_buffer_data->generateVertexBuffer(&vertices[0][0], vertices.size() * sizeof(glm::vec3));
	m_buffer_data->generateIndexBuffer(&indices[0], indices.size() * sizeof(unsigned int));
	m_buffer_data->attribPointer(m_program[attribute], 3);
	m_buffer_data->unbind();
}

void RenderBuffer::set_vertex_buffer(const std::string &attribute,
                                     const void *vertices_ptr,
                                     const size_t vertices_size,
                                     const void *indices_ptr,
                                     const size_t indices_size,
                                     const size_t elements)
{
	init();

	m_elements = elements;

	m_buffer_data->bind();
	m_buffer_data->generateVertexBuffer(vertices_ptr, vertices_size);

	if (indices_ptr) {
		m_buffer_data->generateIndexBuffer(indices_ptr, indices_size);
		m_index_drawing = true;
	}

	m_buffer_data->attribPointer(m_program[attribute], 3);
	m_buffer_data->unbind();
}

void RenderBuffer::set_extra_buffer(const std::string &attribute,
                                    const std::vector<glm::vec3> &values)
{
	init();

	m_buffer_data->bind();
	m_buffer_data->generateNormalBuffer(&values[0][0], values.size() * sizeof(glm::vec3));
	m_buffer_data->attribPointer(m_program[attribute], 3);
	m_buffer_data->unbind();
}

void RenderBuffer::set_normal_buffer(const std::string &attribute,
                                     const std::vector<glm::vec3> &normals)
{
	set_extra_buffer(attribute, normals);
	m_require_normal = true;
}

void RenderBuffer::set_extra_buffer(const std::string &attribute,
                                    const void *data,
                                    const size_t data_size)
{
	init();

	m_buffer_data->bind();
	m_buffer_data->generateNormalBuffer(data, data_size);
	m_buffer_data->attribPointer(m_program[attribute], 3);
	m_buffer_data->unbind();
}

void RenderBuffer::set_normal_buffer(const std::string &attribute,
                                     const void *normals,
                                     const size_t normals_size)
{
	set_extra_buffer(attribute, normals, normals_size);
	m_require_normal = true;
}

void RenderBuffer::set_color_buffer(const std::string &attribute,
                                    const void *colors,
                                    const size_t colors_size)
{
	init();

	m_buffer_data->bind();
	m_buffer_data->generateExtraBuffer(colors, colors_size);
	m_buffer_data->attribPointer(m_program[attribute], 3);
	m_buffer_data->unbind();

	m_require_color = true;
}

void RenderBuffer::set_color_buffer(const std::string &attribute,
                                    const std::vector<glm::vec3> &colors)
{
	set_color_buffer(attribute, &colors[0][0], colors.size() * sizeof(glm::vec3));
}

void RenderBuffer::render(const ViewerContext &context)
{
	if (!m_program.isValid()) {
		std::cerr << "Invalid Program\n";
		return;
	}

	if (m_params.draw_type() == GL_POINTS) {
		glPointSize(m_params.point_size());
	}
	else if (m_params.draw_type() == GL_LINES) {
		glLineWidth(m_params.line_size());
	}

	m_program.enable();
	m_buffer_data->bind();

	if (m_has_texture) {
		m_texture->bind();
	}

	glUniformMatrix4fv(m_program("matrix"), 1, GL_FALSE, glm::value_ptr(context.matrix()));
	glUniformMatrix4fv(m_program("MVP"), 1, GL_FALSE, glm::value_ptr(context.MVP()));

	if (m_require_normal) {
		glUniformMatrix3fv(m_program("N"), 1, GL_FALSE, glm::value_ptr(context.normal()));
	}

	if (m_require_color) {
		glUniform1i(m_program("has_vcolors"), m_require_color);
	}

	if (m_can_outline) {
		glUniform1i(m_program("for_outline"), context.for_outline());
	}

	if (m_index_drawing) {
		glDrawElements(m_params.draw_type(), m_elements, m_params.data_type(), nullptr);
	}
	else {
		glDrawArrays(m_params.draw_type(), 0, m_elements);
	}

	numero7::ego::util::GPU_check_errors("Error rendering buffer\n");

	if (m_has_texture) {
		m_texture->unbind();
	}

	m_buffer_data->unbind();
	m_program.disable();

	if (m_params.draw_type() == GL_POINTS) {
		glPointSize(1.0f);
	}
	else if (m_params.draw_type() == GL_LINES) {
		glLineWidth(1.0f);
	}
}

numero7::ego::Program *RenderBuffer::program()
{
	return &m_program;
}

numero7::ego::Texture3D *RenderBuffer::add_texture_3D()
{
	if (!m_texture.get()) {
		m_has_texture = true;
		m_texture.reset(new numero7::ego::Texture3D(0));
	}

	return m_texture.get();
}

/* ************************************************************************** */

tbb::concurrent_vector<RenderBuffer *> garbage_buffer;
std::mutex garbage_mutex;

void free_renderbuffer(RenderBuffer *buffer)
{
	garbage_buffer.push_back(buffer);
}

void purge_all_buffers()
{
	{
		std::unique_lock<std::mutex> lock(garbage_mutex);

		for (RenderBuffer *buffer : garbage_buffer) {
			delete buffer;
		}
	}

	garbage_buffer.clear();
}
