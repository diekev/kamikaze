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

#include <kamikaze/context.h>
#include "core/context.h"

#include "core/undo.h"

class Main;
class QProgressBar;
class RepondantCommande;

class MainWindow : public QMainWindow {
	Q_OBJECT

	QProgressBar *m_progress_bar;

	Main *m_main;

	QTimer *m_timer;
	CommandManager *m_command_manager;
	CommandFactory *m_command_factory;

	bool m_has_glwindow;

	Context m_context;
	EvaluationContext m_eval_context;

	QMenu *m_add_object_menu;
	QMenu *m_add_nodes_menu;
	QMenu *m_edit_menu;
	QMenu *m_add_window_menu;

	QToolBar *m_tool_bar;

	QDockWidget *m_viewer_dock = nullptr;

	std::vector<QAction *> m_actions_menu_recent;
	std::vector<QString> m_fichiers_recent = {};

	RepondantCommande *m_repondant_commande;

public:
	explicit MainWindow(Main *main, QWidget *parent = nullptr);
	~MainWindow();

public Q_SLOTS:
	/* Progress Bar */
	void taskStarted();
	void updateProgress(float progress);
	void taskEnded();
	void nodeProcessed();

private:
	void genere_menu_fichier();
	void generateWindowMenu();
	void generateEditMenu();
	void generateObjectMenu();
	void generateNodeMenu();
	void generatePresetMenu();
	void generateDebugMenu();

	void ajoute_fichier_recent(const QString &name, bool update_menu);
	void mis_a_jour_menu_fichier_recent();

	void ecrit_reglages() const;
	void charge_reglages();

	void closeEvent(QCloseEvent *) override;

	void ouvre_fichier_implementation(const std::string &chemin_projet);

private Q_SLOTS:
	/* Undo & commands */
	void undo() const;
	void redo() const;
	void handleCommand();

	void ouvre_fichier();
	void ouvre_fichier_recent();
	void sauve_fichier();
	void sauve_fichier_sous();

	void addTimeLineWidget();
	void addGraphEditorWidget();
	void addGraphOutlinerWidget();
	void addGLViewerWidget();
	void addOutlinerWidget();
	void addPropertiesWidget();

	void dumpGraph();
};
