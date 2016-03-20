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

#include "util_openvdb.h"

openvdb::FloatGrid::Ptr transform_grid(const openvdb::FloatGrid &grid,
                                       const openvdb::Vec3s &rot,
                                       const openvdb::Vec3s &scale,
                                       const openvdb::Vec3s &translate,
                                       const openvdb::Vec3s &pivot)
{
	/* make sure the new grid has the same transform and metadatas as the old. */
	openvdb::FloatGrid::Ptr xformed = grid.copy(openvdb::CopyPolicy::CP_NEW);

	openvdb::Mat4R mat(openvdb::Mat4R::identity());
	mat.preTranslate(pivot);
	mat.preRotate(openvdb::math::X_AXIS, rot[0]);
	mat.preRotate(openvdb::math::Y_AXIS, rot[1]);
	mat.preRotate(openvdb::math::Z_AXIS, rot[2]);
	mat.preScale(scale);
	mat.preTranslate(-pivot);
	mat.preTranslate(translate);

	openvdb::tools::GridTransformer transformer(mat);
	transformer.transformGrid<openvdb::tools::PointSampler>(grid, *xformed);
	openvdb::tools::prune(xformed->tree());

	return xformed;
}

int axis_dominant_v3_single(const float vec[3])
{
	const float x = std::abs(vec[0]);
	const float y = std::abs(vec[1]);
	const float z = std::abs(vec[2]);

	return ((x > y) ? ((x > z) ? 0 : 2) : ((y > z) ? 1 : 2));
}

openvdb::math::Vec3d convertGLMVec(const glm::vec3 &vec)
{
	return openvdb::math::Vec3d(vec[0], vec[1], vec[2]);
}

