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

#include <vector>

#include "object.h"

static constexpr auto INVALID_FRAME = std::numeric_limits<int>::max();

struct SimulationContext {
	glm::vec3 gravity;
	float time_step;
};

/* ************************************************************************** */

class Solver : public Persona {
public:
	virtual ~Solver() = default;

	virtual const char *name() const = 0;
	virtual void solve_for_object(const SimulationContext &context, Object *object) = 0;
};

class FreeFallSolver : public Solver {
public:
	const char *name() const override;
	void solve_for_object(const SimulationContext &context, Object *object) override;
};

class SimpleParticleSolver : public Solver {
public:
	const char *name() const override;
	void solve_for_object(const SimulationContext &context, Object *object) override;
};

/* ************************************************************************** */

class Simulation : public SceneNode {
	std::unordered_map<Object *, PrimitiveCollection *> m_states;
	int m_start_frame = INVALID_FRAME;
	int m_last_frame = INVALID_FRAME;

	Solver *m_solver;
	SimulationContext m_simcontext;

public:
	/* Prevent from having simulation with no solver. */
	Simulation() = delete;

	Simulation(Solver *solver);
	~Simulation();

	int type() const override
	{
		return SCE_NODE_SIMULATION;
	}

	void step(const EvaluationContext * const context);

	void sync_states();
	bool update_properties() override;

private:
	float eval_gravity();
};
