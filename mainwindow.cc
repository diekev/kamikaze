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

#include <openvdb/openvdb.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "objects/levelset.h"
#include "objects/volume.h"
#include "render/scene.h"
#include "render/viewer.h"
#include "util/util_openvdb.h"
#include "util/utils.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scene(new Scene)
    , m_viewer(new Viewer(this))
{
	qApp->installEventFilter(this);
	ui->setupUi(this);

	m_viewer->setScene(m_scene);

	setCentralWidget(m_viewer);
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

		if (grid->getGridClass() == GRID_LEVEL_SET) {
			LevelSet *ls = new LevelSet(grid);
			m_scene->add_level_set(ls);
		}
		else {
			Volume *volume = new Volume(grid);
			m_scene->add_volume(volume);
		}
	}
	else {
		std::cerr << "Unable to open file \'" << filename.toStdString() << "\'\n";
	}
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::KeyPress) {
		if (obj == m_viewer) {
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
			m_viewer->keyPressEvent(keyEvent);
			return true;
		}
	}
	else if (e->type() == QEvent::MouseButtonPress) {
		if (obj == m_viewer) {
			QMouseEvent *event = static_cast<QMouseEvent *>(e);
			m_viewer->mousePressEvent(event);
			return true;
		}
	}
	else if (e->type() == QEvent::MouseMove) {
		if (obj == m_viewer) {
			QMouseEvent *event = static_cast<QMouseEvent *>(e);
			m_viewer->mouseMoveEvent(event);
			return true;
		}
	}
	else if (e->type() == QEvent::MouseButtonRelease) {
		if (obj == m_viewer) {
			QMouseEvent *event = static_cast<QMouseEvent *>(e);
			m_viewer->mouseReleaseEvent(event);
			return true;
		}
	}
	else if (e->type() == QEvent::Wheel) {
		if (obj == m_viewer) {
			QWheelEvent *event = static_cast<QWheelEvent *>(e);
			m_viewer->wheelEvent(event);
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
