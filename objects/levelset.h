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

#pragma once

#include <glm/glm.hpp>
#include <openvdb/openvdb.h>

#include "render/GPUProgram.h"
#include "render/GPUBuffer.h"

#include "cube.h"
#include "treetopology.h"

class LevelSet {
	std::unique_ptr<GPUBuffer> m_buffer_data;
	GPUProgram m_program;
	size_t m_elements;

	std::unique_ptr<Cube> m_bbox;
	std::unique_ptr<TreeTopology> m_topology;

	glm::vec3 m_min, m_max;
	glm::vec3 m_size, m_inv_size;

	bool m_draw_bbox, m_draw_topology;

	void generate_mesh(openvdb::FloatGrid::ConstPtr grid);

public:
	LevelSet();
	LevelSet(openvdb::FloatGrid::Ptr &grid);
	~LevelSet() = default;

	void render(const glm::mat4 &MVP, const glm::mat3 &N);

	void loadShader();

	void toggleBBoxDrawing();
	void toggleTopologyDrawing();
};
