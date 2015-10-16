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
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#pragma once

#include <GL/glew.h>
#include <memory>
#include <string>
#include <unordered_map>

enum {
	VERTEX_SHADER = 0,
	FRAGMENT_SHADER = 1,
	GEOMETRY_SHADER = 2,
};

class GPUProgram {
	GLuint m_program;
	GLuint m_shaders[3];
	std::unordered_map<std::string, GLuint> m_attributes;
	std::unordered_map<std::string, GLuint> m_uniforms;

public:
	GPUProgram();
	~GPUProgram();

	using UPtr = std::unique_ptr<GPUProgram>;

	static UPtr create();

	void loadFromString(int shader_type, const std::string &source);
	void loadFromFile(int shader_type, const std::string &source);
	void createAndLinkProgram();
	void enable() const;
	void disable() const;
	void addAttribute(const std::string &attribute);
	void addUniform(const std::string &uniform);
	bool isValid() const;

	GLuint operator[](const std::string &attribute);
	GLuint operator()(const std::string &uniform);
};
