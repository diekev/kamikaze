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

class SolverFactory final {
public:
	typedef Solver *(*factory_func)(void);

	/**
	 * @brief registerType Register a new element in this factory.
	 *
	 * @param key The key associate @ func to.
	 * @param func A function pointer with signature 'Solver *(void)'.
	 *
	 * @return The number of entries after registering the new element.
	 */
	size_t registerType(const std::string &key, factory_func func);

	/**
	 * @brief operator() Create a Solver based on the given key.
	 *
	 * @param key The key to lookup.
	 * @return A new Solver object corresponding to the given key.
	 */
	Solver *operator()(const std::string &key);

	/**
	 * @brief numEntries The number of entries registered in this factory.
	 *
	 * @return The number of entries registered in this factory, 0 if empty.
	 */
	size_t numEntries() const;

	/**
	 * @brief keys Keys registered in this factory.
	 *
	 * @return A vector containing the keys registered in this factory.
	 */
	std::vector<std::string> keys() const;

	/**
	 * @brief registered Check whether or not a key has been registered in this
	 *                   factory.
	 *
	 * @param key The key to lookup.
	 * @return True if the key is found, false otherwise.
	 */
	bool registered(const std::string &key) const;

private:
	std::unordered_map<std::string, factory_func> m_map;
};

void register_builtin_solvers(SolverFactory *factory);

/* ************************************************************************** */

class Simulation : public SceneNode {
	std::unordered_map<Object *, PrimitiveCollection *> m_states;
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

	void step(const EvaluationContext * const context);

	void sync_states();
	bool update_properties() override;

private:
	float eval_gravity();
};
