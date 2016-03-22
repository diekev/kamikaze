﻿/*
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
#include <kamikaze/modifiers.h>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QKeyEvent>
#include <QListWidget>
#include <QSplitter>
#include <QTimer>

#include "core/kamikaze_main.h"
#include "core/object_ops.h"
#include "core/scene.h"
#include "core/undo.h"

#include "modifieritem.h"
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
    , m_scene_mode_box(new QComboBox(this))
    , m_scene_mode_list(new QListWidget(m_scene_mode_box))
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

	disable_list_item(m_scene_mode_list, 1);

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
	generateObjectMenu();
	generateModifiersMenu();
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_command_manager;
	delete m_command_factory;
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
		enable_list_item(m_scene_mode_list, 1);
	}
	else {
		disable_list_item(m_scene_mode_list, 1);
	}

	updateModifiersTab();

	m_scene->objectNameList(ui->m_outliner);
}

void MainWindow::updateModifiersTab() const
{
	Object *ob = m_scene->currentObject();

	if (ob == nullptr) {
		return;
	}

	clear_layout(ui->modifiers_tab->layout());

	for (const auto &modifier : ob->modifiers()) {
		auto item = new ModifierItem(modifier->name().c_str());

		ParamCallback modcb(item->layout());
		modifier->setUIParams(&modcb);
		modcb.setContext(m_scene, SLOT(evalObjectModifiers()));

		ui->modifiers_tab->layout()->addWidget(item);
	}
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

void MainWindow::generateObjectMenu()
{
	m_command_factory->registerType("add object", AddObjectCmd::registerSelf);

	for (const auto &key : m_main->objectFactory()->keys()) {
		auto action = ui->menuAdd->addAction(key.c_str());
		action->setData(QVariant::fromValue(QString("add object")));

		connect(action, SIGNAL(triggered()), this, SLOT(handleObjectCommand()));
	}
}

void MainWindow::generateModifiersMenu()
{
	m_command_factory->registerType("add modifier", AddModifierCmd::registerSelf);

	for (const auto &key : m_main->modifierFactory()->keys()) {
		auto action = ui->add_modifier_menu->addAction(key.c_str());
		action->setData(QVariant::fromValue(QString("add modifier")));

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

	const auto &name = action->text().toStdString();
	const auto &data = action->data().toString().toStdString();

	/* get command */
	Command *cmd = (*m_command_factory)(data);
	cmd->setName(name);

	/* TODO */
	EvaluationContext context;
	context.scene = m_scene;
	context.object_factory = m_main->objectFactory();
	context.modifier_factory = m_main->modifierFactory();

	m_command_manager->execute(cmd, &context);

	/* TODO */
	updateModifiersTab();
}
