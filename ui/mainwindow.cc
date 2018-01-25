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

#include <fstream>

#include <kamikaze/primitive.h>

#include <kangao/kangao.h>
#include <kangao/repondant_bouton.h>

#include <QDockWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenuBar>
#include <QProgressBar>
#include <QStatusBar>
#include <QSettings>
#include <QToolBar>

#include "core/graphs/graph_dumper.h"
#include "core/kamikaze_main.h"
#include "core/object.h"
#include "core/sauvegarde.h"

#include "node_editorwidget.h"
#include "outliner_widget.h"
#include "properties_widget.h"
#include "timeline_widget.h"
#include "utils_ui.h"
#include "viewer.h"

static const char *chemins_scripts[] = {
	"interface/menu_fichier.kangao",
	"interface/menu_objet.kangao",
	"interface/menu_debogage.kangao",
};

static constexpr auto MAX_FICHIER_RECENT = 10;

class RepondantCommande : public kangao::RepondantBouton {
	Main *m_main = nullptr;
	Context *m_contexte = nullptr;
	CommandFactory *m_usine_commandes = nullptr;
	CommandManager *m_gestionnaire_commandes = nullptr;

public:
	RepondantCommande(
			Main *main,
			Context *contexte,
			CommandFactory *usine_commande,
			CommandManager *gestionnaire)
		: m_main(main)
		, m_contexte(contexte)
		, m_usine_commandes(usine_commande)
		, m_gestionnaire_commandes(gestionnaire)
	{}

	void repond_clique(const std::string &identifiant, const std::string &metadonnee)
	{
		std::cerr << "Répond clique : " << identifiant << ", " << metadonnee << '\n';

		/* Get command, and give it the name of the UI button which will be used to
		 * look up keys in the various creation factories if need be. This could and
		 * should be handled better. */
		auto cmd = (*m_usine_commandes)(identifiant);
		cmd->setName(metadonnee);

		/* Execute the command in the current context, the manager will push the
		* command on the undo stack. */
		m_gestionnaire_commandes->execute(m_main, cmd, *m_contexte);
	}
};

MainWindow::MainWindow(Main *main, QWidget *parent)
    : QMainWindow(parent)
    , m_main(main)
    , m_command_manager(new CommandManager)
    , m_command_factory(new CommandFactory)
	, m_repondant_commande(new RepondantCommande(main, &m_context, m_command_factory, m_command_manager))
{
	genere_menu_fichier();
	generateNodeMenu();
	generateWindowMenu();
	generatePresetMenu();

	m_progress_bar = new QProgressBar(this);
	statusBar()->addWidget(m_progress_bar);
	m_progress_bar->setRange(0, 100);
	m_progress_bar->setVisible(false);

	/* setup context */
	m_eval_context.edit_mode = false;
	m_eval_context.animation = false;
	m_context.eval_ctx = &m_eval_context;
	m_context.scene = m_main->scene();
	m_context.primitive_factory = m_main->primitive_factory();
	m_context.usine_operateur = m_main->usine_operateur();
	m_context.main_window = this;
	m_context.active_widget = nullptr;

	m_has_glwindow = false;

	addGLViewerWidget();
	addPropertiesWidget();
	addGraphOutlinerWidget();
	addTimeLineWidget();

	setCentralWidget(nullptr);

	charge_reglages();
}

MainWindow::~MainWindow()
{
	delete m_repondant_commande;
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

void MainWindow::nodeProcessed()
{
	m_context.scene->notify_listeners(event_type::node | event_type::processed);
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

void MainWindow::generateNodeMenu()
{
	m_add_nodes_menu = menuBar()->addMenu("Ajoute Noeud");

	auto categories = m_main->usine_operateur()->categories();

	for (const auto &categorie : categories) {
		auto menu = m_add_nodes_menu->addMenu(categorie.c_str());

		auto cles = m_main->usine_operateur()->cles(categorie);
		std::sort(cles.begin(), cles.end(),
				  [](const DescOperateur &desc1, const DescOperateur &desc2)
		{
			return desc1.nom < desc2.nom;
		});

		for (const auto &description : cles) {
			auto action = menu->addAction(description.nom.c_str());
			action->setData(QVariant::fromValue(QString("add node")));

			//connect(action, SIGNAL(triggered()), this, SLOT(handleCommand()));
		}
	}
}

void MainWindow::generateWindowMenu()
{
	m_add_window_menu = menuBar()->addMenu("View");

	QAction *action;

	action = m_add_window_menu->addAction("3D View");
	action->setToolTip("Add a 3D View");
	connect(action, SIGNAL(triggered()), this, SLOT(addGLViewerWidget()));

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

void MainWindow::genere_menu_fichier()
{
#if 0
	auto menu_fichier = menuBar()->addMenu("Fichier");

	QAction *action;

	action = menu_fichier->addAction("Ouvrir");
	connect(action, SIGNAL(triggered()), this, SLOT(ouvre_fichier()));

	auto sous_menu = menu_fichier->addMenu("Projets récents...");

	m_actions_menu_recent.resize(MAX_FICHIER_RECENT);

	for (auto &action_menu_recent : m_actions_menu_recent) {
		action_menu_recent = new QAction(this);
		action_menu_recent->setVisible(false);
		sous_menu->addAction(action_menu_recent);
		connect(action_menu_recent, SIGNAL(triggered()), this, SLOT(ouvre_fichier_recent()));
	}

	menu_fichier->addSeparator();

	action = menu_fichier->addAction("Sauvegarder");
	connect(action, SIGNAL(triggered()), this, SLOT(sauve_fichier()));

	action = menu_fichier->addAction("Sauvegarder sous...");
	connect(action, SIGNAL(triggered()), this, SLOT(sauve_fichier_sous()));
#else
	kangao::DonneesInterface donnees;
	donnees.manipulable = nullptr;
	donnees.conteneur = nullptr;
	donnees.repondant_bouton = m_repondant_commande;

	for (const auto &chemin : chemins_scripts) {
		std::ifstream entree;
		entree.open(chemin);

		std::string texte_entree;
		std::string temp;

		while (std::getline(entree, temp)) {
			texte_entree += temp;
		}

		auto menu = kangao::compile_menu(donnees, texte_entree.c_str());

		menuBar()->addMenu(menu);
	}
#endif
}

void MainWindow::generatePresetMenu()
{
	m_tool_bar = new QToolBar();
	addToolBar(Qt::TopToolBarArea, m_tool_bar);

	UIButData props[] = {
		{ 0, "Création grille", "icons/icon_grid.png" },
		{ 0, "Création boîte", "icons/icon_box.png" },
		{ 0, "Création cercle", "icons/icon_circle.png" },
		{ 0, "Création icosphère", "icons/icon_icosphere.png" },
		{ 0, "Création tube", "icons/icon_tube.png" },
		{ 0, "Création cone", "icons/icon_cone.png" },
		{ 0, "Création torus", "icons/icon_torus.png" },
		{ 0, "Création nuage point", "icons/icon_point_cloud_cube.png" },
	};

	for (const auto &prop : props) {
		auto action = m_tool_bar->addAction(QIcon(prop.icon_path), prop.name);
		action->setData(QVariant::fromValue(QString("add preset")));

		//connect(action, SIGNAL(triggered()), this, SLOT(handleCommand()));
	}
}

void MainWindow::ouvre_fichier_recent()
{
	auto action = qobject_cast<QAction *>(sender());

	if (action == nullptr) {
		return;
	}

	auto chemin_projet = action->data().toString().toStdString();
	ouvre_fichier_implementation(chemin_projet);
}

void MainWindow::ouvre_fichier_implementation(const std::string &chemin_projet)
{
	const auto erreur = kamikaze::ouvre_projet(chemin_projet, *m_main, m_context);

	if (erreur != kamikaze::erreur_fichier::AUCUNE_ERREUR) {
		QMessageBox boite_message;

		switch (erreur) {
			case kamikaze::erreur_fichier::CORROMPU:
				boite_message.critical(nullptr, "Error", "Le fichier est corrompu !");
				break;
			case kamikaze::erreur_fichier::NON_OUVERT:
				boite_message.critical(nullptr, "Error", "Le fichier n'est pas ouvert !");
				break;
			case kamikaze::erreur_fichier::NON_TROUVE:
				boite_message.critical(nullptr, "Error", "Le fichier n'a pas été trouvé !");
				break;
			case kamikaze::erreur_fichier::INCONNU:
				boite_message.critical(nullptr, "Error", "Erreur inconnu !");
				break;
			case kamikaze::erreur_fichier::GREFFON_MANQUANT:
				boite_message.critical(nullptr, "Error",
									   "Le fichier ne pas être ouvert car il"
									   " y a un greffon manquant !");
				break;
		}

		boite_message.setFixedSize(500, 200);
		return;
	}

	m_main->chemin_projet(chemin_projet);
	m_main->projet_ouvert(true);

	ajoute_fichier_recent(chemin_projet.c_str(), true);
	setWindowTitle(chemin_projet.c_str());
}

void MainWindow::ajoute_fichier_recent(const QString &name, bool update_menu)
{
	auto index = std::find(m_fichiers_recent.begin(), m_fichiers_recent.end(), name);

	if (index != m_fichiers_recent.end()) {
		std::rotate(m_fichiers_recent.begin(), index, index + 1);
	}
	else {
		m_fichiers_recent.insert(m_fichiers_recent.begin(), name);

		if (m_fichiers_recent.size() > MAX_FICHIER_RECENT) {
			m_fichiers_recent.resize(MAX_FICHIER_RECENT);
		}
	}

	if (update_menu) {
		mis_a_jour_menu_fichier_recent();
	}
}

void MainWindow::mis_a_jour_menu_fichier_recent()
{
	/* À FAIRE */
#if 0
	if (m_fichiers_recent.empty()) {
		return;
	}

	//ui->m_no_recent_act->setVisible(false);

	for (int i(0); i < m_fichiers_recent.size();  ++i) {
		auto filename = m_fichiers_recent[i];
		auto name = QFileInfo(filename).fileName();

		m_actions_menu_recent[i]->setText(name);
		m_actions_menu_recent[i]->setData(filename);
		m_actions_menu_recent[i]->setVisible(true);
	}
#endif
}

void MainWindow::addTimeLineWidget()
{
	auto dock = new QDockWidget("Time Line", this);
	dock->setAttribute(Qt::WA_DeleteOnClose);

	TimeLineWidget *time_line = new TimeLineWidget(dock);
	time_line->listens(&m_context);
	time_line->update_state(event_type::time | event_type::modified);

	dock->setWidget(time_line);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::addGraphEditorWidget()
{
	auto dock = new QDockWidget("Graph Editor", this);
	dock->setAttribute(Qt::WA_DeleteOnClose);

	QtNodeEditor *graph_editor = new QtNodeEditor(dock);
	graph_editor->listens(&m_context);
	graph_editor->setAddNodeMenu(m_add_nodes_menu);
	/* XXX - graph editor needs to be able to draw the scene from the scratch. */
	graph_editor->update_state(static_cast<event_type>(-1));

	dock->setWidget(graph_editor);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::addGLViewerWidget()
{
	/* TODO: figure out a way to have multiple GL context. */
	if (m_viewer_dock == nullptr) {
		m_viewer_dock = new QDockWidget("Viewport", this);

		ViewerWidget *glviewer = new ViewerWidget(m_viewer_dock);
		glviewer->listens(&m_context);
		glviewer->update_state(static_cast<event_type>(-1));

		m_viewer_dock->setWidget(glviewer);
		m_viewer_dock->setAllowedAreas(Qt::AllDockWidgetAreas);

		addDockWidget(Qt::LeftDockWidgetArea, m_viewer_dock);
	}

	m_viewer_dock->show();
}

void MainWindow::addOutlinerWidget()
{
	auto dock = new QDockWidget("Outliner", this);
	dock->setAttribute(Qt::WA_DeleteOnClose);

	OutlinerTreeWidget *outliner = new OutlinerTreeWidget(dock);
	outliner->listens(&m_context);
	outliner->update_state(event_type::object | event_type::added);

	dock->setWidget(outliner);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::addGraphOutlinerWidget()
{
	auto outliner_dock = new QDockWidget("Outliner", this);
	outliner_dock->setAttribute(Qt::WA_DeleteOnClose);

	OutlinerTreeWidget *outliner = new OutlinerTreeWidget(outliner_dock);
	outliner->listens(&m_context);
	outliner->update_state(event_type::object | event_type::added);

	outliner_dock->setWidget(outliner);
	outliner_dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, outliner_dock);

	auto graph_dock = new QDockWidget("Graph Editor", this);
	graph_dock->setAttribute(Qt::WA_DeleteOnClose);

	QtNodeEditor *graph_editor = new QtNodeEditor(graph_dock);
	graph_editor->listens(&m_context);
	graph_editor->setAddNodeMenu(m_add_nodes_menu);

	graph_dock->setWidget(graph_editor);
	graph_dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, graph_dock);

	tabifyDockWidget(graph_dock, outliner_dock);

	/* Make sure the graph editor visible by default. */
	graph_dock->raise();
}

void MainWindow::addPropertiesWidget()
{
	auto dock = new QDockWidget("Properties", this);
	dock->setAttribute(Qt::WA_DeleteOnClose);

	PropertiesWidget *properties = new PropertiesWidget(dock);
	properties->listens(&m_context);
	properties->update_state(static_cast<event_type>(-1));

	dock->setWidget(properties);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::closeEvent(QCloseEvent *)
{
	ecrit_reglages();
}

void MainWindow::ecrit_reglages() const
{
	QSettings settings;
	QStringList recent;

	for (const auto &recent_file : m_fichiers_recent) {
		recent.push_front(recent_file);
	}

	settings.setValue("projet_récents", recent);
}

void MainWindow::charge_reglages()
{
	QSettings settings;

	const auto &recent_files = settings.value("projet_récents").toStringList();

	for (const auto &file : recent_files) {
		if (QFile(file).exists()) {
			ajoute_fichier_recent(file, false);
		}
	}

	mis_a_jour_menu_fichier_recent();
}

std::string MainWindow::requiers_dialogue(int type)
{
	if (type == FICHIER_OUVERTURE) {
		const auto chemin = QFileDialog::getOpenFileName(this);
		return chemin.toStdString();
	}

	if (type == FICHIER_SAUVEGARDE) {
		const auto chemin = QFileDialog::getSaveFileName(this);
		return chemin.toStdString();
	}

	return "";
}
