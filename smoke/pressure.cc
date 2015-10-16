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

#include <openvdb/math/FiniteDifference.h>
#include <openvdb/math/Stencils.h>
#include <openvdb/tools/GridOperators.h>
#include <openvdb/tools/PoissonSolver.h>

#include "forces.h"
#include "util/utils.h"

void correct_velocity(const float dt,
                      VectorGrid &velocity,
                      ScalarGrid &pressure,
                      const openvdb::Int32Grid &flags)
{
	using namespace openvdb;
	using namespace openvdb::math;

	typedef FloatTree::LeafNodeType LeafType;

	Int32Grid::ConstAccessor facc = flags.getAccessor();
	ConstScalarAccessor pacc = pressure.getConstAccessor();
	VectorAccessor vacc = velocity.getAccessor();

	for (FloatTree::LeafIter lit = pressure.tree().beginLeaf(); lit; ++lit) {
		LeafType &leaf = *lit;

		for (typename LeafType::ValueOnIter it = leaf.beginValueOn(); it; ++it) {
			const Coord &co = it.getCoord();
			Vec3s grad(0.0f);

			const Coord neighbours[3] = {
				Coord(co.offsetBy(-1, 0, 0)),
			    Coord(co.offsetBy(0, -1, 0)),
			    Coord(co.offsetBy(0, 0, -1)),
			};

			const int flag = facc.getValue(co);

			if (flag == FLUID) {
				for (int i = 0; i < 3; i++) {
					const int nflag = facc.getValue(neighbours[i]);

					if (nflag == FLUID) {
						grad[i] -= (pacc.getValue(co) - pacc.getValue(neighbours[i]));
					}
					else if (nflag == EMPTY) {
						grad[i] -= (pacc.getValue(co));
					}
				}
			}
			else if (flag == EMPTY) {
				for (int i = 0; i < 3; i++) {
					if (facc.getValue(neighbours[i]) == FLUID) {
						grad[i] += pacc.getValue(neighbours[i]);
					}
					else {
						grad[i] = 0.0f;
					}
				}
			}

			Vec3s vel = vacc.getValue(co);
			vel += grad * dt;
			vacc.setValue(co, vel);
		}
	}
}

#if 1
void solve_pressure(const float dt,
                    VectorGrid &velocity,
                    ScalarGrid &pressure,
                    const openvdb::Int32Grid &flags)
{
	Timer(__func__);

	using namespace openvdb;
	using namespace openvdb::math;
	using namespace openvdb::tools::poisson;

	typedef pcg::SparseStencilMatrix<float, 7> MatrixType;

	typedef FloatTree::LeafNodeType LeafType;
	Int32Grid::ConstAccessor facc = flags.getAccessor();

	// find divergence
	ScalarGrid::Ptr div = tools::divergence(velocity);

	// form poisson
	Int32Tree::ConstPtr index_tree = createIndexTree(div->tree());
	pcg::VectorS::Ptr b = createVectorFromTree<float>(div->tree(), *index_tree);

	const pcg::SizeType rows = b->size();
	MatrixType A(rows);

	for (FloatTree::LeafIter lit = div->tree().beginLeaf(); lit; ++lit) {
		LeafType &leaf = *lit;

		for (typename LeafType::ValueOnIter it = leaf.beginValueOn(); it; ++it) {
			Coord co = it.getCoord();
			VIndex row = index_tree->getValue(co);

			const Coord neighbours[6] = {
				Coord(co.offsetBy(-1, 0, 0)),
			    Coord(co.offsetBy(+1, 0, 0)),
			    Coord(co.offsetBy(0, -1, 0)),
			    Coord(co.offsetBy(0, +1, 0)),
			    Coord(co.offsetBy(0, 0, -1)),
			    Coord(co.offsetBy(0, 0, +1)),
			};

			float diag = 0.0f;
			float bg = 0.0f;

			for (int i(0); i < 6; ++i) {
				const auto &neighbour = neighbours[i];
				const int flag = facc.getValue(neighbour);

				if (flag != SOLID) {
					diag -= 1.0f;

					if (flag == FLUID) {
						VIndex column = index_tree->getValue(neighbour);

						if (column != -1) {
							A.setValue(row, column, 1.0f);
						}
					}
					else {
						bg -= 1.0f;
					}
				}
			}

			if (diag == 0.0f) {
				diag = 1.0f;
			}

			A.setValue(row, row, diag);
			(*b)[row] += bg;
		}
	}

	b->scale(dt);

	// form preconditionner
	pcg::JacobiPreconditioner<MatrixType> precond(A);

	// solve pressure
	MatrixType::VectorType x(rows, 0.0f);

	pcg::State terminator = pcg::terminationDefaults<float>();
	terminator.iterations = 200;
	terminator.relativeError = 1e-6f;

	util::NullInterrupter interrupter;
	pcg::State result = pcg::solve(A, *b, x, precond, interrupter, terminator);

	std::cout << "-------- solver iterations: " << result.iterations << "\n";

	if (result.success) {
		pressure.setTree(tools::poisson::createTreeFromVector(x, *index_tree, 0.0f));
	}
	else {
		pressure.clear();
		return;
	}

	// subtract pressure gradient
	correct_velocity(dt, velocity, pressure, flags);
}
#else
void solve_pressure(const SimulationGlobals &sg,
                    VectorGrid &velocity,
                    ScalarGrid &pressure,
                    const openvdb::Int32Grid &flags)
{
	Timer(__func__);

	using namespace openvdb;
	using namespace openvdb::math;

	typedef FloatTree::LeafNodeType LeafType;
	typedef Vec3STree::LeafNodeType VLeafType;
	ScalarAccessor pacc = pressure.getAccessor();

	// find divergence
	ScalarGrid::Ptr div = tools::divergence(velocity);
//	ScalarGrid::Ptr div = ScalarGrid::create(0.0f);
//	div->setTransform(velocity.transformPtr());
//	ScalarAccessor dacc = div->getAccessor();
//	VectorAccessor vacc = velocity.getAccessor();

//	for (Vec3STree::LeafIter lit = velocity.tree().beginLeaf(); lit; ++lit) {
//		VLeafType &leaf = *lit;

//		for (typename VLeafType::ValueOnIter it = leaf.beginValueOn(); it; ++it) {
//			Coord co = it.getCoord();

//			const Coord n[6] = {
//			    Coord(co.offsetBy(-1, 0, 0)),
//			    Coord(co.offsetBy(+1, 0, 0)),
//			    Coord(co.offsetBy(0, -1, 0)),
//			    Coord(co.offsetBy(0, +1, 0)),
//			    Coord(co.offsetBy(0, 0, -1)),
//			    Coord(co.offsetBy(0, 0, +1)),
//			};

//			float d = 1.0f * ((vacc.getValue(co).x() - vacc.getValue(n[1]).x())
//			                + (vacc.getValue(co).y() - vacc.getValue(n[3]).y())
//			                + (vacc.getValue(co).z() - vacc.getValue(n[5]).z()));

//			dacc.setValue(co, d);
//		}
//	}

	// linear solver using Gauss Seidel relaxation

	for (int i = 0; i < 20; ++i) {
		for (FloatTree::LeafIter lit = div->tree().beginLeaf(); lit; ++lit) {
			LeafType &leaf = *lit;

			for (typename LeafType::ValueOnIter it = leaf.beginValueOn(); it; ++it) {
				Coord co = it.getCoord();
				const auto val = it.getValue();

				const Coord n[6] = {
				    Coord(co.offsetBy(-1, 0, 0)),
				    Coord(co.offsetBy(+1, 0, 0)),
				    Coord(co.offsetBy(0, -1, 0)),
				    Coord(co.offsetBy(0, +1, 0)),
				    Coord(co.offsetBy(0, 0, -1)),
				    Coord(co.offsetBy(0, 0, +1)),
				};

				float p = val * sg.dt + (pacc.getValue(n[0]) + pacc.getValue(n[1])
				        + pacc.getValue(n[2]) + pacc.getValue(n[3])
				        + pacc.getValue(n[4]) + pacc.getValue(n[5])) / 6;

				pacc.setValue(co, p);
			}
		}
	}

	// subtract pressure gradient
	correct_velocity(sg, velocity, pressure, flags);
}
#endif
