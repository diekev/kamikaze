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

#include <openvdb/openvdb.h>

using openvdb::math::Coord;

enum {
	BRUSH_MODE_ADD = 0,
	BRUSH_MODE_SUB = 1,
};

enum {
	BRUSH_TOOL_DRAW = 0,
	BRUSH_TOOL_SMOOTH = 1,
};

class Brush {
	float m_radius, m_inv_radius;
	float m_strength;
	int m_mode;
	int m_tool;

public:
	Brush();
	Brush(const float radius, const float strength);
	~Brush() = default;

	inline float influence(const Coord &center, const Coord &pos)
	{
		return 1.0f - (center - pos).asVec3d().length() * m_inv_radius + 0.001f;
	}

	void radius(const float rad);
	float radius() const;

	void strength(const float s);
	float strength() const;

	void mode(const int mode);

	int tool() const;
	void tool(const int tool);
};
