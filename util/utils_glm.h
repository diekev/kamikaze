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
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

/**
 * Vector output.
 */

template <typename T, glm::precision P>
std::ostream &operator<<(std::ostream &os, const glm::detail::tvec3<T, P> &vec)
{
	os << '[' << vec[0] << ',' << vec[1] << ',' << vec[2] << ']';
	return os;
}

/**
 * Matrix output.
 */

template <typename T, glm::precision P>
std::ostream &operator<<(std::ostream &os, const glm::detail::tmat4x4<T, P> &mat)
{
	for (int i = 0; i < 4; ++i) {
		os << '[' << mat[i][0] << ',' << mat[i][1] << ',' << mat[i][2] << ',' << mat[i][3] << ']' << '\n';
	}
	return os;
}

/**
 * Matrix pre transformation.
 */

template <typename T, glm::precision P>
glm::detail::tmat4x4<T, P> pre_translate(const glm::detail::tmat4x4<T, P> &mat,
                                         const glm::detail::tvec3<T, P> &v)
{
	auto tmat = glm::translate(glm::detail::tmat4x4<T, P>(1), v);
	return tmat * mat;
}

template <typename T, glm::precision P>
glm::detail::tmat4x4<T, P> pre_scale(const glm::detail::tmat4x4<T, P> &mat,
                                     const glm::detail::tvec3<T, P> &s)
{
	auto smat = glm::scale(glm::detail::tmat4x4<T, P>(1), s);
	return smat * mat;
}

template <typename T, glm::precision P>
glm::detail::tmat4x4<T, P> pre_rotate(const glm::detail::tmat4x4<T, P> &mat,
                                      const T &angle,
                                      const glm::detail::tvec3<T, P> &axis)
{
	auto rmat = glm::rotate(glm::detail::tmat4x4<T, P>(1), angle, axis);
	return rmat * mat;
}

/**
 * Matrix post transformation.
 */

template <typename T, glm::precision P>
glm::detail::tmat4x4<T, P> post_translate(const glm::detail::tmat4x4<T, P> &mat,
                                          const glm::detail::tvec3<T, P> &v)
{
	auto tmat = glm::translate(glm::detail::tmat4x4<T, P>(1), v);
	return mat * tmat;
}

template <typename T, glm::precision P>
glm::detail::tmat4x4<T, P> post_scale(const glm::detail::tmat4x4<T, P> &mat,
                                      const glm::detail::tvec3<T, P> &s)
{
	auto smat = glm::scale(glm::detail::tmat4x4<T, P>(1), s);
	return mat * smat;
}

template <typename T, glm::precision P>
glm::detail::tmat4x4<T, P> post_rotate(const glm::detail::tmat4x4<T, P> &mat,
                                       const T &angle,
                                       const glm::detail::tvec3<T, P> &axis)
{
	auto rmat = glm::rotate(glm::detail::tmat4x4<T, P>(1), angle, axis);
	return mat * rmat;
}
