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

#include "primitive.h"

class Mesh : public Primitive {
	std::vector<glm::ivec4> m_quads{};
	std::vector<glm::ivec3> m_tris{};
	std::vector<glm::vec3> m_verts{};
	std::vector<glm::vec2> m_uvs{};
	std::vector<glm::vec3> m_normals{};

	ego::BufferObject::Ptr m_buffer_data;
	ego::Program m_program;
	size_t m_elements;

public:
	Mesh();

	std::vector<glm::ivec4> &quads();
	const std::vector<glm::ivec4> &quads() const;

	std::vector<glm::ivec3> &tris();
	const std::vector<glm::ivec3> &tris() const;

	std::vector<glm::vec3> &verts();
	const std::vector<glm::vec3> &verts() const;

	std::vector<glm::vec3> &normals();
	const std::vector<glm::vec3> &normals() const;

	void update() override;

	void render(ViewerContext *context, const bool for_outline) override;

	void setCustomUIParams(ParamCallback *cb) override;

	/* TODO */
	void generateGPUData();

	static void registerSelf(PrimitiveFactory *factory);

	Primitive *copy() const override;

private:
	void computeBBox();
	void computeNormals();
	void loadShader();
};
