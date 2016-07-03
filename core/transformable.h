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

#include <glm/glm.hpp>

class Transformable {
protected:
	glm::vec3 m_pos = glm::vec3(0.0f);
	glm::vec3 m_rot = glm::vec3(0.0f);
	glm::vec3 m_scale = glm::vec3(1.0f);

	glm::mat4 m_matrix = glm::mat4(1.0f);
	glm::mat4 m_inv_matrix = glm::mat4(0.0f);

public:
	Transformable() = default;
	virtual ~Transformable() = default;

	void recompute_matrix();

	void pos(const glm::vec3 &p);
	const glm::vec3 &pos() const;

	void rot(const glm::vec3 &r);
	const glm::vec3 &rot() const;

	void scale(const glm::vec3 &s);
	const glm::vec3 &scale() const;

	void matrix(const glm::mat4 &m);
	const glm::mat4 &matrix() const;
};
