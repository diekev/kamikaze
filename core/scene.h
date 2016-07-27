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

#include <QString>

#include <kamikaze/util_render.h>

#include "context.h"

class Depsgraph;
class EvaluationContext;
class Node;
class Object;

class Scene : public Listened {
	std::vector<Object *> m_objects;
	Object *m_active_object;
	int m_mode;

	Depsgraph *m_depsgraph;

	int m_start_frame;
	int m_end_frame;
	int m_cur_frame;
	float m_fps = 24.0f;

public:
	Scene();
	~Scene();

	Object *currentObject();
	void setActiveObject(Object *object);

	void addObject(Object *object);
	void removeObject(Object *object);

	void intersect(const Ray &ray);

	void selectObject(const glm::vec3 &pos);

	/* Time/Frame */

	int startFrame() const;
	void startFrame(int value);

	int endFrame() const;
	void endFrame(int value);

	int currentFrame() const;
	void currentFrame(int value);

	float framesPerSecond() const;
	void framesPerSecond(float value);

	void updateForNewFrame(const EvaluationContext * const context);

	const std::vector<Object *> &objects() const;

	void tagObjectUpdate();

private:
	bool ensureUniqueName(QString &name) const;
};
