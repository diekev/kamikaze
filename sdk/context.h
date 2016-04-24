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

class NodeFactory;
class PrimitiveFactory;
class Scene;

struct EvaluationContext {
	Scene *scene;
	PrimitiveFactory *object_factory;
	NodeFactory *node_factory;
};

class ViewerContext {
	glm::mat4 m_model_view;
	glm::mat4 m_projection;
	glm::mat4 m_modelviewprojection;
	glm::vec3 m_view;
	glm::mat3 m_normal;

public:
	glm::mat4 modelview() const;
	void setModelview(const glm::mat4 &modelview);

	glm::mat4 projection() const;
	void setProjection(const glm::mat4 &projection);

	glm::vec3 view() const;
	void setView(const glm::vec3 &view);

	glm::mat3 normal() const;
	void setNormal(const glm::mat3 &normal);

	glm::mat4 MVP() const;
	void setMVP(const glm::mat4 &MVP);
};
