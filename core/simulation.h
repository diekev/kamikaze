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
	glm::vec3 gravity = glm::vec3(0.0f, 0.0f, 0.0f);
	float time_step = 0.0f;
};

struct ObjectState {
	PrimitiveCollection *collection = nullptr;
	glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(0.0f, 0.0f, 0.0f);
};

/* ************************************************************************** */

enum class solver_flag : char {
	transform = (1 << 0),
	data      = (1 << 1),
};

constexpr solver_flag operator&(solver_flag lhs, solver_flag rhs)
{
	return static_cast<solver_flag>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

constexpr solver_flag operator|(solver_flag lhs, solver_flag rhs)
{
	return static_cast<solver_flag>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

constexpr solver_flag operator^(solver_flag lhs, solver_flag rhs)
{
	return static_cast<solver_flag>(static_cast<int>(lhs) ^ static_cast<int>(rhs));
}

constexpr solver_flag operator~(solver_flag lhs)
{
	return static_cast<solver_flag>(~static_cast<int>(lhs));
}

solver_flag &operator|=(solver_flag &lhs, solver_flag rhs);
solver_flag &operator&=(solver_flag &lhs, solver_flag rhs);
solver_flag &operator^=(solver_flag &lhs, solver_flag rhs);

constexpr bool operator==(solver_flag lhs, int rhs)
{
	return static_cast<int>(lhs) == rhs;
}

constexpr bool operator!=(solver_flag lhs, int rhs)
{
	return !(lhs == rhs);
}

constexpr bool operator==(int lhs, solver_flag rhs)
{
	return lhs == static_cast<int>(rhs);
}

constexpr bool operator!=(int lhs, solver_flag rhs)
{
	return !(lhs == rhs);
}

/* ************************************************************************** */

class Solver : public Persona {
	solver_flag m_flags = static_cast<solver_flag>(0);

public:
	virtual ~Solver() = default;

	solver_flag flags();
	void set_flags(solver_flag flags);
	void unset_flags(solver_flag flags);

	virtual const char *name() const = 0;
	virtual void solve_for_object(const SimulationContext &context, Object *object) = 0;

	virtual void add_required_attributes(Object *) {}
};

/* ************************************************************************** */

class FreeFallSolver : public Solver {
public:
	FreeFallSolver();

	const char *name() const override;
	void solve_for_object(const SimulationContext &context, Object *object) override;
};

/* ************************************************************************** */

class SimpleParticleSolver : public Solver {
public:
	SimpleParticleSolver();

	const char *name() const override;
	void solve_for_object(const SimulationContext &context, Object *object) override;

	void add_required_attributes(Object *object) override;
};

/* ************************************************************************** */

class SimpleRBDSolver : public Solver {
public:
	SimpleRBDSolver();

	const char *name() const override;
	void solve_for_object(const SimulationContext &context, Object *object) override;

	void add_required_attributes(Object *object) override;
};

/* ************************************************************************** */

#include <kamikaze/factory.h>

using SolverFactory = Factory<Solver>;

#define REGISTER_SOLVER(factory, name, type) \
	REGISTER_TYPE(factory, name, Solver, type)

void register_builtin_solvers(SolverFactory *factory);

/* ************************************************************************** */

class Context;

class Simulation : public SceneNode {
	std::unordered_map<Object *, ObjectState> m_states;
	int m_start_frame = INVALID_FRAME;
	int m_last_frame = INVALID_FRAME;

	Solver *m_solver = nullptr;
	SimulationContext m_simcontext = {};

public:
	/* Prevent from having simulation with no solver. */
	Simulation() = delete;

	explicit Simulation(Solver *solver);
	~Simulation();

	int type() const override
	{
		return SCE_NODE_SIMULATION;
	}

	void step(const Context &context);

	void sync_states();
	bool update_properties() override;

private:
	float eval_gravity();
};
