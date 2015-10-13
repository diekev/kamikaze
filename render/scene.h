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

#include <glm/glm.hpp>
#include <QObject>

#include "util_render.h"

class Brush;
class Object;

enum {
	SCENE_MODE_OBJECT = 0,
	SCENE_MODE_SCULPT = 1,
};

class Scene : public QObject {
	Q_OBJECT

	std::vector<Object *> m_objects;
	Object *m_active_object;
	Brush *m_brush;
	int m_mode;

Q_SIGNALS:
	void objectChanged();
	void updateViewport();

public Q_SLOTS:
	void setMode(int mode);

	void moveObjectX(double value);
	void moveObjectY(double value);
	void moveObjectZ(double value);
	void scaleObjectX(double value);
	void scaleObjectY(double value);
	void scaleObjectZ(double value);
	void rotateObjectX(double value);
	void rotateObjectY(double value);
	void rotateObjectZ(double value);
	void setVoxelSize(double value);
	void setObjectName(const QString &name);

	void setBrushMode(int mode);
	void setBrushRadius(double value);
	void setBrushStrength(double value);
	void setBrushTool(int tool);

public:
	Scene();
	~Scene();

	void keyboardEvent(int key);

	Object *currentObject();
	void addObject(Object *object);

	void render(const glm::mat4 &MV, const glm::mat4 &P, const glm::vec3 &view_dir);
	void intersect(const Ray &ray);

	int mode() const;
	void selectObject(const glm::vec3 &pos);
};
