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

#include "object.h"

Simulation::Simulation()
{

}

void Simulation::step()
{
	auto gravity = glm::vec3(0.0f, -9.80665f, 0.0f);
	auto time_step = 0.1f;

	for (SceneInputSocket *input : m_inputs) {
		if (input->parent->type() != SCE_NODE_OBJECT) {
			continue;
		}

		Object *object = static_cast<Object *>(input->parent);

		auto pos = object->eval_vec3("Position");
		pos += time_step * gravity;
//		object->set_value("Position", pos);
	}
}
