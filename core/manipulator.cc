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

#include "manipulator.h"

#include <ego/utils.h>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include <kamikaze/context.h>

void add_arrow(std::vector<glm::vec3> &points, const int axis)
{
	const auto osize = points.size();

	/* add line */
	points.push_back(glm::vec3(-1.0f, 0.0f, 0.0f));
    points.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

	/* add arrow */
    points.push_back(glm::vec3(0.95f, 0.0f, -0.05f));
    points.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    points.push_back(glm::vec3(0.95f, 0.0f, 0.05f));
    points.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

	switch (axis) {
		default:
		case 0:
			/* nothing to do */
			break;
		case 1:
			for (auto i = osize, ie = points.size(); i < ie; ++i) {
				points[i] = glm::rotateY(points[i], static_cast<float>(M_PI_2));
			}
			break;
		case 2:
			for (auto i = osize, ie = points.size(); i < ie; ++i) {
				points[i] = glm::rotateZ(points[i], static_cast<float>(M_PI_2));
			}
			break;
	}
}

void Manipulator::updateMatrix()
{
	m_min = m_pos - m_dimensions / 2.0f;
	m_max = m_min + m_dimensions;

	m_matrix = glm::mat4(1.0f);
	m_matrix = glm::translate(m_matrix, m_pos);
	m_matrix = glm::scale(m_matrix, 1.0f * m_dimensions);

	m_inv_matrix = glm::inverse(m_matrix);
}

Manipulator::Manipulator()
{
	m_program.load(ego::VERTEX_SHADER, ego::util::str_from_file("shaders/tree_topology.vert"));
	m_program.load(ego::FRAGMENT_SHADER, ego::util::str_from_file("shaders/tree_topology.frag"));

	m_program.createAndLinkProgram();

	m_program.enable();
	{
		m_program.addAttribute("vertex");
		m_program.addAttribute("color");
		m_program.addUniform("matrix");
		m_program.addUniform("MVP");
	}
	m_program.disable();

	m_draw_type = GL_LINES;

	m_min = glm::vec3(0.0f, 0.0f, 0.0f);
	m_max = glm::vec3(1.0f, 1.0f, 1.0f);
	m_dimensions = m_max - m_min;

	updateMatrix();

	/* generate vertices */
	add_arrow(m_vertices, 0);
	add_arrow(m_vertices, 1);
	add_arrow(m_vertices, 2);

	m_elements = m_vertices.size();

	for (int i = 0; i < m_elements; ++i) {
		m_vertices[i] = m_vertices[i] * glm::mat3(m_inv_matrix);
	}

	/* generate colors */
	std::vector<glm::vec3> colors;
	colors.resize(m_elements);

	const auto stride = m_elements / 3;
	for (int i = 0; i < stride; ++i) {
		colors[i]              = glm::vec3(1.0f, 0.0f, 0.0f);
		colors[i + stride]     = glm::vec3(0.0f, 1.0f, 0.0f);
		colors[i + stride * 2] = glm::vec3(0.0f, 0.0f, 1.0f);
	}

	/* generate buffer */
	m_buffer_data = ego::BufferObject::create();
	m_buffer_data->bind();
	m_buffer_data->generateVertexBuffer(&m_vertices[0][0], m_vertices.size() * sizeof(glm::vec3));
	m_buffer_data->attribPointer(m_program["vertex"], 3);
	m_buffer_data->generateNormalBuffer(&colors[0][0], sizeof(glm::vec3) * m_elements);
	m_buffer_data->attribPointer(m_program["color"], 3);
	m_buffer_data->unbind();
}

void Manipulator::render(ViewerContext *context)
{
	if (m_program.isValid()) {
		glLineWidth(2.0f);

		m_program.enable();
		m_buffer_data->bind();

		glUniformMatrix4fv(m_program("matrix"), 1, GL_FALSE, glm::value_ptr(m_matrix));
		glUniformMatrix4fv(m_program("MVP"), 1, GL_FALSE, glm::value_ptr(context->MVP()));
		glDrawArrays(m_draw_type, 0, m_elements);

		m_buffer_data->unbind();
		m_program.disable();

		glLineWidth(1.0f);
	}
}
