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
#include <vector>

#include "GPUProgram.h"
#include "GPUBuffer.h"

#include "util/util_render.h"

enum {
	DRAW_WIRE = 0,
	DRAW_QUADS = 1,
};

class Object {
protected:
	std::unique_ptr<GPUBuffer> m_buffer_data;
	GPUProgram m_program;
	size_t m_elements;
	GLenum m_draw_type;

	std::vector<glm::vec3> m_vertices;
	glm::vec3 m_dimensions, m_scale, m_inv_size, m_rotation;
	glm::vec3 m_min, m_max, m_pos;
	glm::mat4 m_matrix, m_inv_matrix;

	bool m_draw_bbox, m_draw_topology, m_need_update;

	void updateMatrix();

public:
	Object();
	~Object() = default;

	virtual bool intersect(const Ray &ray, float &min) const;
	virtual void render(const glm::mat4 &MVP, const glm::mat3 &N, const glm::vec3 &view_dir) = 0;
	void setDrawType(int draw_type);

	virtual void drawBBox(const bool b);
	virtual bool drawBBox() const { return m_draw_bbox; }
	virtual void drawTreeTopology(const bool b);
	virtual bool drawTreeTopology() const { return m_draw_topology; }

	glm::vec3 pos() const;
	void setPos(const glm::vec3 &pos);
	glm::vec3 scale() const;
	void setScale(const glm::vec3 &scale);
	glm::vec3 rotation() const;
	void setRotation(const glm::vec3 &rotation);
};
