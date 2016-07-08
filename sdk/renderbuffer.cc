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

void RenderBuffer::finalize_shader(std::ostream &os)
{
	m_program.createAndLinkProgram(os);
}

void RenderBuffer::can_outline(bool yesno)
{
	m_can_outline = yesno;
}

void RenderBuffer::set_vertex_buffer(const std::string &attribute,
                                     const std::vector<glm::vec3> &vertices,
                                     const std::vector<unsigned int> &indices)
{
	if (m_buffer_data == nullptr) {
		m_buffer_data = ego::BufferObject::create();
	}

	m_elements = indices.size();

	m_buffer_data->bind();
	m_buffer_data->generateVertexBuffer(&vertices[0][0], vertices.size() * sizeof(glm::vec3));
	m_buffer_data->generateIndexBuffer(&indices[0], indices.size() * sizeof(unsigned int));
	m_buffer_data->attribPointer(m_program[attribute], 3);
	m_buffer_data->unbind();
}

void RenderBuffer::set_extra_buffer(const std::string &attribute,
                                    const std::vector<glm::vec3> &values)
{
	if (m_buffer_data == nullptr) {
		m_buffer_data = ego::BufferObject::create();
	}

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

void RenderBuffer::render(const ViewerContext * const context, const bool for_outline)
{
	if (!m_program.isValid()) {
		std::cerr << "Invalid Program\n";
		return;
	}

	m_program.enable();
	m_buffer_data->bind();

	glUniformMatrix4fv(m_program("matrix"), 1, GL_FALSE, glm::value_ptr(context->matrix()));
	glUniformMatrix4fv(m_program("MVP"), 1, GL_FALSE, glm::value_ptr(context->MVP()));

	if (m_require_normal) {
		glUniformMatrix3fv(m_program("N"), 1, GL_FALSE, glm::value_ptr(context->normal()));
	}

	if (m_can_outline) {
		glUniform1i(m_program("for_outline"), for_outline);
	}

	glDrawElements(m_draw_type, m_elements, m_data_type, nullptr);

	ego::util::GPU_check_errors("Error rendering buffer\n");

	m_buffer_data->unbind();
	m_program.disable();
}
