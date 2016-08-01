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

#include "simulation.h"

#include <kamikaze/context.h>

#include "object.h"
#include "scene.h"

Simulation::Simulation()
{
	m_name = "Simulation";
	add_input("Input");
}

void Simulation::step(const EvaluationContext * const context)
{
	auto scene = context->scene;

	if (m_start_frame != scene->startFrame()) {
		m_start_frame = scene->startFrame();
	}

	if (scene->currentFrame() == m_start_frame) {
		/* Save object state. */
		sync_states();

		m_last_frame = scene->currentFrame();
		return;
	}

	/* Make sure we didn't advance too much. */
	if (scene->currentFrame() != m_last_frame + 1) {
		return;
	}

	m_last_frame = scene->currentFrame();

	auto gravity = glm::vec3(0.0f, -9.80665f, 0.0f);
	auto time_step = (context->time_direction == TIME_DIR_BACKWARD) ? -0.1f : 0.1f;

	for (SceneInputSocket *input : m_inputs) {
		if (!input->link) {
			continue;
		}

		Object *object = static_cast<Object *>(input->link->parent);

		auto pos = object->eval_vec3("Position");
		pos += time_step * gravity;
		object->set_prop_value("Position", pos);

		object->updateMatrix();
	}
}

void Simulation::sync_states()
{
	for (SceneInputSocket *input : m_inputs) {
		if (!input->link) {
			continue;
		}

		auto object = static_cast<Object *>(input->link->parent);

		auto iter = m_states.find(object);

		if (iter == m_states.end()) {
			m_states[object] = object->eval_vec3("Position");
		}
		else {
			object->set_prop_value("Position", iter->second);
			object->updateMatrix();
		}
	}
}
