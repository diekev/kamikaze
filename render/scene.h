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

#include <QObject>

#include "../util/util_render.h"

class Brush;
class Object;
class QListWidget;
class QListWidgetItem;
class SmokeSimulation;

enum {
	SCENE_MODE_OBJECT = 0,
	SCENE_MODE_SCULPT = 1,
};

class Scene : public QObject {
	Q_OBJECT

	std::vector<Object *> m_objects;
	Object *m_active_object;
	Brush *m_brush;
	SmokeSimulation *m_smoke_simulation;
	int m_mode;

public:
	Scene();
	~Scene();

	void keyboardEvent(int key);

	Object *currentObject();
	void addObject(Object *object);
	void removeObject(Object *ob);

	void render(const glm::mat4 &MV, const glm::mat4 &P, const glm::vec3 &view_dir);
	void intersect(const Ray &ray);

	int mode() const;
	void selectObject(const glm::vec3 &pos);
	void objectNameList(QListWidget *widget) const;

public Q_SLOTS:
	void setMode(int mode);

	void moveObject(double value, int axis);
	void scaleObject(double value, int axis);
	void rotateObject(double value, int axis);
	void setVoxelSize(double value);
	void setObjectName(const QString &name);

	void setBrushMode(int mode);
	void setBrushRadius(double value);
	void setBrushStrength(double value);
	void setBrushTool(int tool);

	void setCurrentObject(QListWidgetItem *item);

	void setSimulationDt(double value);
	void setSimulationCache(const QString &path);
	void setSimulationAdvection(int index);

	void setVolumeSlices(int slices);
	void setVolumeLUT(bool b);

private:
	bool ensureUniqueName(QString &name) const;

Q_SIGNALS:
	void objectChanged();
};
