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

#include <QFileDialog>
#include <QKeyEvent>
#include <QSplitter>
#include <QTimer>

#include <openvdb/tools/LevelSetSphere.h>

#include "mainwindow.h"

#include "objects/levelset.h"
#include "objects/volume.h"

#include "render/scene.h"
#include "render/viewer.h"

#include "util/utils.h"
#include "util/util_openvdb.h"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scene(new Scene)
    , m_timer(new QTimer(this))
    , m_timer_has_started(false)
{
	qApp->installEventFilter(this);
	ui->setupUi(this);
	ui->m_viewport->setScene(m_scene);

	connect(m_scene, SIGNAL(objectChanged()), this, SLOT(updateObjectTab()));
	ui->tabWidget->setTabEnabled(0, false);

	/* set default widths for the viewport and side panel in the horizontal splitter */
	const int width = ui->splitter->size().width();
	const int viewport_width = 0.8f * float(width);
	const int panel_width = width - viewport_width;
	QList<int> sizes;
	sizes << viewport_width << panel_width;
	ui->splitter->setSizes(sizes);

	/* set default heights for the timeline wigdet and the vertical splitter */
	const int height = ui->vsplitter->size().height();
	const int tiemeline_height = 0.1f * float(height);
	const int hsplister_height= height - tiemeline_height;
	QList<int> hsizes;
	hsizes << hsplister_height << tiemeline_height;
	ui->vsplitter->setSizes(hsizes);

	/* Object transform */
	connect(ui->m_move_x, SIGNAL(valueChanged(double)), m_scene, SLOT(moveObjectX(double)));
	connect(ui->m_move_y, SIGNAL(valueChanged(double)), m_scene, SLOT(moveObjectY(double)));
	connect(ui->m_move_z, SIGNAL(valueChanged(double)), m_scene, SLOT(moveObjectZ(double)));
	connect(ui->m_scale_x, SIGNAL(valueChanged(double)), m_scene, SLOT(scaleObjectX(double)));
	connect(ui->m_scale_y, SIGNAL(valueChanged(double)), m_scene, SLOT(scaleObjectY(double)));
	connect(ui->m_scale_z, SIGNAL(valueChanged(double)), m_scene, SLOT(scaleObjectZ(double)));
	connect(ui->m_rotate_x, SIGNAL(valueChanged(double)), m_scene, SLOT(rotateObjectX(double)));
	connect(ui->m_rotate_y, SIGNAL(valueChanged(double)), m_scene, SLOT(rotateObjectY(double)));
	connect(ui->m_rotate_z, SIGNAL(valueChanged(double)), m_scene, SLOT(rotateObjectZ(double)));

	connect(m_scene, SIGNAL(updateViewport()), ui->m_viewport, SLOT(update()));
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateFrame()));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::openFile(const QString &filename)
{
	using namespace openvdb;

	initialize();
	io::File file(filename.toStdString());

	if (file.open()) {
		FloatGrid::Ptr grid;

		if (file.hasGrid(Name("Density"))) {
			grid = gridPtrCast<FloatGrid>(file.readGrid(Name("Density")));
		}
		else if (file.hasGrid(Name("density"))) {
			grid = gridPtrCast<FloatGrid>(file.readGrid(Name("density")));
		}
		else {
			GridPtrVecPtr grids = file.getGrids();
			grid = gridPtrCast<FloatGrid>((*grids)[0]);
//			os << "No density grid found in file: \'" << filename << "\'!\n";
//			return false;
		}

		auto meta_map = file.getMetadata();

		file.close();

		if ((*meta_map)["creator"]) {
			auto creator = (*meta_map)["creator"]->str();

			/* If the grid comes from Blender (Z-up), rotate it so it is Y-up */
			if (creator == "Blender/OpenVDBWriter") {
				Timer("Transform Blender Grid");
				grid = transform_grid(*grid, Vec3s(-M_PI_2, 0.0f, 0.0f),
				                      Vec3s(1.0f), Vec3s(0.0f), Vec3s(0.0f));
			}
		}

		Object *ob;
		if (grid->getGridClass() == GRID_LEVEL_SET) {
			ob = new LevelSet(grid);
		}
		else {
			ob = new Volume(grid);
		}
		m_scene->addObject(ob);
	}
	else {
		std::cerr << "Unable to open file \'" << filename.toStdString() << "\'\n";
	}
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
	                          this, tr("Ouvrir fichier image"),
		                      QDir::homePath(),
		                      tr("*.vdb"));

	if (!filename.isEmpty()) {
		openFile(filename);
	}
}

void MainWindow::updateObject()
{
	Object *ob = m_scene->currentObject();

	if (ob == nullptr) {
		return;
	}

	ob->drawBBox(ui->m_draw_bbox->isChecked());
	ob->drawTreeTopology(ui->m_draw_tree->isChecked());

	ui->m_viewport->update();
}

void MainWindow::updateObjectTab()
{
	Object *ob = m_scene->currentObject();

	if (ob == nullptr) {
		return;
	}

	ui->tabWidget->setTabEnabled(0, true);

	ui->m_draw_bbox->setChecked(ob->drawBBox());
	ui->m_draw_tree->setChecked(ob->drawTreeTopology());

	const glm::vec3 pos = ob->pos();
	ui->m_move_x->setValue(pos.x);
	ui->m_move_y->setValue(pos.y);
	ui->m_move_z->setValue(pos.z);
	const glm::vec3 scale = ob->scale();
	ui->m_scale_x->setValue(scale.x);
	ui->m_scale_y->setValue(scale.y);
	ui->m_scale_z->setValue(scale.z);
	const glm::vec3 rotation = ob->rotation();
	ui->m_rotate_x->setValue(rotation.x);
	ui->m_rotate_y->setValue(rotation.y);
	ui->m_rotate_z->setValue(rotation.z);
}

void MainWindow::addCube()
{
	float radius = 2.0f;
	glm::vec3 min(-1.0f), max(1.0f);

	Object *ob = new Cube(min * radius, max * radius);
	m_scene->addObject(ob);
}

void MainWindow::addLevelSetSphere()
{
	using namespace openvdb;
	FloatGrid::Ptr sphere = tools::createLevelSetSphere<FloatGrid>(2.0f, Vec3f(0.0f), 0.1f);

	Object *ob = new LevelSet(sphere);
	m_scene->addObject(ob);
}

void MainWindow::startAnimation()
{
	if (m_timer_has_started) {
		m_timer_has_started = false;
		m_timer->stop();
		ui->m_start_but->setText("Play Animation");
	}
	else {
		m_timer_has_started = true;
		ui->m_start_but->setText("Pause Animation");
		m_timer->start(1000 / ui->m_fps->value());
	}
}

void MainWindow::updateFrame()
{
	ui->m_timeline->incrementFrame();
}
