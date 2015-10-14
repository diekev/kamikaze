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

#include <openvdb/tools/Morphology.h>

#include "forces.h"
#include "util/utils.h"

Vec3s accumulate_forces(const std::vector<Vec3s> &forces)
{
	Vec3s force(0.0f);

	for (const auto &f : forces) {
		force += f;
	}

	return force;
}

void add_buoyancy(const float dt,
                  VectorGrid::Ptr &velocity,
                  ScalarGrid::Ptr &density,
                  ScalarGrid::Ptr &temperature,
                  const openvdb::Int32Grid::Ptr &flags)
{
	Timer(__func__);

	using namespace openvdb;
	using namespace openvdb::math;

	typedef FloatTree::LeafNodeType LeafType;

	tools::dilateVoxels(temperature->tree(), 1, tools::NN_FACE);

	VectorAccessor vacc = velocity->getAccessor();
	ScalarAccessor dacc = density->getAccessor();
	ScalarAccessor tacc = temperature->getAccessor();
	Int32Grid::ConstAccessor facc = flags->getConstAccessor();

	// TODO:
	const float alpha = 0.001f;
	const float beta = 0.1f;
	const float tmp_amb = 26.85f;  // 300°K

	for (FloatTree::LeafIter lit = temperature->tree().beginLeaf(); lit; ++lit) {
		LeafType &leaf = *lit;

		for (typename LeafType::ValueOnIter it = leaf.beginValueOn(); it; ++it) {
			const Coord co(it.getCoord());
			const Coord co_y(co.offsetBy(0, -1, 0));

			if (facc.getValue(co_y) == FLUID) {
				Vec3s vel = vacc.getValue(co);
				const float temp = (tacc.getValue(co) + tacc.getValue(co_y)) * 0.5f;
				const float dens = (dacc.getValue(co) + dacc.getValue(co_y)) * 0.5f;

				vel.y() += (alpha * dens + beta * (tmp_amb - temp)) * dt;
				vacc.setValue(co, vel);
			}
		}
	}
}

void set_neumann_boundary(VectorGrid &velocity, const openvdb::Int32Grid::Ptr &flags)
{
	Timer(__func__);

	using namespace openvdb;
	using namespace openvdb::math;

	typedef Vec3STree::LeafNodeType LeafType;

	Int32Grid::ConstAccessor facc = flags->getConstAccessor();

	// TODO: reverse this operation: iterate over the collision field? What if
	// there is noise?
	for (Vec3STree::LeafIter lit = velocity.tree().beginLeaf(); lit; ++lit) {
		LeafType &leaf = *lit;

		for (typename LeafType::ValueOnIter it = leaf.beginValueOn(); it; ++it) {
			const Coord co(it.getCoord());

			if ((facc.getValue(co) == SOLID)) {
				it.setValue(Vec3s(0.0f));
				continue;
			}

			Vec3s vel = it.getValue();

			const Coord neighbours[6] = {
				Coord(co.offsetBy(-1, 0, 0)),
			    Coord(co.offsetBy(0, -1, 0)),
			    Coord(co.offsetBy(0, 0, -1)),
			    Coord(co.offsetBy(+1, 0, 0)),
			    Coord(co.offsetBy(0, +1, 0)),
			    Coord(co.offsetBy(0, 0, +1)),
			};

			for (int i = 0; i < 3; ++i) {
				if (facc.getValue(neighbours[i]) == SOLID ||
				    facc.getValue(neighbours[i + 3]) == SOLID)
				{
					vel[i] = 0.0f;
				}
			}

			it.setValue(vel);
		}
	}
}

openvdb::Int32Grid::Ptr build_flag_grid(const ScalarGrid::Ptr &inflow, const openvdb::BoolGrid::Ptr &collision)
{
	using namespace openvdb;

	typedef BoolTree::LeafNodeType BoolLeafType;
	typedef FloatTree::LeafNodeType FloatLeafType;

	Int32Grid::Ptr flag_grid = openvdb::Int32Grid::create(FLUID);
	flag_grid->setTransform(inflow->transformPtr());
	flag_grid->setName("flags");

	Int32Grid::Accessor facc = flag_grid->getAccessor();

	for (BoolTree::LeafIter lit = collision->tree().beginLeaf(); lit; ++lit) {
		BoolLeafType &leaf = *lit;

		for (typename BoolLeafType::ValueOnIter it = leaf.beginValueOn(); it; ++it) {
			facc.setValue(it.getCoord(), SOLID);
		}
	}

	for (FloatTree::LeafIter lit = inflow->tree().beginLeaf(); lit; ++lit) {
		FloatLeafType &leaf = *lit;

		for (typename FloatLeafType::ValueOnIter it = leaf.beginValueOn(); it; ++it) {
			facc.setValue(it.getCoord(), FLUID);
		}
	}

	return flag_grid;
}
