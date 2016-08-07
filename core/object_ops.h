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

#include "simulation.h"
#include "undo.h"

class Object;
class Scene;

class AddObjectCmd : public Command {
	Object *m_object = nullptr;
	Scene *m_scene = nullptr;

	/* TODO */
	bool m_was_undone = false;

public:
	AddObjectCmd() = default;
	~AddObjectCmd();

	void execute(EvaluationContext *context) override;
	void undo() override;
	void redo() override;
};

class AddNodeCmd : public Command {
	Object *m_object = nullptr;
	Scene *m_scene = nullptr;

public:
	AddNodeCmd() = default;
	~AddNodeCmd() = default;

	void execute(EvaluationContext *context) override;
	void undo() override;
	void redo() override;
};

class AddPresetObjectCmd : public Command {
	Object *m_object = nullptr;
	Scene *m_scene = nullptr;

public:
	AddPresetObjectCmd() = default;
	~AddPresetObjectCmd() = default;

	void execute(EvaluationContext *context) override;
	void undo() override;
	void redo() override;
};

class AddSimulationCmd : public Command {
	SolverFactory *m_solver_factory;

public:
	AddSimulationCmd() = default;
	~AddSimulationCmd() = default;

	void execute(EvaluationContext *context) override;
	void undo() override;
	void redo() override;
	void set_solver_factory(SolverFactory *solver_factory);
};
