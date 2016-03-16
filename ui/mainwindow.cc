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

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QKeyEvent>
#include <QListWidget>
#include <QSplitter>
#include <QTimer>

#include "objects/context.h"
#include "objects/object_ops.h"
#include "objects/undo.h"
#include "objects/volumebase.h"

#include "render/scene.h"

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

	/* Object */
	ui->m_move_object->setMinMax(-9999.99f, 9999.99f);
	ui->m_scale_object->setMinMax(-99.99f, 99.99f);
	ui->m_rotate_object->setMinMax(-360.0f, 360.0f);
	connectObjectSignals();

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

	/* Smoke Simulation. */
	connect(ui->m_time_step, SIGNAL(valueChanged(double)), m_scene, SLOT(setSimulationDt(double)));
	connect(ui->m_cache_path, SIGNAL(textChanged(QString)), m_scene, SLOT(setSimulationCache(QString)));
	connect(ui->m_advection, SIGNAL(currentIndexChanged(int)), m_scene, SLOT(setSimulationAdvection(int)));

	/* Timeline */
	ui->m_timeline->setMaximum(250);

	/* TODO: find another place to do this */
	registerCommandType("Add Object", AddObjectCmd::registerSelf);
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_command_manager;
	delete m_command_factory;
}

void MainWindow::openFile(const QString &filename) const
{
	m_command_manager->execute(new LoadFromFileCmd(m_scene, filename), nullptr);
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

void MainWindow::updateObject() const
{
	Object *ob = m_scene->currentObject();

	if (ob == nullptr) {
		return;
	}

	ob->drawBBox(ui->m_draw_bbox->isChecked());
	ob->drawTreeTopology(ui->m_draw_tree->isChecked());
}

void MainWindow::updateObjectTab() const
{
	Object *ob = m_scene->currentObject();

	if (ob == nullptr) {
		ui->tabWidget->setTabEnabled(0, false);
		return;
	}

	/* Disconnect signals to avoid unnecessary updates which could lead to
	 * crashes in some cases. */
	disconnectObjectSignals();

	ui->tabWidget->setTabEnabled(0, true);

	ui->m_object_name->setText(ob->name());

	ui->m_draw_bbox->setChecked(ob->drawBBox());

	ui->m_move_object->setValue(&ob->pos()[0]);
	ui->m_scale_object->setValue(&ob->scale()[0]);
	ui->m_rotate_object->setValue(&ob->rotation()[0]);

	const bool is_volume = ob->type() == VOLUME;
	const bool is_level_set = ob->type() == LEVEL_SET;
	const bool is_volume_base = is_volume || is_level_set;

	if (is_volume_base) {
		VolumeBase *vb = static_cast<VolumeBase *>(ob);

		ui->m_voxel_size->setValue(vb->voxelSize());
		ui->m_draw_tree->setChecked(vb->drawTreeTopology());
	}
	else {
		ui->m_draw_tree->setChecked(false);
		ui->m_voxel_size->setValue(0.0f);
	}

	ui->m_voxel_size->setEnabled(is_volume_base);
	ui->m_draw_tree->setEnabled(is_volume_base);
	ui->m_use_lut->setEnabled(is_volume);
	ui->m_num_slices->setEnabled(is_volume);

	if (is_level_set) {
		enableListItem(m_scene_mode_list, 1);
	}
	else {
		disableListItem(m_scene_mode_list, 1);
	}

	m_scene->objectNameList(ui->m_outliner);

	/* Reconnect signals. */
	connectObjectSignals();
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

void MainWindow::connectObjectSignals() const
{
	connect(ui->m_move_object, SIGNAL(valueChanged(double, int)), m_scene, SLOT(moveObject(double, int)));
	connect(ui->m_scale_object, SIGNAL(valueChanged(double, int)), m_scene, SLOT(scaleObject(double, int)));
	connect(ui->m_rotate_object, SIGNAL(valueChanged(double, int)), m_scene, SLOT(rotateObject(double, int)));
	connect(ui->m_voxel_size, SIGNAL(valueChanged(double)), m_scene, SLOT(setVoxelSize(double)));
	connect(ui->m_object_name, SIGNAL(textChanged(QString)), m_scene, SLOT(setObjectName(QString)));
	connect(ui->m_outliner, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
	        m_scene, SLOT(setCurrentObject(QListWidgetItem*)));
	connect(ui->m_use_lut, SIGNAL(clicked(bool)), m_scene, SLOT(setVolumeLUT(bool)));
	connect(ui->m_num_slices, SIGNAL(valueChanged(int)), m_scene, SLOT(setVolumeSlices(int)));
}

void MainWindow::disconnectObjectSignals() const
{
	disconnect(ui->m_move_object, SIGNAL(valueChanged(double, int)), m_scene, SLOT(moveObject(double, int)));
	disconnect(ui->m_scale_object, SIGNAL(valueChanged(double, int)), m_scene, SLOT(scaleObject(double, int)));
	disconnect(ui->m_rotate_object, SIGNAL(valueChanged(double, int)), m_scene, SLOT(rotateObject(double, int)));
	disconnect(ui->m_voxel_size, SIGNAL(valueChanged(double)), m_scene, SLOT(setVoxelSize(double)));
	disconnect(ui->m_object_name, SIGNAL(textChanged(QString)), m_scene, SLOT(setObjectName(QString)));
	disconnect(ui->m_outliner, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
	           m_scene, SLOT(setCurrentObject(QListWidgetItem*)));
	disconnect(ui->m_use_lut, SIGNAL(clicked(bool)), m_scene, SLOT(setVolumeLUT(bool)));
	disconnect(ui->m_num_slices, SIGNAL(valueChanged(int)), m_scene, SLOT(setVolumeSlices(int)));
}

void MainWindow::registerCommandType(const char *name, CommandFactory::factory_func func)
{
	auto action = ui->menuAdd->addAction(name);
	m_command_factory->registerType(name, func);
	connect(action, SIGNAL(triggered()), this, SLOT(handleCommand()));
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
	cmd->setUIParams(cb);

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
