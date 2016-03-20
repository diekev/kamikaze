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

#include "sculpt.h"

#include <openvdb/math/Stencils.h>

#include "sculpt/brush.h"

/* Modifiy level set by adding or subtracting values based on the brush's strength */
void do_sculpt_draw(openvdb::FloatGrid &level_set, Brush *brush,
                    openvdb::math::Coord &ijk, const float voxel_size)
{
	openvdb::FloatGrid::Accessor accessor = level_set.getAccessor();
	const float radius = brush->radius();
	const float strength = brush->strength() * voxel_size;
	float influence;

	openvdb::math::Coord co(ijk);
	int &x = ijk[0], &y = ijk[1], &z = ijk[2];

	for (x = co[0] - radius; x < co[0] + radius; ++x) {
		for (y = co[1] - radius; y < co[1] + radius; ++y) {
			for (z = co[2] - radius; z < co[2] + radius; ++z) {
				influence = brush->influence(co, ijk);

				if (influence > 0.0f) {
					//accessor.modifyValue(ijk, addOp);
					float value = accessor.getValue(ijk);
					accessor.setValue(ijk, value - strength * influence);
				}
			}
		}
	}
}

/* Perform laplacian smoothing of the level set */
void do_sculpt_smooth(openvdb::FloatGrid &level_set, Brush *brush,
                      openvdb::math::Coord &ijk, const float voxel_size)
{
	using namespace openvdb;

	FloatGrid::Accessor accessor = level_set.getAccessor();
    math::GradStencil<FloatGrid> stencil(level_set, voxel_size);
	const float dt = math::Pow2(voxel_size) / 6.0f;

	const int radius = brush->radius();
	const int diameter = radius + radius;
	float *buffer = new float[diameter * diameter * diameter];

	const float strength = brush->strength() * voxel_size;
	float influence;

	int index = 0;
	math::Coord co(ijk);
	int &x = ijk[0], &y = ijk[1], &z = ijk[2];

	for (x = co[0] - radius; x < co[0] + radius; ++x) {
		for (y = co[1] - radius; y < co[1] + radius; ++y) {
			for (z = co[2] - radius; z < co[2] + radius; ++z, ++index) {
				float value = accessor.getValue(ijk);
				influence = brush->influence(co, ijk);

				if (influence > 0.0f) {
					stencil.moveTo(ijk);
					accessor.setValue(ijk, value - strength * influence);
					const float phi = value + dt * stencil.laplacian();
                    buffer[index] = phi - strength * influence;
				}
				else {
					buffer[index] = value;
				}
			}
		}
	}

	index = 0;
	/* copy buffer back to voxels */
	for (x = co[0] - radius; x < co[0] + radius; ++x) {
		for (y = co[1] - radius; y < co[1] + radius; ++y) {
			for (z = co[2] - radius; z < co[2] + radius; ++z, ++index) {
				accessor.setValue(ijk, buffer[index]);
			}
		}
	}

	delete [] buffer;
}
