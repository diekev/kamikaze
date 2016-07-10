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

#include <kamikaze/primitive.h>
#include <kamikaze/nodes.h>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QKeyEvent>
#include <QListWidget>
#include <QSplitter>
#include <QTimer>

#include "core/graphs/object_graph.h"
#include "core/graphs/object_nodes.h"
#include "core/kamikaze_main.h"
#include "core/object.h"
#include "core/object_ops.h"
#include "core/scene.h"

#include "node_compound.h"
#include "node_editorwidget.h"
#include "node_node.h"
#include "node_scene.h"

#include "paramcallback.h"
#include "paramfactory.h"
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
	generateObjectMenu();
	generateNodeMenu();
	generatePresetMenu();

	m_progress_bar = new QProgressBar(this);
	ui->statusBar->addWidget(m_progress_bar);
	m_progress_bar->setRange(0, 100);
	m_progress_bar->setVisible(false);

	/* setup context */
	m_context.scene = m_scene;
	m_context.node_factory = m_main->nodeFactory();
	m_context.primitive_factory = m_main->primitiveFactory();
	m_context.main_window = this;

	ui->m_start_but->setIcon(QIcon("icons/icon_play_forward.png"));
	ui->m_begin_but->setIcon(QIcon("icons/icon_jump_first.png"));
	ui->m_end_but->setIcon(QIcon("icons/icon_jump_last.png"));

	ui->treeWidget->setScene(m_scene);
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_command_manager;
	delete m_command_factory;
	delete m_scene;
}

void MainWindow::taskStarted()
{
	m_progress_bar->setValue(0);
	m_progress_bar->setVisible(true);
}

void MainWindow::updateProgress(float progress)
{
	m_progress_bar->setValue(progress);
}

void MainWindow::taskEnded()
{
	m_progress_bar->setVisible(false);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::KeyPress) {
		auto event = static_cast<QKeyEvent *>(e);

		if (obj == ui->m_viewport) {
			ui->m_viewport->keyPressEvent(event);
			return true;
		}
		else if (obj == ui->treeWidget) {
			ui->treeWidget->keyPressEvent(event);
			return true;
		}
	}
	else if (e->type() == QEvent::MouseButtonPress) {
		auto event = static_cast<QMouseEvent *>(e);

		if (obj == ui->m_viewport) {
			ui->m_viewport->mousePressEvent(event);
			return true;
		}
		else if (obj == ui->treeWidget) {
			ui->treeWidget->mousePressEvent(event);
			return true;
		}
	}
	else if (e->type() == QEvent::MouseMove) {
		auto event = static_cast<QMouseEvent *>(e);

		if (obj == ui->m_viewport) {
			ui->m_viewport->mouseMoveEvent(event);
			return true;
		}
		else if (obj == ui->treeWidget) {
			ui->treeWidget->mouseMoveEvent(event);
			return true;
		}
	}
	else if (e->type() == QEvent::MouseButtonRelease) {
		auto event = static_cast<QMouseEvent *>(e);

		if (obj == ui->m_viewport) {
			ui->m_viewport->mouseReleaseEvent(event);
			return true;
		}
		else if (obj == ui->treeWidget) {
			ui->treeWidget->mouseReleaseEvent(event);
			return true;
		}
	}
	else if (e->type() == QEvent::Wheel) {
		auto event = static_cast<QWheelEvent *>(e);

		if (obj == ui->m_viewport) {
			ui->m_viewport->wheelEvent(event);
			return true;
		}
		else if (obj == ui->treeWidget) {
			ui->treeWidget->wheelEvent(event);
			return true;
		}
	}

	return QObject::eventFilter(obj, e);
}

void MainWindow::updateObjectTab() const
{
	auto ob = m_scene->currentObject();

	if (ob == nullptr) {
		return;
	}

	clear_layout(ui->node_param_layout);

	ParamCallback cb(ui->node_param_layout);
	ob->setUIParams(&cb);

	cb.setContext(m_scene, SLOT(tagObjectUpdate()));

	ui->treeWidget->updateScene();
}

void MainWindow::startAnimation()
{
	if (m_timer_has_started) {
		m_timer_has_started = false;
		m_timer->stop();
		ui->m_start_but->setText("Play");
		ui->m_start_but->setIcon(QIcon("icons/icon_play_forward.png"));
	}
	else {
		m_timer_has_started = true;
		ui->m_start_but->setText("Pause");
		ui->m_start_but->setIcon(QIcon("icons/icon_pause.png"));
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

	m_scene->updateForNewFrame(&m_context);
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

	auto action = ui->menuAdd->addAction("Empty Object");
	action->setData(QVariant::fromValue(QString("add object")));

	connect(action, SIGNAL(triggered()), this, SLOT(handleCommand()));

#if 0
	for (const auto &key : m_main->primitiveFactory()->keys()) {
		auto action = ui->menuAdd->addAction(key.c_str());
		action->setData(QVariant::fromValue(QString("add object")));

		connect(action, SIGNAL(triggered()), this, SLOT(handleCommand()));
	}
#endif
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

struct UIProp {
	const char *name;
	const char *icon_path;
};

void MainWindow::generatePresetMenu()
{
	m_command_factory->registerType("add preset", AddPresetObjectCmd::registerSelf);

	UIProp props[] = {
	    { "Grid", "icons/icon_grid.png" },
	    { "Box", "icons/icon_box.png" },
	    { "Circle", "icons/icon_circle.png" },
	    { "IcoSphere", "icons/icon_icosphere.png" },
	    { "Tube", "icons/icon_tube.png" },
	    { "Cone", "icons/icon_cone.png" },
	    { "Torus", "icons/icon_torus.png" },
	};

	for (const auto &prop : props) {
		auto action = ui->toolBar->addAction(QIcon(prop.icon_path), prop.name);
		action->setData(QVariant::fromValue(QString("add preset")));

		connect(action, SIGNAL(triggered()), this, SLOT(handleCommand()));
	}
}

void MainWindow::handleCommand()
{
	auto action = qobject_cast<QAction *>(sender());

	if (!action) {
		return;
	}

	const auto &name = action->text().toStdString();
	const auto &data = action->data().toString().toStdString();

	/* Get command, and give it the name of the UI button which will be used to
	 * look up keys in the various creation factories if need be. This could and
	 * should be handled better. */
	auto cmd = (*m_command_factory)(data);
	cmd->setName(name);

	/* TODO: handle this in a better way. */
	m_context.edit_mode = (ui->graph_editor->editor_mode() == EDITOR_MODE_OBJECT);

	/* Execute the command in the current context, the manager will push the
	* command on the undo stack. */
	m_command_manager->execute(cmd, &m_context);
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

	node->update_properties();

	ParamCallback *callback = &cb;

	for (Property &prop : node->props()) {
		if (!prop.visible) {
			continue;
		}

		assert(!prop.data.empty());

		switch (prop.type) {
			case property_type::prop_bool:
				bool_param(callback,
				           prop.name.c_str(),
				           any_cast<bool>(&prop.data),
				           any_cast<bool>(prop.data));
				break;
			case property_type::prop_float:
				float_param(callback,
				            prop.name.c_str(),
				            any_cast<float>(&prop.data),
				            prop.min, prop.max,
				            any_cast<float>(prop.data));
				break;
			case property_type::prop_int:
				int_param(callback,
				          prop.name.c_str(),
				          any_cast<int>(&prop.data),
				          prop.min, prop.max,
				          any_cast<int>(prop.data));
				break;
			case property_type::prop_enum:
				enum_param(callback,
				           prop.name.c_str(),
				           any_cast<int>(&prop.data),
				           prop.enum_items,
				           any_cast<int>(prop.data));
				break;
			case property_type::prop_vec3:
				xyz_param(callback,
				          prop.name.c_str(),
				          &(any_cast<glm::vec3>(&prop.data)->x));
				break;
			case property_type::prop_input_file:
				input_file_param(callback,
				                 prop.name.c_str(),
				                 any_cast<std::string>(&prop.data));
				break;
			case property_type::prop_output_file:
				output_file_param(callback,
				                  prop.name.c_str(),
				                  any_cast<std::string>(&prop.data));
				break;
			case property_type::prop_string:
				string_param(callback,
				             prop.name.c_str(),
				             any_cast<std::string>(&prop.data),
				             any_cast<std::string>(prop.data).c_str());
				break;
		}

		if (!prop.tooltip.empty()) {
			param_tooltip(callback, prop.tooltip.c_str());
		}
	}

	/* Only update/evaluate the graph if the node is connected. */
	if (node->isLinked()) {
		cb.setContext(this, SLOT(evalObjectGraph()));
	}
}

/* TODO: evaluation system */
void MainWindow::evalObjectGraph()
{
	eval_graph(&m_context);
}

void MainWindow::setupObjectUI(Object *object)
{
	auto obnode_item = new ObjectNodeItem(object, object->name());
	obnode_item->setTitleColor(Qt::white);
	obnode_item->alignTitle(ALIGNED_CENTER);

	/* add node item for the object's graph output node */
	{
		auto graph = object->graph();
		auto output_node = graph->output();

		QtNode *out_node_item = new QtNode(output_node->name().c_str());
		out_node_item->setTitleColor(Qt::white);
		out_node_item->alignTitle(ALIGNED_LEFT);
		out_node_item->setNode(output_node);
		out_node_item->setScene(obnode_item->nodeScene());
		out_node_item->setEditor(ui->graph_editor);

		obnode_item->addNode(out_node_item);

		/* TODO: this is just when adding an object from a preset. */
		if (graph->nodes().size() > 1) {
			Node *node = graph->nodes().back();

			QtNode *node_item = new QtNode(node->name().c_str());
			node_item->setTitleColor(Qt::white);
			node_item->alignTitle(ALIGNED_LEFT);
			node_item->setNode(node);
			node_item->setScene(obnode_item->nodeScene());
			node_item->setEditor(ui->graph_editor);
			node_item->setPos(out_node_item->pos() - QPointF(200, 100));

			obnode_item->addNode(node_item);

			ui->graph_editor->connectNodes(node_item, node_item->output(0),
			                               out_node_item, out_node_item->input(0));
		}
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
		eval_graph(&m_context);
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

	eval_graph(&m_context);
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

	eval_graph(&m_context);
}
