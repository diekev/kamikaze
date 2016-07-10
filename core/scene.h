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

#include <kamikaze/util_render.h>

class Depsgraph;
class EvaluationContext;
class Node;
class Object;

class Scene : public QObject {
	Q_OBJECT

	std::vector<Object *> m_objects;
	Object *m_active_object;
	int m_mode;

	Depsgraph *m_depsgraph;

public:
	Scene();
	~Scene();

	Object *currentObject();
	void addObject(Object *object);
	void removeObject(Object *object);

	void intersect(const Ray &ray);

	void selectObject(const glm::vec3 &pos);

	void emitNodeAdded(Object *ob, Node *node);

	void updateForNewFrame(const EvaluationContext * const context);

	const std::vector<Object *> &objects() const;

public Q_SLOTS:
	void setObjectName(const QString &name);
	void tagObjectUpdate();

	void setActiveObject(Object *object);

private:
	bool ensureUniqueName(QString &name) const;

Q_SIGNALS:
	void objectAdded(Object *);
	void nodeAdded(Object *, Node *);
	void objectChanged();
};
