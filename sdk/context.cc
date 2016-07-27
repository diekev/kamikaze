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

#include "context.h"

const glm::mat4 &ViewerContext::projection() const
{
	return m_projection;
}

void ViewerContext::setProjection(const glm::mat4 &projection)
{
	m_projection = projection;
}

const glm::vec3 &ViewerContext::view() const
{
	return m_view;
}

void ViewerContext::setView(const glm::vec3 &view)
{
	m_view = view;
}

const glm::mat3 &ViewerContext::normal() const
{
	return m_normal;
}

void ViewerContext::setNormal(const glm::mat3 &normal)
{
	m_normal = normal;
}

const glm::mat4 &ViewerContext::MVP() const
{
	return m_modelviewprojection;
}

void ViewerContext::setMVP(const glm::mat4 &MVP)
{
	m_modelviewprojection = MVP;
}

const glm::mat4 &ViewerContext::matrix() const
{
	return m_matrix;
}

void ViewerContext::setMatrix(const glm::mat4 &matrix)
{
	m_matrix = matrix;
}

const glm::mat4 &ViewerContext::modelview() const
{
	return m_model_view;
}

void ViewerContext::setModelview(const glm::mat4 &model_view)
{
	m_model_view = model_view;
}
