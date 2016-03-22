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

#include "core/undo.h"

namespace Ui {
class MainWindow;
}

class Command;
class CommandManager;
class Main;
class QComboBox;
class QListWidget;
class QTimer;
class Scene;

class MainWindow : public QMainWindow {
	Q_OBJECT

	Ui::MainWindow *ui;

	Main *m_main;

	Scene *m_scene;
	QTimer *m_timer;
	CommandManager *m_command_manager;
	CommandFactory *m_command_factory;
	bool m_timer_has_started;
	QComboBox *m_scene_mode_box;
	QListWidget *m_scene_mode_list;

public:
	explicit MainWindow(Main *main, QWidget *parent = nullptr);
	~MainWindow();

protected:
	bool eventFilter(QObject *obj, QEvent *e);

private:
	void generateObjectMenu();
	void generateModifiersMenu();

private Q_SLOTS:
	void updateObjectTab() const;
	void updateModifiersTab() const;
	void startAnimation();
	void updateFrame() const;
	void setStartFrame(int value) const;
	void setEndFrame(int value) const;
	void setSceneMode(int idx) const;
	void goToStartFrame() const;
	void goToEndFrame() const;
	void undo() const;
	void redo() const;
	void handleCommand();
	void handleObjectCommand();
};
