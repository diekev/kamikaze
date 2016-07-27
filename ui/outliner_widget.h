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

#include <QTreeWidget>

#include "context.h"

class Object;
class Scene;

/* ************************************************************************** */

class SceneTreeWidgetItem : public QWidget, public QTreeWidgetItem {
	Scene *m_scene;
    bool m_visited;

public:
    explicit SceneTreeWidgetItem(QWidget *parent = nullptr);

    Scene *getScene() const;
    void setScene(Scene *scene);

    bool visited() const;
    void setVisited();

    int numChildren() const;
};

/* ************************************************************************** */

class ObjectTreeWidgetItem : public QTreeWidgetItem {
	Object *m_object;
    bool m_visited;

public:
    explicit ObjectTreeWidgetItem(QTreeWidgetItem *parent = nullptr);

    Object *getObject() const;
    void setObject(Object *object);

    bool visited() const;
    void setVisited();

    int numChildren() const;
};

/* ************************************************************************** */

class OutlinerTreeWidget : public QTreeWidget, public ContextListener {
	Q_OBJECT

public:
	explicit OutlinerTreeWidget(QWidget *parent = nullptr);

	void update_state(int event_type) override;

	/* Events. */
	void keyPressEvent(QKeyEvent *e) override;
	void mouseMoveEvent(QMouseEvent *e) override;
	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void wheelEvent(QWheelEvent *e) override;

public Q_SLOTS:
	void handleItemExpanded(QTreeWidgetItem *item);
	void handleItemSelection();
};
