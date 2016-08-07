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

#include <kamikaze/context.h>

#include <QDropEvent>

#include "core/object.h"
#include "core/scene.h"

#include "util/utils.h"

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
	return m_scene->nodes().size();
}

/* ************************************************************************** */

ObjectTreeWidgetItem::ObjectTreeWidgetItem(QTreeWidgetItem *parent)
    : QTreeWidgetItem(parent)
    , m_scene_node(nullptr)
    , m_visited(false)
{
	setFlags(flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
}

SceneNode *ObjectTreeWidgetItem::getNode() const
{
	return m_scene_node;
}

void ObjectTreeWidgetItem::setNode(SceneNode *scene_node)
{
	m_scene_node = scene_node;
	setText(0, m_scene_node->name().c_str());

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
	auto node = getNode();
	return static_cast<Object *>(node)->children().size();
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
	setDragDropMode(InternalMove);
	setDragEnabled(true);
	setFocusPolicy(Qt::NoFocus);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setHeaderHidden(true);

	connect(this, SIGNAL(itemExpanded(QTreeWidgetItem *)),
	        this, SLOT(handleItemExpanded(QTreeWidgetItem *)));

	connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem *)),
	        this, SLOT(handleItemCollapsed(QTreeWidgetItem *)));

	connect(this, SIGNAL(itemSelectionChanged()),
	        this, SLOT(handleItemSelection()));
}

void OutlinerTreeWidget::update_state(event_type event)
{
	if (event == static_cast<event_type>(-1)) {
		return;
	}

	if (get_category(event) != event_type::object) {
		return;
	}

	if (!is_elem(get_action(event), event_type::added, event_type::parented)) {
		return;
	}

	/* TODO */
	clear();

	auto scene = m_context->scene;
	auto item = new SceneTreeWidgetItem(this);
    item->setScene(scene);
    addTopLevelItem(item);

	/* Need to first add the item to the tree. */
	item->setExpanded(scene->has_flags(SCENE_OL_EXPANDED));
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
		scene->set_flags(SCENE_OL_EXPANDED);

		for (const auto &node : scene->nodes()) {
			auto object = static_cast<Object *>(node);

			if (object->parent() != nullptr) {
				continue;
			}

			auto child = new ObjectTreeWidgetItem(scene_item);
			child->setNode(node);
			child->setSelected(node == scene->active_node());
			scene_item->addChild(child);
			child->setExpanded(node->has_flags(SNODE_OL_EXPANDED));
		}

		scene_item->setVisited();
		return;
	}

	auto object_item = dynamic_cast<ObjectTreeWidgetItem *>(item);

	if (object_item && !object_item->visited()) {
		auto node = object_item->getNode();
		node->set_flags(SNODE_OL_EXPANDED);
		auto object = static_cast<Object *>(node);

		for (const auto &child : object->children()) {
			auto child_item = new ObjectTreeWidgetItem(object_item);
			child_item->setNode(child);
			child_item->setSelected(child == m_context->scene->active_node());
			object_item->addChild(child_item);
			child_item->setExpanded(child->has_flags(SNODE_OL_EXPANDED));
		}

		object_item->setVisited();
		return;
	}
}

void OutlinerTreeWidget::handleItemCollapsed(QTreeWidgetItem *item)
{
	auto scene_item = dynamic_cast<SceneTreeWidgetItem *>(item);

	if (scene_item) {
		Scene *scene = scene_item->getScene();
		scene->unset_flags(SCENE_OL_EXPANDED);
		return;
	}

	auto object_item = dynamic_cast<ObjectTreeWidgetItem *>(item);

	if (object_item) {
		auto node = object_item->getNode();
		node->unset_flags(SNODE_OL_EXPANDED);
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
		m_context->scene->set_active_node(object_item->getNode());
		return;
	}
}

void OutlinerTreeWidget::dropEvent(QDropEvent *event)
{
	if (event->source() != this) {
		return;
	}

	auto item = itemAt(event->pos());

	if (!item) {
		return;
	}

	auto object_item = dynamic_cast<ObjectTreeWidgetItem *>(item);

	if (!object_item) {
		return;
	}

	auto source_item = dynamic_cast<ObjectTreeWidgetItem *>(selectedItems()[0]);

	if (!source_item) {
		return;
	}

	if (source_item == object_item) {
		return;
	}

	m_context->scene->connect(m_context, object_item->getNode(), source_item->getNode());

	QTreeView::dropEvent(event);
}
