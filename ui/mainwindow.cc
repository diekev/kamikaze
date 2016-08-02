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
	m_context.edit_mode = false;
	m_context.animation = false;
	m_context.scene = m_main->scene();
	m_context.node_factory = m_main->nodeFactory();
	m_context.primitive_factory = m_main->primitiveFactory();
	m_context.main_window = this;

	m_has_glwindow = false;

	addGLViewerWidget();
	addPropertiesWidget();
	addGraphOutlinerWidget();
	addTimeLineWidget();

	setCentralWidget(nullptr);

	setupPalette();
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
	m_command_factory->registerType("add object", AddObjectCmd::registerSelf);

	m_add_object_menu = menuBar()->addMenu("Add Object");
	auto action = m_add_object_menu->addAction("Empty Object");
	action->setData(QVariant::fromValue(QString("add object")));

	connect(action, SIGNAL(triggered()), this, SLOT(handleCommand()));
}

void MainWindow::generateDebugMenu()
{
	m_add_object_menu = menuBar()->addMenu("Debug");
	auto action = m_add_object_menu->addAction("Dump Object Graph");
	action->setData(QVariant::fromValue(QString("dump_object_graph")));

	connect(action, SIGNAL(triggered()), this, SLOT(dumpGraph()));

	action = m_add_object_menu->addAction("Dump Dependency Graph");
	action->setData(QVariant::fromValue(QString("dump_dependency_graph")));

	connect(action, SIGNAL(triggered()), this, SLOT(dumpGraph()));
}

void MainWindow::generateNodeMenu()
{
	m_command_factory->registerType("add node", AddNodeCmd::registerSelf);
	m_add_nodes_menu = menuBar()->addMenu("Add Node");

	for (const auto &category : m_main->nodeFactory()->categories()) {
		auto sub_menu = m_add_nodes_menu->addMenu(category.c_str());

		for (const auto &key : m_main->nodeFactory()->keys(category)) {
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

	action = m_add_window_menu->addAction("3D View");
	action->setToolTip("Add a 3D View");
	connect(action, SIGNAL(triggered()), this, SLOT(addGLViewerWidget()));
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
	m_command_factory->registerType("add preset", AddPresetObjectCmd::registerSelf);

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
	m_command_manager->execute(cmd, &m_context);
}

void MainWindow::addTimeLineWidget()
{
	QDockWidget *dock = new QDockWidget("Time Line", this);
	dock->setAttribute(Qt::WA_DeleteOnClose);

	TimeLineWidget *time_line = new TimeLineWidget(dock);
	time_line->listens(&m_context);
	time_line->update_state(TIME_CHANGED);

	dock->setWidget(time_line);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::addGraphEditorWidget()
{
	QDockWidget *dock = new QDockWidget("Graph Editor", this);
	dock->setAttribute(Qt::WA_DeleteOnClose);

	QtNodeEditor *graph_editor = new QtNodeEditor(dock);
	graph_editor->listens(&m_context);
	graph_editor->setAddNodeMenu(m_add_nodes_menu);
	/* XXX - graph editor needs to be able to draw the scene from the scratch. */
	graph_editor->update_state(-1);

	dock->setWidget(graph_editor);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::addGLViewerWidget()
{
	/* TODO: figure out a way to have multiple GL context. */
	if (m_viewer_dock == nullptr) {
		m_viewer_dock = new QDockWidget("Viewport", this);

		Viewer *glviewer = new Viewer(m_viewer_dock);
		glviewer->listens(&m_context);
		glviewer->update_state(-1);

		m_viewer_dock->setWidget(glviewer);
		m_viewer_dock->setAllowedAreas(Qt::AllDockWidgetAreas);

		addDockWidget(Qt::LeftDockWidgetArea, m_viewer_dock);
	}

	m_viewer_dock->show();
}

void MainWindow::addOutlinerWidget()
{
	QDockWidget *dock = new QDockWidget("Outliner", this);
	dock->setAttribute(Qt::WA_DeleteOnClose);

	OutlinerTreeWidget *outliner = new OutlinerTreeWidget(dock);
	outliner->listens(&m_context);
	/* XXX - outliner needs to be able to draw the scene from the scratch. */
	outliner->update_state(OBJECT_ADDED);

	dock->setWidget(outliner);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::addGraphOutlinerWidget()
{
	QDockWidget *outliner_dock = new QDockWidget("Outliner", this);
	outliner_dock->setAttribute(Qt::WA_DeleteOnClose);

	OutlinerTreeWidget *outliner = new OutlinerTreeWidget(outliner_dock);
	outliner->listens(&m_context);

	outliner_dock->setWidget(outliner);
	outliner_dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, outliner_dock);

	QDockWidget *graph_dock = new QDockWidget("Graph Editor", this);
	graph_dock->setAttribute(Qt::WA_DeleteOnClose);

	QtNodeEditor *graph_editor = new QtNodeEditor(graph_dock);
	graph_editor->listens(&m_context);
	graph_editor->setAddNodeMenu(m_add_nodes_menu);

	graph_dock->setWidget(graph_editor);
	graph_dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, graph_dock);

	tabifyDockWidget(graph_dock, outliner_dock);
}

void MainWindow::addPropertiesWidget()
{
	QDockWidget *dock = new QDockWidget("Properties", this);
	dock->setAttribute(Qt::WA_DeleteOnClose);

	PropertiesWidget *properties = new PropertiesWidget(dock);
	properties->listens(&m_context);
	properties->update_state(-1);

	dock->setWidget(properties);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::setupPalette()
{
	QPalette palette;
	QBrush brush(QColor(255, 255, 255, 255));
	brush.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
	QBrush brush1(QColor(127, 127, 127, 255));
	brush1.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::Button, brush1);
	QBrush brush2(QColor(191, 191, 191, 255));
	brush2.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::Light, brush2);
	QBrush brush3(QColor(159, 159, 159, 255));
	brush3.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::Midlight, brush3);
	QBrush brush4(QColor(63, 63, 63, 255));
	brush4.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::Dark, brush4);
	QBrush brush5(QColor(84, 84, 84, 255));
	brush5.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::Mid, brush5);
	palette.setBrush(QPalette::Active, QPalette::Text, brush);
	palette.setBrush(QPalette::Active, QPalette::BrightText, brush);
	palette.setBrush(QPalette::Active, QPalette::ButtonText, brush);
	QBrush brush6(QColor(0, 0, 0, 255));
	brush6.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::Base, brush6);
	palette.setBrush(QPalette::Active, QPalette::Window, brush1);
	palette.setBrush(QPalette::Active, QPalette::Shadow, brush6);
	palette.setBrush(QPalette::Active, QPalette::AlternateBase, brush4);
	QBrush brush7(QColor(255, 255, 220, 255));
	brush7.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::ToolTipBase, brush7);
	palette.setBrush(QPalette::Active, QPalette::ToolTipText, brush6);
	palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
	palette.setBrush(QPalette::Inactive, QPalette::Button, brush1);
	palette.setBrush(QPalette::Inactive, QPalette::Light, brush2);
	palette.setBrush(QPalette::Inactive, QPalette::Midlight, brush3);
	palette.setBrush(QPalette::Inactive, QPalette::Dark, brush4);
	palette.setBrush(QPalette::Inactive, QPalette::Mid, brush5);
	palette.setBrush(QPalette::Inactive, QPalette::Text, brush);
	palette.setBrush(QPalette::Inactive, QPalette::BrightText, brush);
	palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
	palette.setBrush(QPalette::Inactive, QPalette::Base, brush6);
	palette.setBrush(QPalette::Inactive, QPalette::Window, brush1);
	palette.setBrush(QPalette::Inactive, QPalette::Shadow, brush6);
	palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush4);
	palette.setBrush(QPalette::Inactive, QPalette::ToolTipBase, brush7);
	palette.setBrush(QPalette::Inactive, QPalette::ToolTipText, brush6);
	palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush4);
	palette.setBrush(QPalette::Disabled, QPalette::Button, brush1);
	palette.setBrush(QPalette::Disabled, QPalette::Light, brush2);
	palette.setBrush(QPalette::Disabled, QPalette::Midlight, brush3);
	palette.setBrush(QPalette::Disabled, QPalette::Dark, brush4);
	palette.setBrush(QPalette::Disabled, QPalette::Mid, brush5);
	palette.setBrush(QPalette::Disabled, QPalette::Text, brush4);
	palette.setBrush(QPalette::Disabled, QPalette::BrightText, brush);
	palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush4);
	palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
	palette.setBrush(QPalette::Disabled, QPalette::Window, brush1);
	palette.setBrush(QPalette::Disabled, QPalette::Shadow, brush6);
	palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush1);
	palette.setBrush(QPalette::Disabled, QPalette::ToolTipBase, brush7);
	palette.setBrush(QPalette::Disabled, QPalette::ToolTipText, brush6);

	setPalette(palette);
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
		auto object = scene->currentObject();

		if (!object) {
			return;
		}

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
