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

#include "outliner_widget.h"

#include "core/object.h"
#include "core/scene.h"

/* ************************************************************************** */

SceneTreeWidgetItem::SceneTreeWidgetItem(QWidget *parent)
    : QWidget(parent)
    , m_scene(nullptr)
    , m_visited(false)
{}

Scene *SceneTreeWidgetItem::getScene() const
{
	return m_scene;
}

void SceneTreeWidgetItem::setScene(Scene *scene)
{
	m_scene = scene;
	setText(0, "Scene");
	setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
}

bool SceneTreeWidgetItem::visited() const
{
	return m_visited;
}

void SceneTreeWidgetItem::setVisited()
{
	m_visited = true;
}

int SceneTreeWidgetItem::numChildren() const
{
	return m_scene->objects().size();
}

/* ************************************************************************** */

ObjectTreeWidgetItem::ObjectTreeWidgetItem(QTreeWidgetItem *parent)
    : QTreeWidgetItem(parent)
    , m_object(nullptr)
    , m_visited(false)
{}

Object *ObjectTreeWidgetItem::getObject() const
{
	return m_object;
}

void ObjectTreeWidgetItem::setObject(Object *object)
{
	m_object = object;
	setText(0, m_object->name());

	if (this->numChildren() > 0) {
		setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	}
}

bool ObjectTreeWidgetItem::visited() const
{
	return m_visited;
}

void ObjectTreeWidgetItem::setVisited()
{
	m_visited = true;
}

int ObjectTreeWidgetItem::numChildren() const
{
	return getObject()->children().size();
}

/* ************************************************************************** */

OutlinerTreeWidget::OutlinerTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
	setIconSize(QSize(20, 20));
	setAllColumnsShowFocus(true);
	setAnimated(false);
	setAutoScroll(false);
	setUniformRowHeights(true);
	setSelectionMode(SingleSelection);
	setDragDropMode(NoDragDrop);
	setFocusPolicy(Qt::NoFocus);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setHeaderHidden(true);

	connect(this, SIGNAL(itemExpanded(QTreeWidgetItem *)),
	        this, SLOT(handleItemExpanded(QTreeWidgetItem *)));

	connect(this, SIGNAL(itemSelectionChanged()),
	        this, SLOT(handleItemSelection()));
}

void OutlinerTreeWidget::setScene(Scene *scene)
{
	m_scene = scene;
	SceneTreeWidgetItem *item = new SceneTreeWidgetItem(this);
    item->setScene(m_scene);
    addTopLevelItem(item);
}

void OutlinerTreeWidget::updateScene()
{
	/* TODO */
	clear();
	setScene(m_scene);
}

void OutlinerTreeWidget::keyPressEvent(QKeyEvent *e)
{
	return QTreeWidget::keyPressEvent(e);
}

void OutlinerTreeWidget::mouseMoveEvent(QMouseEvent *e)
{
	return QTreeWidget::mouseMoveEvent(e);
}

void OutlinerTreeWidget::mousePressEvent(QMouseEvent *e)
{
	return QTreeWidget::mousePressEvent(e);
}

void OutlinerTreeWidget::mouseReleaseEvent(QMouseEvent *e)
{
	return QTreeWidget::mouseReleaseEvent(e);
}

void OutlinerTreeWidget::wheelEvent(QWheelEvent *e)
{
	return QTreeWidget::wheelEvent(e);
}

void OutlinerTreeWidget::handleItemExpanded(QTreeWidgetItem *item)
{
	auto scene_item = dynamic_cast<SceneTreeWidgetItem *>(item);

	if (scene_item && !scene_item->visited()) {
		Scene *scene = scene_item->getScene();

		for (const auto &object : scene->objects()) {
			auto child = new ObjectTreeWidgetItem(scene_item);
			child->setObject(object);
			scene_item->addChild(child);
		}

		scene_item->setVisited();
		return;
	}

	auto object_item = dynamic_cast<ObjectTreeWidgetItem *>(item);

	if (object_item && !object_item->visited()) {
		Object *object = object_item->getObject();

		for (const auto &child : object->children()) {
			auto child_item = new ObjectTreeWidgetItem(object_item);
			child_item->setObject(child);
			object_item->addChild(child_item);
		}

		object_item->setVisited();
		return;
	}
}

void OutlinerTreeWidget::handleItemSelection()
{
	auto items = selectedItems();

	if (items.size() != 1) {
		return;
	}

	auto item = items[0];

	auto scene_item = dynamic_cast<SceneTreeWidgetItem *>(item);

	if (scene_item) {
		return;
	}

	auto object_item = dynamic_cast<ObjectTreeWidgetItem *>(item);

	if (object_item) {
		m_scene->setActiveObject(object_item->getObject());
		return;
	}
}
