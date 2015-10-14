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

#include "smokesimulation.h"

#include <openvdb/tools/VolumeAdvect.h>

#include "advection.h"
#include "forces.h"
#include "types.h"
#include "util_smoke.h"

#include "util/utils.h"

SmokeSimulation::SmokeSimulation()
    : m_dt(0.1f)
    , m_dh(0.1f)
    , m_advection_scheme(ADVECT_RK3)
    , m_max_vel(0.0f)
{
	using namespace openvdb;

	m_xform = *math::Transform::createLinearTransform(m_dh);

	initialize_field<ScalarGrid>(density, "density", m_xform);
	initialize_field<openvdb::BoolGrid>(obstacle, "obstacle", m_xform);
	initialize_field<ScalarGrid>(pressure, "pressure", m_xform);
	initialize_field<ScalarGrid>(temperature, "temperature", m_xform);
	initialize_field<VectorGrid>(velocity, "velocity", m_xform,
	                             VEC_CONTRAVARIANT_RELATIVE, GRID_STAGGERED);
}

/* Per Bridson, "Fluid Simulation Course Notes", SIGGRAPH 2007:
 * Limit dt to be no greater than 5 grid cells (the largest distance a particle
 * will be able to move during semi-Lagrangian back tracing). Also include forces
 * into the computation to account for velocities induced by them.
 */
void SmokeSimulation::CFL()
{
	const float factor = 1.0f; // user variable
	const float dh = 5.0f * m_dh;
	const float max_vel = std::max(1e-16f, std::max(dh, m_max_vel));

	std::cout << "----------max vel = " << m_max_vel << "\n";
	std::cout << "----------CFL dt = " << dh / std::sqrt(max_vel) << "\n";

	/* dt <= (5dh / max_u) */
	m_dt = std::min(m_dt, factor * dh / std::sqrt(max_vel));
}

void SmokeSimulation::write(const int frame) const
{
	using namespace openvdb;

	std::stringstream ss;
	ss << frame;
	std::string num(ss.str());
	num.insert(num.begin(), 1 - (num.size() - 1), '0');

	std::string path("/home/kevin/src/test_files/poseidon/");

	io::File file(path + "test_" + num + ".vdb");
	GridPtrVec grids;

	grids.push_back(density->deepCopy());
	grids.push_back(velocity->deepCopy());
	grids.push_back(temperature->deepCopy());
	grids.push_back(flags->deepCopy());
	grids.push_back(pressure->deepCopy());
	grids.push_back(obstacle->deepCopy());

	file.setCompression(io::COMPRESS_BLOSC);

	file.write(grids);
}

void SmokeSimulation::step()
{
	float t(0.0f), t_frame(1.0f / 24.0f);

	velocity->topologyUnion(*density);
	flags = build_flag_grid(density, obstacle);

	while (t < t_frame) {
		advectSemiLagrange();

		set_neumann_boundary(*velocity, flags);
		add_buoyancy(m_dt, velocity, density, temperature, flags);

		solve_pressure(m_dt, *velocity, *pressure, *flags);
		set_neumann_boundary(*velocity, flags);

		std::cout << "----------dt = " << m_dt << "\n";
		t = t + m_dt;
	}
}

void SmokeSimulation::advectSemiLagrange()
{
	Timer(__func__);

	typedef openvdb::tools::Sampler<1, false> Sampler;

	openvdb::tools::VolumeAdvection<VectorGrid, false> advector(*velocity);
	advector.setIntegrator(advection_scheme(m_advection_scheme));

	m_max_vel = advector.getMaxVelocity();
	const int n = advector.getMaxDistance(*density, m_dt);

	if (n > 20) {
		OPENVDB_THROW(openvdb::RuntimeError, "Advection distance is too high!");
	}

	ScalarGrid::Ptr sresult;

	sresult = advector.advect<ScalarGrid, Sampler>(*density, m_dt);
	density.swap(sresult);

	sresult = advector.advect<ScalarGrid, Sampler>(*temperature, m_dt);
	temperature.swap(sresult);

	VectorGrid::Ptr result;
	result = advector.advect<VectorGrid, Sampler>(*velocity, m_dt);
	velocity.swap(result);
}

void SmokeSimulation::addInflow(const openvdb::FloatGrid::Ptr &inflow)
{
	Timer(__func__);

	using namespace openvdb;
	using namespace openvdb::math;

	typedef FloatTree::LeafNodeType LeafType;

	ScalarAccessor dacc = density->getAccessor();

	for (FloatTree::LeafIter lit = inflow->tree().beginLeaf(); lit; ++lit) {
		LeafType &leaf = *lit;

		for (typename LeafType::ValueOnIter it = leaf.beginValueOn(); it; ++it) {
			dacc.setValue(it.getCoord(), 1.0f);
		}
	}
}
