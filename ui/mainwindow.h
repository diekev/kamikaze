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

#pragma once

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class Scene;
class QTimer;

class MainWindow : public QMainWindow {
	Q_OBJECT

	Ui::MainWindow *ui;
	Scene *m_scene;
	QTimer *m_timer;
	bool m_timer_has_started;

private Q_SLOTS:
	void openFile();
	void updateObject();
	void updateObjectTab();
	void addCube();
	void addLevelSetSphere();
	void startAnimation();
	void updateFrame();

protected:
	bool eventFilter(QObject *obj, QEvent *e);

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void openFile(const QString &filename);
};
