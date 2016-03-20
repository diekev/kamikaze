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
#include <kamikaze/object.h>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QKeyEvent>
#include <QListWidget>
#include <QSplitter>
#include <QTimer>

#include "core/dynamiclibrary.h"
#include "core/filesystem.h"
#include "core/object_ops.h"
#include "core/scene.h"
#include "core/undo.h"

#include "paramcallback.h"
#include "ui_mainwindow.h"

static void disableListItem(QListWidget *list, int index)
{
	QListWidgetItem *item = list->item(index);
	item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
}

static void enableListItem(QListWidget *list, int index)
{
	QListWidgetItem *item = list->item(index);
	item->setFlags(item->flags() | Qt::ItemIsEnabled);
}

static void clear_layout(QLayout *layout)
{
	QLayoutItem *item;

	while ((item = layout->takeAt(0)) != nullptr) {
		if (item->layout()) {
			clear_layout(item->layout());
			delete item->layout();
		}

		if (item->widget()) {
			delete item->widget();
		}

		delete item;
	}
}

namespace fs = filesystem;
using PluginVec = std::vector<fs::shared_library>;

PluginVec load_plugins(const std::string &path)
{
	PluginVec plugins;
	fs::dir dir(path);

	for (const auto &file : dir) {
		if (!fs::is_library(file)) {
			continue;
		}

		fs::shared_library lib(file);

		if (!lib) {
			std::cerr << lib.error() << '\n';
			continue;
		}

		plugins.push_back(std::move(lib));
	}

	return plugins;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scene(new Scene)
    , m_timer(new QTimer(this))
    , m_command_manager(new CommandManager)
    , m_command_factory(new CommandFactory)
    , m_timer_has_started(false)
    , m_scene_mode_box(new QComboBox(this))
    , m_scene_mode_list(new QListWidget(m_scene_mode_box))
    , m_object_factory(new ObjectFactory)
{
	qApp->installEventFilter(this);
	ui->setupUi(this);
	ui->m_viewport->setScene(m_scene);

	connect(m_scene, SIGNAL(objectChanged()), this, SLOT(updateObjectTab()));
	ui->tabWidget->setTabEnabled(0, false);
	ui->tabWidget->setTabEnabled(3, false);

	/* set default strecthes for the splitters */
	ui->splitter->setStretchFactor(0, 1);
	ui->splitter->setStretchFactor(1, 0);

	ui->vsplitter->setStretchFactor(0, 1);
	ui->vsplitter->setStretchFactor(1, 0);

	/* Brush */
	connect(ui->m_brush_strength, SIGNAL(valueChanged(double)), m_scene, SLOT(setBrushStrength(double)));
	connect(ui->m_brush_radius, SIGNAL(valueChanged(double)), m_scene, SLOT(setBrushRadius(double)));
	connect(ui->m_brush_mode, SIGNAL(currentIndexChanged(int)), m_scene, SLOT(setBrushMode(int)));
	connect(ui->m_brush_tool, SIGNAL(currentIndexChanged(int)), m_scene, SLOT(setBrushTool(int)));

	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateFrame()));

	/* Cannot add widget to toolbar in Qt Designer, do a little hack to disable
	 * items in the box */
	ui->toolBar->addWidget(m_scene_mode_box);

	m_scene_mode_list->hide();
	m_scene_mode_box->setModel(m_scene_mode_list->model());

	m_scene_mode_list->addItem("Object Mode");
	m_scene_mode_list->addItem("Sculpt Mode");

	disableListItem(m_scene_mode_list, 1);

	connect(m_scene_mode_box, SIGNAL(currentIndexChanged(int)), this, SLOT(setSceneMode(int)));

#if 0
	/* Smoke Simulation. */
	connect(ui->m_time_step, SIGNAL(valueChanged(double)), m_scene, SLOT(setSimulationDt(double)));
	connect(ui->m_cache_path, SIGNAL(textChanged(QString)), m_scene, SLOT(setSimulationCache(QString)));
	connect(ui->m_advection, SIGNAL(currentIndexChanged(int)), m_scene, SLOT(setSimulationAdvection(int)));
#endif

	/* Timeline */
	ui->m_timeline->setMaximum(250);

	/* TODO: find another place to do this */
	registerObjectType();
	generateObjectMenu();
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_command_manager;
	delete m_command_factory;
	delete m_object_factory;
}

void MainWindow::openFile(const QString &filename) const
{
#if 0
	m_command_manager->execute(new LoadFromFileCmd(m_scene, filename), nullptr);
#endif
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

void MainWindow::openFile()
{
	const auto &filename = QFileDialog::getOpenFileName(
	                          this, tr("Open File"),
		                      QDir::homePath(),
		                      tr("*.vdb"));

	if (!filename.isEmpty()) {
		openFile(filename);
	}
}

void MainWindow::updateObjectTab() const
{
	Object *ob = m_scene->currentObject();

	if (ob == nullptr) {
		ui->tabWidget->setTabEnabled(0, false);
		return;
	}

	ui->tabWidget->setTabEnabled(0, true);

	clear_layout(ui->m_object_layout);

	ParamCallback cb(ui->m_object_layout);
	ob->setUIParams(&cb);
	ob->setCustomUIParams(&cb);

	cb.setContext(m_scene, SLOT(tagObjectUpdate()));

	if ((ob->flags() & object_flags::object_supports_sculpt) != object_flags::object_flags_none) {
		enableListItem(m_scene_mode_list, 1);
	}
	else {
		disableListItem(m_scene_mode_list, 1);
	}

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
}

void MainWindow::setStartFrame(int value) const
{
	ui->m_timeline->setMinimum(value);
}

void MainWindow::setEndFrame(int value) const
{
	ui->m_timeline->setMaximum(value);
}

void MainWindow::setSceneMode(int idx) const
{
	m_scene->setMode(idx);
	ui->tabWidget->setTabEnabled(3, idx == SCENE_MODE_SCULPT);
}

void MainWindow::registerCommandType(const char *name, CommandFactory::factory_func func)
{
	auto action = ui->menuAdd->addAction(name);
	m_command_factory->registerType(name, func);
	connect(action, SIGNAL(triggered()), this, SLOT(handleCommand()));
}

typedef void (*register_func_t)(ObjectFactory *);

void MainWindow::registerObjectType()
{
	PluginVec plugins = load_plugins("/opt/kamikaze/");

	for (const auto &plugin : plugins) {
		auto register_figures = plugin.symbol<register_func_t>("new_kamikaze_objects");

		if (register_figures != nullptr) {
			register_figures(m_object_factory);
		}
	}
}

void MainWindow::generateObjectMenu()
{
	std::vector<std::string> keys = m_object_factory->keys();

	for (const auto &key : keys) {
		auto action = ui->menuAdd->addAction(key.c_str());
		connect(action, SIGNAL(triggered()), this, SLOT(handleObjectCommand()));
	}
}

void MainWindow::handleCommand()
{
	QAction *action = qobject_cast<QAction *>(sender());

	if (!action) {
		return;
	}

	const auto &name = action->text();

	/* create UI */
	QDialog *dialog = new QDialog();
	dialog->setPalette(this->palette());
	dialog->setWindowTitle(name);

	QDialogButtonBox *button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(button_box, SIGNAL(accepted()), dialog, SLOT(accept()));
	connect(button_box, SIGNAL(rejected()), dialog, SLOT(reject()));

	QGridLayout *layout = new QGridLayout();

	dialog->setLayout(layout);

	ParamCallback cb(layout);

	/* get command */
	Command *cmd = (*m_command_factory)(name.toStdString());
	cmd->setUIParams(&cb);

	layout->addWidget(button_box);

	/* TODO */
	EvaluationContext context;
	context.scene = m_scene;

	if (dialog->exec() == QDialog::Accepted) {
		m_command_manager->execute(cmd, &context);
	}
	else {
		delete cmd;
	}

	delete layout;
	delete dialog;
}

void MainWindow::handleObjectCommand()
{
	QAction *action = qobject_cast<QAction *>(sender());

	if (!action) {
		return;
	}

	const auto &name = action->text();

	/* get command */
	Command *cmd = new AddObjectCmd(name);

	/* TODO */
	EvaluationContext context;
	context.scene = m_scene;
	context.object_factory = m_object_factory;

	m_command_manager->execute(cmd, &context);
}
