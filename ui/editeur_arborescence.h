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

#include "base_editeur.h"

class Noeud;
class Scene;
class SceneNode;

/* ************************************************************************** */

class SceneTreeWidgetItem : public QWidget, public QTreeWidgetItem {
	Scene *m_scene;
    bool m_visited;

public:
    explicit SceneTreeWidgetItem(Scene *scene, QWidget *parent = nullptr);

    Scene *getScene() const;

    bool visited() const;
    void setVisited();
};

/* ************************************************************************** */

class ObjectTreeWidgetItem : public QTreeWidgetItem {
	SceneNode *m_scene_node;
    bool m_visited;

public:
    explicit ObjectTreeWidgetItem(SceneNode *scene_node, QTreeWidgetItem *parent = nullptr);

    SceneNode *getNode() const;

    bool visited() const;
    void setVisited();
};

/* ************************************************************************** */

class ObjectNodeTreeWidgetItem : public QTreeWidgetItem {
	Noeud *m_noeud;

public:
	explicit ObjectNodeTreeWidgetItem(Noeud *noeud, QTreeWidgetItem *parent = nullptr);

	Noeud *pointeur_noeud() const;
};

/* ************************************************************************** */

class TreeWidget : public QTreeWidget {
	BaseEditeur *m_base = nullptr;

public:
	explicit TreeWidget(QWidget *parent = nullptr);

	void set_base(BaseEditeur *base);

	void mousePressEvent(QMouseEvent *e) override;
	void dropEvent(QDropEvent *event) override;
};

/* ************************************************************************** */

/* This is to add a level of indirection because we can't have an object derive
 * from both QTreeWidget and WidgetBase, and we can't apparently use virtual
 * inheritance with Qt classes. */
class EditeurArborescence : public BaseEditeur {
	Q_OBJECT

	TreeWidget *m_tree_widget;

public:
	explicit EditeurArborescence(QWidget *parent = nullptr);

	void update_state(event_type event) override;

public Q_SLOTS:
	void handleItemCollapsed(QTreeWidgetItem *item);
	void handleItemExpanded(QTreeWidgetItem *item);
	void handleItemSelection();
};
