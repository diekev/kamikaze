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

#include <glm/glm.hpp>

#include <openvdb/openvdb.h>
#include <openvdb/tools/RayIntersector.h>

#include "volumebase.h"

class Brush;

class LevelSet : public VolumeBase {
	typedef openvdb::math::Ray<double> ray_t;
	typedef openvdb::tools::LevelSetRayIntersector<openvdb::FloatGrid> isector_t;
	std::unique_ptr<isector_t> m_isector;

	openvdb::FloatGrid::Ptr m_level_set; // For sculpting

	void generateMesh(const bool is_sculpt_mode);
	void loadShader();

public:
	LevelSet();
	explicit LevelSet(openvdb::GridBase::Ptr grid);
	~LevelSet() = default;

	void setGrid(openvdb::GridBase::Ptr grid);

	void render(ViewerContext *context, const bool for_outline) override;
	void setCustomUIParams(ParamCallback *cb) override;

	bool intersectLS(const Ray &ray, Brush *brush);

	void swapGrids(const bool is_scuplt_mode);

	static void registerSelf(ObjectFactory *factory);
};
