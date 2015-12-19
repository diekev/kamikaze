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

#include <cstdio>
#include <string>

#include <glm/glm.hpp>

#include <openvdb/openvdb.h>

/* return current time */
double time_dt();

/* A utility class to print the time elapsed during its lifetime, usefull for e.g.
 * timing the overall execution time of a function */
class ScopeTimer {
	double m_start;
	std::string m_message;

public:
	explicit ScopeTimer(const std::string &message)
	    : m_start(time_dt())
	    , m_message(message)
	{}

	~ScopeTimer()
	{
		printf("%s: %fs\n", m_message.c_str(), time_dt() - m_start);
	}
};

#define Timer(x) \
	ScopeTimer func(x);

int axis_dominant_v3_single(const float vec[3]);

/* Functions to convert between glm and openvdb types. */

template <typename T>
glm::vec3 convertOpenVDBVec(const openvdb::math::Vec3<T> &vec)
{
	return glm::vec3(vec[0], vec[1], vec[2]);
}

openvdb::math::Vec3d convertGLMVec(const glm::vec3 &vec);

template <typename T>
glm::mat4 convertOpenVDBMat4(const openvdb::math::Mat4<T> &mat)
{
	glm::mat4 ret;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			ret[i][j] = mat[i][j];
		}
	}

	return ret;
}

template <typename T>
void print_mat4(const openvdb::math::Mat4<T> &mat, const std::string &title = "")
{
	if (!title.empty()) {
		printf("%s:\n", title.c_str());
	}

	for (int i = 0; i < 4; ++i) {
		printf("[%.6f, %.6f, %.6f, %.6f]\n", mat[i][0], mat[i][1], mat[i][2], mat[i][3]);
	}

	printf("\n");
}
