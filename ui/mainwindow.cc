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

#include "mainwindow.h"

#include <iostream>

#include <kamikaze/context.h>
#include <kamikaze/primitive.h>
#include <kamikaze/nodes.h>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QKeyEvent>
#include <QListWidget>
#include <QSplitter>
#include <QTimer>

#include "core/nodes/graph.h"
#include "core/kamikaze_main.h"
#include "core/object.h"
#include "core/object_ops.h"
#include "core/scene.h"
#include "core/undo.h"

#include "node_compound.h"
#include "node_editorwidget.h"
#include "node_node.h"
#include "node_scene.h"

#include "paramcallback.h"
#include "ui_mainwindow.h"
#include "utils_ui.h"

MainWindow::MainWindow(Main *main, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_main(main)
    , m_scene(new Scene)
    , m_timer(new QTimer(this))
    , m_command_manager(new CommandManager)
    , m_command_factory(new CommandFactory)
    , m_timer_has_started(false)
{
	qApp->installEventFilter(this);
	ui->setupUi(this);
	ui->m_viewport->setScene(m_scene);

	connect(m_scene, SIGNAL(objectAdded(Object *)), this, SLOT(setupObjectUI(Object *)));
	connect(m_scene, SIGNAL(nodeAdded(Object *, Node *)), this, SLOT(setupNodeUI(Object *, Node *)));
	connect(ui->graph_editor, SIGNAL(objectNodeSelected(ObjectNodeItem *)), this, SLOT(setActiveObject(ObjectNodeItem *)));
	connect(ui->graph_editor, SIGNAL(objectNodeRemoved(ObjectNodeItem *)), this, SLOT(removeObject(ObjectNodeItem *)));
	connect(ui->graph_editor, SIGNAL(nodeRemoved(QtNode *)), this, SLOT(removeNode(QtNode *)));
	connect(ui->graph_editor, SIGNAL(nodeSelected(QtNode *)), this, SLOT(setupNodeParamUI(QtNode *)));
	connect(ui->graph_editor, SIGNAL(nodesConnected(QtNode *, const QString &, QtNode *, const QString &)),
	        this, SLOT(nodesConnected(QtNode *, const QString &, QtNode *, const QString &)));
	connect(ui->graph_editor, SIGNAL(connectionRemoved(QtNode *, const QString &, QtNode *, const QString &)),
	        this, SLOT(connectionRemoved(QtNode *, const QString &, QtNode *, const QString &)));

	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateFrame()));

	/* Timeline */
	ui->m_timeline->setMaximum(250);

	/* TODO: find another place to do this */
	generateNodeMenu();

	ui->graphicsView->GLScene(new OpenGLScene);
	ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->graphicsView->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
	ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	ui->graphicsView->GLScene()->setScene(m_scene);
	ui->graphicsView->adjustSize();
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_command_manager;
	delete m_command_factory;
	delete m_scene;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::KeyPress) {
		if (obj == ui->m_viewport) {
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
			ui->m_viewport->keyPressEvent(keyEvent);
			return true;
		}
	}
	else if (e->type() == QEvent::MouseButtonPress) {
		if (obj == ui->m_viewport) {
			QMouseEvent *event = static_cast<QMouseEvent *>(e);
			ui->m_viewport->mousePressEvent(event);
			return true;
		}
	}
	else if (e->type() == QEvent::MouseMove) {
		if (obj == ui->m_viewport) {
			QMouseEvent *event = static_cast<QMouseEvent *>(e);
			ui->m_viewport->mouseMoveEvent(event);
			return true;
		}
	}
	else if (e->type() == QEvent::MouseButtonRelease) {
		if (obj == ui->m_viewport) {
			QMouseEvent *event = static_cast<QMouseEvent *>(e);
			ui->m_viewport->mouseReleaseEvent(event);
			return true;
		}
	}
	else if (e->type() == QEvent::Wheel) {
		if (obj == ui->m_viewport) {
			QWheelEvent *event = static_cast<QWheelEvent *>(e);
			ui->m_viewport->wheelEvent(event);
			return true;
		}
	}

	return QObject::eventFilter(obj, e);
}

void MainWindow::updateObjectTab() const
{
	Object *ob = m_scene->currentObject();

	if (ob == nullptr) {
		return;
	}

	clear_layout(ui->node_param_layout);

	ParamCallback cb(ui->node_param_layout);
	ob->setUIParams(&cb);
	ob->primitive()->setCustomUIParams(&cb);

	cb.setContext(m_scene, SLOT(tagObjectUpdate()));

	m_scene->objectNameList(ui->m_outliner);
}

void MainWindow::startAnimation()
{
	if (m_timer_has_started) {
		m_timer_has_started = false;
		m_timer->stop();
		ui->m_start_but->setText("Play");
	}
	else {
		m_timer_has_started = true;
		ui->m_start_but->setText("Pause");
		m_timer->start(1000 / ui->m_fps->value());
	}
}

void MainWindow::goToStartFrame() const
{
	ui->m_timeline->setValue(ui->m_timeline->minimum());
}

void MainWindow::goToEndFrame() const
{
	ui->m_timeline->setValue(ui->m_timeline->maximum());
}

void MainWindow::undo() const
{
	/* TODO: figure out how to update everything properly */
	m_command_manager->undo();
}

void MainWindow::redo() const
{
	/* TODO: figure out how to update everything properly */
	m_command_manager->redo();
}

void MainWindow::updateFrame() const
{
	auto value = ui->m_timeline->value() + 1;

	if (value == ui->m_timeline->maximum()) {
		value = 0;
	}

	ui->m_timeline->setValue(value);

	m_scene->updateForNewFrame();
}

void MainWindow::setStartFrame(int value) const
{
	ui->m_timeline->setMinimum(value);
}

void MainWindow::setEndFrame(int value) const
{
	ui->m_timeline->setMaximum(value);
}

void MainWindow::generateObjectMenu()
{
	m_command_factory->registerType("add object", AddObjectCmd::registerSelf);

	for (const auto &key : m_main->objectFactory()->keys()) {
		auto action = ui->menuAdd->addAction(key.c_str());
		action->setData(QVariant::fromValue(QString("add object")));

		connect(action, SIGNAL(triggered()), this, SLOT(handleCommand()));
	}
}

void MainWindow::generateNodeMenu()
{
	m_command_factory->registerType("add node", AddNodeCmd::registerSelf);

	for (const auto &category : m_main->nodeFactory()->categories()) {
		auto sub_menu = ui->add_nodes_menu->addMenu(category.c_str());

		for (const auto &key : m_main->nodeFactory()->keys(category)) {
			auto action = sub_menu->addAction(key.c_str());
			action->setData(QVariant::fromValue(QString("add node")));

			connect(action, SIGNAL(triggered()), this, SLOT(handleCommand()));
		}
	}

	ui->graph_editor->setAddNodeMenu(ui->add_nodes_menu);
}

void MainWindow::handleCommand()
{
	QAction *action = qobject_cast<QAction *>(sender());

	if (!action) {
		return;
	}

	const auto &name = action->text().toStdString();
	const auto &data = action->data().toString().toStdString();

	/* get command */
	Command *cmd = (*m_command_factory)(data);
	cmd->setName(name);

	/* TODO */
	EvaluationContext context;
	context.scene = m_scene;
	context.object_factory = m_main->objectFactory();
	context.node_factory = m_main->nodeFactory();

	/* Create node */
	m_command_manager->execute(cmd, &context);
}

void MainWindow::setupNodeUI(Object *, Node *node)
{
	QtNode *node_item = new QtNode(node->name().c_str());
	node_item->setTitleColor(Qt::white);
	node_item->alignTitle(ALIGNED_LEFT);
	node_item->setNode(node);

	ui->graph_editor->addNode(node_item);
}

void MainWindow::setupNodeParamUI(QtNode *node_item)
{
	auto node = node_item->getNode();

	if (!node) {
		return;
	}

	clear_layout(ui->node_param_layout);

	ParamCallback cb(ui->node_param_layout);
	node->setUIParams(&cb);

	/* Only update/evaluate the graph if the node is connected. */
	if (node->isLinked()) {
		cb.setContext(m_scene, SLOT(evalObjectGraph()));
	}
}

void MainWindow::setupObjectUI(Object *object)
{
	ObjectNodeItem *obnode_item = new ObjectNodeItem(object, object->name());
	obnode_item->setTitleColor(Qt::white);
	obnode_item->alignTitle(ALIGNED_CENTER);

	/* add node item for the object's graph output node */
	{
		auto graph = object->graph();
		auto node = graph->output();

		QtNode *node_item = new QtNode(node->name().c_str());
		node_item->setTitleColor(Qt::white);
		node_item->alignTitle(ALIGNED_LEFT);
		node_item->setNode(node);
		node_item->setScene(obnode_item->nodeScene());
		node_item->setEditor(ui->graph_editor);

		obnode_item->addNode(node_item);
	}

	ui->graph_editor->addNode(obnode_item);

	updateObjectTab();
}

void MainWindow::setActiveObject(ObjectNodeItem *node)
{
	m_scene->setActiveObject(node->object());
	updateObjectTab();
}

void MainWindow::removeObject(ObjectNodeItem *node)
{
	m_scene->removeObject(node->object());
}

void MainWindow::removeNode(QtNode *node)
{
	auto object = m_scene->currentObject();
	auto graph = object->graph();

	const auto was_connected = node->getNode()->isLinked();

	graph->remove(node->getNode());

	if (was_connected) {
		object->evalGraph(true);
	}
}

void MainWindow::nodesConnected(QtNode *from, const QString &socket_from, QtNode *to, const QString &socket_to)
{
	auto object = m_scene->currentObject();
	auto graph = object->graph();

	auto node_from = from->getNode();
	auto node_to = to->getNode();

	auto output_socket = node_from->output(socket_from.toStdString());
	auto input_socket = node_to->input(socket_to.toStdString());

	assert((output_socket != nullptr) && (input_socket != nullptr));

	graph->connect(output_socket, input_socket);

	object->evalGraph(true);
}

void MainWindow::connectionRemoved(QtNode *from, const QString &socket_from, QtNode *to, const QString &socket_to)
{
	auto object = m_scene->currentObject();
	auto graph = object->graph();

	auto node_from = from->getNode();
	auto node_to = to->getNode();

	auto output_socket = node_from->output(socket_from.toStdString());
	auto input_socket = node_to->input(socket_to.toStdString());

	assert((output_socket != nullptr) && (input_socket != nullptr));

	graph->disconnect(output_socket, input_socket);

	object->evalGraph(true);
}
