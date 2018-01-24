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
 * The Original Code is Copyright (C) 2017 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#pragma once

template <class S, class T>
inline auto interp_lineaire(const S &v0, const S &v1, T f)
{
	return (static_cast<T>(1) - f) * v0 + f * v1;
}

template <class S, class T>
inline auto interp_bilineaire(
		const S &v00, const S &v10,
		const S &v01, const S &v11,
		T fx, T fy)
{
	return interp_lineaire(
				interp_lineaire(v00, v10, fx),
				interp_lineaire(v01, v11, fx),
				fy);
}

template <class S, class T>
inline auto interp_trilineaire(
		const S &v000, const S &v100,
		const S &v010, const S &v110,
		const S &v001, const S &v101,
		const S &v011, const S &v111,
		T fx, T fy, T fz)
{
	return interp_lineaire(
				interp_bilineaire(v000, v100, v010, v110, fx, fy),
				interp_bilineaire(v001, v101, v011, v111, fx, fy),
				fz);
}

template <class S, class T>
inline auto interp_quadrilineaire(
		const S &v0000, const S &v1000,
		const S &v0100, const S &v1100,
		const S &v0010, const S &v1010,
		const S &v0110, const S &v1110,
		const S &v0001, const S &v1001,
		const S &v0101, const S &v1101,
		const S &v0011, const S &v1011,
		const S &v0111, const S &v1111,
		T fx, T fy, T fz, T ft)
{
	return interp_lineaire(
				interp_trilineaire(v0000, v1000, v0100, v1100, v0010, v1010, v0110, v1110, fx, fy, fz),
				interp_trilineaire(v0001, v1001, v0101, v1101, v0011, v1011, v0111, v1111, fx, fy, fz),
				ft);
}
