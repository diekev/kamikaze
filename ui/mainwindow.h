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
#include <QProgressBar>

#include <kamikaze/context.h>

#include "core/undo.h"

namespace Ui {
class MainWindow;
}

class QtNode;

class Command;
class CommandManager;
class Main;
class Node;
class Object;
class ObjectNodeItem;
class Primitive;
class QComboBox;
class QListWidget;
class QProgressBar;
class QTimer;
class Scene;

class MainWindow : public QMainWindow {
	Q_OBJECT

	Ui::MainWindow *ui;
	QProgressBar *m_progress_bar;

	Main *m_main;

	Scene *m_scene;
	QTimer *m_timer;
	CommandManager *m_command_manager;
	CommandFactory *m_command_factory;
	bool m_timer_has_started;

	EvaluationContext m_context;

public:
	explicit MainWindow(Main *main, QWidget *parent = nullptr);
	~MainWindow();

public Q_SLOTS:
	/* Progress Bar */
	void taskStarted();
	void updateProgress(float progress);
	void taskEnded();

protected:
	bool eventFilter(QObject *obj, QEvent *e);

private:
	void generateObjectMenu();
	void generateNodeMenu();
	void generatePresetMenu();

private Q_SLOTS:
	/* Timeline */
	void startAnimation();
	void updateFrame() const;
	void setStartFrame(int value) const;
	void setEndFrame(int value) const;
	void goToStartFrame() const;
	void goToEndFrame() const;

	/* Undo & commands */
	void undo() const;
	void redo() const;
	void handleCommand();

	/* Generic object actions */
	void setupObjectUI(Object *);
	void updateObjectTab() const;

	/* Nodes */
	void setupNodeUI(Object *, Node *node);
	void setupNodeParamUI(QtNode *node_item);
	void setActiveObject(ObjectNodeItem *);
	void removeObject(ObjectNodeItem *node);
	void removeNode(QtNode *node);
	void nodesConnected(QtNode *, const QString &, QtNode *, const QString &);
	void connectionRemoved(QtNode *, const QString &, QtNode *, const QString &);
	void evalObjectGraph();
};
