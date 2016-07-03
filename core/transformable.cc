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

#include "transformable.h"

#include <glm/gtc/matrix_transform.hpp>

void Transformable::recompute_matrix()
{
	m_matrix = glm::mat4(1.0f);
	m_matrix = glm::translate(m_matrix, m_pos);
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
	m_matrix = glm::scale(m_matrix, m_scale);

	m_inv_matrix = glm::inverse(m_matrix);
}

void Transformable::pos(const glm::vec3 &p)
{
	m_pos = p;
	recompute_matrix();
}

const glm::vec3 &Transformable::pos() const
{
	return m_pos;
}

void Transformable::rot(const glm::vec3 &r)
{
	m_rot = r;
	recompute_matrix();
}

const glm::vec3 &Transformable::rot() const
{
	return m_rot;
}

void Transformable::scale(const glm::vec3 &s)
{
	m_scale = s;
	recompute_matrix();
}

const glm::vec3 &Transformable::scale() const
{
	return m_scale;
}

void Transformable::matrix(const glm::mat4 &m)
{
	m_matrix = m;
	m_inv_matrix = glm::inverse(m_matrix);
}

const glm::mat4 &Transformable::matrix() const
{
	return m_matrix;
}

void Transformable::inverse_matrix(const glm::mat4 &m)
{
	m_inv_matrix = m;
	m_matrix = glm::inverse(m_inv_matrix);
}

const glm::mat4 &Transformable::inverse_matrix() const
{
	return m_inv_matrix;
}
