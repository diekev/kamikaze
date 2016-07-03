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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#include "cube.h"
#include "context.h"

#include <ego/utils.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Cube::Cube(const glm::vec3 &min, const glm::vec3 &max)
{
	m_program.load(ego::VERTEX_SHADER, ego::util::str_from_file("shaders/flat_shader.vert"));
	m_program.load(ego::FRAGMENT_SHADER, ego::util::str_from_file("shaders/flat_shader.frag"));

	m_program.createAndLinkProgram();

	m_program.enable();
	{
		m_program.addAttribute("vertex");
		m_program.addUniform("matrix");
		m_program.addUniform("MVP");
	}
	m_program.disable();

	m_draw_type = GL_LINES;

	const glm::vec3 vertices[8] = {
	    glm::vec3(min[0], min[1], min[2]),
	    glm::vec3(max[0], min[1], min[2]),
	    glm::vec3(max[0], max[1], min[2]),
	    glm::vec3(min[0], max[1], min[2]),
	    glm::vec3(min[0], min[1], max[2]),
	    glm::vec3(max[0], min[1], max[2]),
	    glm::vec3(max[0], max[1], max[2]),
	    glm::vec3(min[0], max[1], max[2])
	};

	m_min = min;
	m_max = max;
	m_pos = (min + max) / 2.0f;
	m_dimensions = max - min;

	updateMatrix();

	for (int i = 0; i < 8; ++i) {
		m_vertices.push_back(vertices[i] * glm::mat3(m_inv_matrix));
	}

	const GLushort indices[24] = {
	    0, 1, 1, 2,
	    2, 3, 3, 0,
	    4, 5, 5, 6,
	    6, 7, 7, 4,
	    0, 4, 1, 5,
	    2, 6, 3, 7
	};

	m_elements = 24;

	m_buffer_data = ego::BufferObject::create();
	m_buffer_data->bind();
	m_buffer_data->generateVertexBuffer(&m_vertices[0][0], m_vertices.size() * sizeof(glm::vec3));
	m_buffer_data->generateIndexBuffer(&indices[0], sizeof(indices));
	m_buffer_data->attribPointer(m_program["vertex"], 3);
	m_buffer_data->unbind();
}

void Cube::render(ViewerContext *context, const bool /*for_outline*/)
{
	if (m_program.isValid()) {
		m_program.enable();
		m_buffer_data->bind();

		glUniformMatrix4fv(m_program("matrix"), 1, GL_FALSE, glm::value_ptr(context->matrix()));
		glUniformMatrix4fv(m_program("MVP"), 1, GL_FALSE, glm::value_ptr(context->MVP()));

		glDrawElements(m_draw_type, m_elements, GL_UNSIGNED_SHORT, nullptr);

		m_buffer_data->unbind();
		m_program.disable();
	}
}

void Cube::updateMatrix()
{
	m_min = m_pos - m_dimensions / 2.0f;
	m_max = m_min + m_dimensions;

	m_matrix = glm::mat4(1.0f);
	m_matrix = glm::translate(m_matrix, m_pos);
	m_matrix = glm::scale(m_matrix, 1.0f * m_dimensions);

	m_inv_matrix = glm::inverse(m_matrix);
}
