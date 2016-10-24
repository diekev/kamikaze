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

#include <kamikaze/primitive.h>
#include <kamikaze/nodes.h>

#include <QDockWidget>
#include <QMenuBar>
#include <QProgressBar>
#include <QStatusBar>
#include <QToolBar>

#include "core/graphs/graph_dumper.h"
#include "core/kamikaze_main.h"
#include "core/object.h"
#include "core/object_ops.h"

#include "node_editorwidget.h"
#include "outliner_widget.h"
#include "properties_widget.h"
#include "timeline_widget.h"
#include "utils_ui.h"
#include "viewer.h"

MainWindow::MainWindow(Main *main, QWidget *parent)
    : QMainWindow(parent)
    , m_main(main)
    , m_command_manager(new CommandManager)
    , m_command_factory(new CommandFactory)
{
	generateObjectMenu();
	generateNodeMenu();
	generateWindowMenu();
	generatePresetMenu();
	generateDebugMenu();

	m_progress_bar = new QProgressBar(this);
	statusBar()->addWidget(m_progress_bar);
	m_progress_bar->setRange(0, 100);
	m_progress_bar->setVisible(false);

	/* setup context */
	m_eval_context.edit_mode = false;
	m_eval_context.animation = false;
	m_context.eval_ctx = &m_eval_context;
	m_context.scene = m_main->scene();
	m_context.node_factory = m_main->node_factory();
	m_context.primitive_factory = m_main->primitive_factory();
	m_context.main_window = this;
	m_context.active_widget = nullptr;

	m_has_glwindow = false;

	addGLViewerWidget();
	addPropertiesWidget();
	addGraphOutlinerWidget();
	addTimeLineWidget();

	setCentralWidget(nullptr);
}

MainWindow::~MainWindow()
{
	delete m_command_manager;
	delete m_command_factory;
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

void MainWindow::nodeProcessed()
{
	m_context.scene->notify_listeners(event_type::node | event_type::processed);
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

void MainWindow::generateObjectMenu()
{
	REGISTER_COMMAND(m_command_factory, "add object", AddObjectCmd);

	m_add_object_menu = menuBar()->addMenu("Add Object");
	auto action = m_add_object_menu->addAction("Empty Object");
	action->setData(QVariant::fromValue(QString("add object")));

	connect(action, SIGNAL(triggered()), this, SLOT(handleCommand()));
}

void MainWindow::generateDebugMenu()
{
	m_add_object_menu = menuBar()->addMenu("Debug");

	QAction *action;

	action = m_add_object_menu->addAction("Dump Dependency Graph");
	action->setData(QVariant::fromValue(QString("dump_dependency_graph")));

	connect(action, SIGNAL(triggered()), this, SLOT(dumpGraph()));

	action = m_add_object_menu->addAction("Dump Object Graph");
	action->setData(QVariant::fromValue(QString("dump_object_graph")));

	connect(action, SIGNAL(triggered()), this, SLOT(dumpGraph()));
}

void MainWindow::generateNodeMenu()
{
	REGISTER_COMMAND(m_command_factory, "add node", AddNodeCmd);

	m_add_nodes_menu = menuBar()->addMenu("Add Node");

	auto categories = m_main->node_factory()->categories();
	std::sort(categories.begin(), categories.end());

	for (const auto &category : categories) {
		auto sub_menu = m_add_nodes_menu->addMenu(category.c_str());

		auto keys = m_main->node_factory()->keys(category);
		std::sort(keys.begin(), keys.end());

		for (const auto &key : keys) {
			auto action = sub_menu->addAction(key.c_str());
			action->setData(QVariant::fromValue(QString("add node")));

			connect(action, SIGNAL(triggered()), this, SLOT(handleCommand()));
		}
	}
}

void MainWindow::generateWindowMenu()
{
	m_add_window_menu = menuBar()->addMenu("View");

	QAction *action;

	action = m_add_window_menu->addAction("3D View");
	action->setToolTip("Add a 3D View");
	connect(action, SIGNAL(triggered()), this, SLOT(addGLViewerWidget()));

	action = m_add_window_menu->addAction("Graph Editor");
	action->setToolTip("Add a Graph Editor");
	connect(action, SIGNAL(triggered()), this, SLOT(addGraphEditorWidget()));

	action = m_add_window_menu->addAction("Outliner");
	action->setToolTip("Add an Outliner");
	connect(action, SIGNAL(triggered()), this, SLOT(addOutlinerWidget()));

	action = m_add_window_menu->addAction("Properties Editor");
	action->setToolTip("Add a Properties Editor");
	connect(action, SIGNAL(triggered()), this, SLOT(addPropertiesWidget()));

	action = m_add_window_menu->addAction("Time Line");
	action->setToolTip("Add a Time Line");
	connect(action, SIGNAL(triggered()), this, SLOT(addTimeLineWidget()));
}

void MainWindow::generateEditMenu()
{
	m_edit_menu = menuBar()->addMenu("Edit");

	QAction *action;

	action = m_edit_menu->addAction("Undo");
	connect(action, SIGNAL(triggered()), this, SLOT(undo()));

	action = m_edit_menu->addAction("Redo");
	connect(action, SIGNAL(triggered()), this, SLOT(redo()));
}

void MainWindow::generatePresetMenu()
{
	REGISTER_COMMAND(m_command_factory, "add preset", AddPresetObjectCmd);

	m_tool_bar = new QToolBar();
	addToolBar(Qt::TopToolBarArea, m_tool_bar);

	UIButData props[] = {
	    { 0, "Grid", "icons/icon_grid.png" },
	    { 0, "Box", "icons/icon_box.png" },
	    { 0, "Circle", "icons/icon_circle.png" },
	    { 0, "IcoSphere", "icons/icon_icosphere.png" },
	    { 0, "Tube", "icons/icon_tube.png" },
	    { 0, "Cone", "icons/icon_cone.png" },
	    { 0, "Torus", "icons/icon_torus.png" },
	    { 0, "Point Cloud", "icons/icon_point_cloud_cube.png" },
	};

	for (const auto &prop : props) {
		auto action = m_tool_bar->addAction(QIcon(prop.icon_path), prop.name);
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

	/* Execute the command in the current context, the manager will push the
	* command on the undo stack. */
	m_command_manager->execute(cmd, m_context);
}

void MainWindow::addTimeLineWidget()
{
	auto dock = new QDockWidget("Time Line", this);
	dock->setAttribute(Qt::WA_DeleteOnClose);

	TimeLineWidget *time_line = new TimeLineWidget(dock);
	time_line->listens(&m_context);
	time_line->update_state(event_type::time | event_type::modified);

	dock->setWidget(time_line);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::addGraphEditorWidget()
{
	auto dock = new QDockWidget("Graph Editor", this);
	dock->setAttribute(Qt::WA_DeleteOnClose);

	QtNodeEditor *graph_editor = new QtNodeEditor(dock);
	graph_editor->listens(&m_context);
	graph_editor->setAddNodeMenu(m_add_nodes_menu);
	/* XXX - graph editor needs to be able to draw the scene from the scratch. */
	graph_editor->update_state(static_cast<event_type>(-1));

	dock->setWidget(graph_editor);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::addGLViewerWidget()
{
	/* TODO: figure out a way to have multiple GL context. */
	if (m_viewer_dock == nullptr) {
		m_viewer_dock = new QDockWidget("Viewport", this);

		ViewerWidget *glviewer = new ViewerWidget(m_viewer_dock);
		glviewer->listens(&m_context);
		glviewer->update_state(static_cast<event_type>(-1));

		m_viewer_dock->setWidget(glviewer);
		m_viewer_dock->setAllowedAreas(Qt::AllDockWidgetAreas);

		addDockWidget(Qt::LeftDockWidgetArea, m_viewer_dock);
	}

	m_viewer_dock->show();
}

void MainWindow::addOutlinerWidget()
{
	auto dock = new QDockWidget("Outliner", this);
	dock->setAttribute(Qt::WA_DeleteOnClose);

	OutlinerTreeWidget *outliner = new OutlinerTreeWidget(dock);
	outliner->listens(&m_context);
	outliner->update_state(event_type::object | event_type::added);

	dock->setWidget(outliner);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::addGraphOutlinerWidget()
{
	auto outliner_dock = new QDockWidget("Outliner", this);
	outliner_dock->setAttribute(Qt::WA_DeleteOnClose);

	OutlinerTreeWidget *outliner = new OutlinerTreeWidget(outliner_dock);
	outliner->listens(&m_context);
	outliner->update_state(event_type::object | event_type::added);

	outliner_dock->setWidget(outliner);
	outliner_dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, outliner_dock);

	auto graph_dock = new QDockWidget("Graph Editor", this);
	graph_dock->setAttribute(Qt::WA_DeleteOnClose);

	QtNodeEditor *graph_editor = new QtNodeEditor(graph_dock);
	graph_editor->listens(&m_context);
	graph_editor->setAddNodeMenu(m_add_nodes_menu);

	graph_dock->setWidget(graph_editor);
	graph_dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, graph_dock);

	tabifyDockWidget(graph_dock, outliner_dock);

	/* Make sure the graph editor visible by default. */
	graph_dock->raise();
}

void MainWindow::addPropertiesWidget()
{
	auto dock = new QDockWidget("Properties", this);
	dock->setAttribute(Qt::WA_DeleteOnClose);

	PropertiesWidget *properties = new PropertiesWidget(dock);
	properties->listens(&m_context);
	properties->update_state(static_cast<event_type>(-1));

	dock->setWidget(properties);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::dumpGraph()
{
	auto action = qobject_cast<QAction *>(sender());

	if (!action) {
		return;
	}

	auto data = action->data().toString();
	auto scene = m_context.scene;

	if (data == "dump_object_graph") {
		auto scene_node = scene->active_node();

		if (!scene_node) {
			return;
		}

		auto object = static_cast<Object *>(scene_node);

		GraphDumper gd(object->graph());
		gd("/tmp/object_graph.gv");

		if (system("dot /tmp/object_graph.gv -Tpng -o object_graph.png") == -1) {
			std::cerr << "Cannot create graph image from dot\n";
		}
	}
	else if (data == "dump_dependency_graph") {
		DepsGraphDumper gd(scene->depsgraph());
		gd("/tmp/depsgraph.gv");

		if (system("dot /tmp/depsgraph.gv -Tpng -o depsgraph.png") == -1) {
			std::cerr << "Cannot create graph image from dot\n";
		}
	}
}
