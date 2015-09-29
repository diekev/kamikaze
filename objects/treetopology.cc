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

#include <algorithm>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <openvdb/openvdb.h>

#include "render/GPUShader.h"
#include "render/GPUBuffer.h"

#include "util/util_opengl.h"
#include "util/utils.h"

#include "treetopology.h"

TreeTopology::TreeTopology(openvdb::FloatGrid::ConstPtr grid)
    : m_buffer_data(new GPUBuffer)
{
	m_shader.loadFromFile(GL_VERTEX_SHADER, "shader/tree_topo.vert");
	m_shader.loadFromFile(GL_FRAGMENT_SHADER, "shader/tree_topo.frag");

	m_shader.createAndLinkProgram();

	m_shader.use();
	{
		m_shader.addAttribute("vertex");
		m_shader.addAttribute("color");
		m_shader.addUniform("MVP");
	}
	m_shader.unUse();

	using openvdb::Index64;
	using openvdb::math::Vec3d;

    Index64 nodeCount = grid->tree().leafCount() + grid->tree().nonLeafCount();
    const Index64 N = nodeCount * 8;
	m_elements = N * 3;

	std::vector<glm::vec3> vertices(N);
	std::vector<glm::vec3> colors(N);
	std::vector<GLuint> indices(m_elements);

	openvdb::CoordBBox bbox;
	int idx = 0, count = 0, col_offset = 0, idx_offset = 0;

	glm::vec3 node_colors[] = {
	    glm::vec3(0.045f, 0.045f, 0.045f),         // root
	    glm::vec3(0.0432f, 0.33f, 0.0411023f),     // first internal node level
	    glm::vec3(0.871f, 0.394f, 0.01916f),       // intermediate internal node levels
	    glm::vec3(0.00608299f, 0.279541f, 0.625f)  // leaf nodes
	};

	for (openvdb::FloatTree::NodeCIter iter = grid->tree().cbeginNode(); iter; ++iter) {
        iter.getBoundingBox(bbox);

        /* Nodes are rendered as cell-centered */
        Vec3d min(bbox.min().asVec3d() - Vec3d(0.5));
        Vec3d max(bbox.max().asVec3d() + Vec3d(0.5));
		min = grid->indexToWorld(min);
		max = grid->indexToWorld(max);

		const glm::vec3 corners[8] = {
		    glm::vec3(min.x(), min.y(), min.z()),
		    glm::vec3(min.x(), min.y(), max.z()),
		    glm::vec3(max.x(), min.y(), max.z()),
		    glm::vec3(max.x(), min.y(), min.z()),
		    glm::vec3(min.x(), max.y(), min.z()),
		    glm::vec3(min.x(), max.y(), max.z()),
		    glm::vec3(max.x(), max.y(), max.z()),
		    glm::vec3(max.x(), max.y(), min.z()),
		};

        vertices[count++] = corners[0];
		vertices[count++] = corners[1];
		vertices[count++] = corners[2];
		vertices[count++] = corners[3];
		vertices[count++] = corners[4];
		vertices[count++] = corners[5];
		vertices[count++] = corners[6];
		vertices[count++] = corners[7];

        // edge 1
        indices[idx_offset++] = GLuint(idx);
        indices[idx_offset++] = GLuint(idx + 1);
        // edge 2
        indices[idx_offset++] = GLuint(idx + 1);
        indices[idx_offset++] = GLuint(idx + 2);
        // edge 3
        indices[idx_offset++] = GLuint(idx + 2);
        indices[idx_offset++] = GLuint(idx + 3);
        // edge 4
        indices[idx_offset++] = GLuint(idx + 3);
        indices[idx_offset++] = GLuint(idx);
        // edge 5
        indices[idx_offset++] = GLuint(idx + 4);
        indices[idx_offset++] = GLuint(idx + 5);
        // edge 6
        indices[idx_offset++] = GLuint(idx + 5);
        indices[idx_offset++] = GLuint(idx + 6);
        // edge 7
        indices[idx_offset++] = GLuint(idx + 6);
        indices[idx_offset++] = GLuint(idx + 7);
        // edge 8
        indices[idx_offset++] = GLuint(idx + 7);
        indices[idx_offset++] = GLuint(idx + 4);
        // edge 9
        indices[idx_offset++] = GLuint(idx);
        indices[idx_offset++] = GLuint(idx + 4);
        // edge 10
        indices[idx_offset++] = GLuint(idx + 1);
        indices[idx_offset++] = GLuint(idx + 5);
        // edge 11
        indices[idx_offset++] = GLuint(idx + 2);
        indices[idx_offset++] = GLuint(idx + 6);
        // edge 12
        indices[idx_offset++] = GLuint(idx + 3);
        indices[idx_offset++] = GLuint(idx + 7);

        idx += 8;

		const int level = iter.getLevel();
        glm::vec3 color = node_colors[(level == 0) ? 3 : (level == 1) ? 2 : 1];
		for (int i(0); i < 8; ++i) {
			colors[col_offset++] = color;
		}
    }

	m_buffer_data->bind();
	m_buffer_data->create_vertex_buffer(&(vertices[0].x), sizeof(glm::vec3) * N);
	m_buffer_data->create_index_buffer(&indices[0], sizeof(GLuint) * m_elements);
	m_buffer_data->attrib_pointer(m_shader["vertex"], 3);
	m_buffer_data->create_color_buffer(&colors[0][0], sizeof(glm::vec3) * colors.size());
	m_buffer_data->attrib_pointer(m_shader["color"], 3);
	m_buffer_data->unbind();
}

TreeTopology::~TreeTopology()
{
	delete m_buffer_data;
}

void TreeTopology::render(const glm::mat4 &MVP)
{
	glEnable(GL_DEPTH_TEST);

	m_shader.use();
	{
		m_buffer_data->bind();

		glUniformMatrix4fv(m_shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glDrawElements(GL_LINES, m_elements, GL_UNSIGNED_INT, nullptr);

		m_buffer_data->unbind();
	}
	m_shader.unUse();

	glEnable(GL_DEPTH_TEST);
}
