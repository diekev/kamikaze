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

class SmokeSimulation {
	float m_dt;  /* time step */
	float m_dh;  /* voxel size, sometimes refered to as dx or dTau */
	int m_advection_scheme;
	float m_max_vel;
	openvdb::math::Transform m_xform;

	std::string m_cache_path;

	/* Simulation fields. */
	openvdb::VectorGrid::Ptr velocity;
	openvdb::ScalarGrid::Ptr density;
	openvdb::ScalarGrid::Ptr temperature;
	openvdb::ScalarGrid::Ptr pressure;
	openvdb::BoolGrid::Ptr obstacle;
	openvdb::Int32Grid::Ptr flags;

	/* Compute time step with respect to the CFL condition. */
	void CFL();

	/* Move fields though the velocity field. */
	void advectSemiLagrange();

public:
	SmokeSimulation();
	~SmokeSimulation() = default;

	void addInflow(const openvdb::ScalarGrid::Ptr &inflow);

	/* Write simulation fields to disk. */
	void write(const int frame) const;

	/* Perform one simulation step. */
	void step();

	void timeStep(const float value) { m_dt = value; }
	void advectionScheme(const int scheme) { m_advection_scheme = scheme; }
	void cachePath(const std::string &path) { m_cache_path = path; }
};
