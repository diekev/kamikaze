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

#include <memory>

#include "GPUProgram.h"
#include "GPUBuffer.h"

#include "util/util_render.h"

class Object {
protected:
	std::unique_ptr<GPUBuffer> m_buffer_data;
	GPUProgram m_program;
	size_t m_elements;

	glm::vec3 m_size, m_inv_size;
	glm::vec3 m_min, m_max;

	bool m_draw_bbox, m_draw_topology;

public:
	Object();
	~Object() = default;

	virtual bool intersect(const Ray &ray, float &min) const;
	virtual void render(const glm::mat4 &MVP, const glm::mat3 &N, const glm::vec3 &view_dir) = 0;

	virtual void toggleBBoxDrawing();
	virtual void toggleTopologyDrawing();
};
