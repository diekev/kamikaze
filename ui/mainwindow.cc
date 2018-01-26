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
//	"interface/menu_edition.kangao",
	"interface/menu_objet.kangao",
	"interface/menu_debogage.kangao",
	"interface/menu_prereglage.kangao",
};

class RepondantCommande : public kangao::RepondantBouton {
	Main *m_main = nullptr;
	Context *m_contexte = nullptr;

public:
	RepondantCommande(
			Main *main,
			Context *contexte)
		: m_main(main)
		, m_contexte(contexte)
	{}

	void repond_clique(const std::string &identifiant, const std::string &metadonnee) override
	{
		std::cerr << "Répond clique : " << identifiant << ", " << metadonnee << '\n';

		/* Get command, and give it the name of the UI button which will be used to
		 * look up keys in the various creation factories if need be. This could and
		 * should be handled better. */
		auto commande = (*m_main->usine_commandes())(identifiant);

		/* Execute the command in the current context, the manager will push the
		* command on the undo stack. */
		m_main->gestionnaire_commande()->execute(m_main, commande, *m_contexte, metadonnee);
	}

	bool evalue_predicat(const std::string &identifiant, const std::string &metadonnee) override
	{
		std::cerr << "Évalue prédicat : " << identifiant << ", " << metadonnee << '\n';
		auto commande = (*m_main->usine_commandes())(identifiant);
		return commande->evalue_predicat(m_main, *m_contexte, metadonnee);
	}
};

MainWindow::MainWindow(Main *main, QWidget *parent)
    : QMainWindow(parent)
	, m_main(main)
	, m_repondant_commande(new RepondantCommande(main, &m_context))
	, m_gestionnaire(new kangao::GestionnaireInterface)
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
	delete m_gestionnaire;
	delete m_repondant_commande;
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

void MainWindow::generateNodeMenu()
{
	auto categories = m_main->usine_operateur()->categories();

	std::stringstream ss;

	ss << "menu \"Ajoute Noeud\" {\n";

	for (const auto &categorie : categories) {
		ss << "\tmenu \"" << categorie << "\" {\n";

		auto cles = m_main->usine_operateur()->cles(categorie);

		std::sort(cles.begin(), cles.end(),
				  [](const DescOperateur &desc1, const DescOperateur &desc2)
		{
			return desc1.nom < desc2.nom;
		});

		for (const auto &description : cles) {
			ss << "\t\taction(valeur=\"" << description.nom << "\"; attache=\"ajouter_noeud\"; métadonnée=\"" << description.nom << "\")\n";
		}

		ss << "\t}\n";
	}

	ss << "}";

	kangao::DonneesInterface donnees;
	donnees.manipulable = nullptr;
	donnees.conteneur = nullptr;
	donnees.repondant_bouton = m_repondant_commande;

	m_add_nodes_menu = m_gestionnaire->compile_menu(donnees, ss.str().c_str());
	menuBar()->addMenu(m_add_nodes_menu);
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

void MainWindow::genere_menu_fichier()
{
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

		auto menu = m_gestionnaire->compile_menu(donnees, texte_entree.c_str());

		menuBar()->addMenu(menu);
	}

	auto menu_fichiers_recents = m_gestionnaire->pointeur_menu("Projets Récents");
	connect(menu_fichiers_recents, SIGNAL(aboutToShow()),
			this, SLOT(mis_a_jour_menu_fichier_recent()));
}

void MainWindow::generatePresetMenu()
{
	/* À FAIRE */
#if 0
	m_tool_bar = kangao::compile_barre_outils("interface/menu_prereglage.kangao");
	addToolBar(Qt::TopToolBarArea, m_tool_bar);
#endif
}

void MainWindow::mis_a_jour_menu_fichier_recent()
{
	std::vector<kangao::DonneesAction> donnees_actions;

	kangao::DonneesAction donnees;
	donnees.attache = "ouvrir_fichier_recent";
	donnees.repondant_bouton = m_repondant_commande;

	for (const auto &fichier_recent : m_main->fichiers_recents()) {
		auto name = QFileInfo(fichier_recent.c_str()).fileName();

		donnees.nom = name.toStdString();
		donnees.metadonnee = fichier_recent;

		donnees_actions.push_back(donnees);
	}

	m_gestionnaire->recree_menu("Projets Récents", donnees_actions);
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

	QtNodeEditor *graph_editor = new QtNodeEditor(m_repondant_commande, m_gestionnaire, dock);
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

	QtNodeEditor *graph_editor = new QtNodeEditor(m_repondant_commande, m_gestionnaire, graph_dock);
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

	for (const auto &fichier_recent : m_main->fichiers_recents()) {
		recent.push_front(fichier_recent.c_str());
	}

	settings.setValue("projet_récents", recent);
}

void MainWindow::charge_reglages()
{
	QSettings settings;

	const auto &recent_files = settings.value("projet_récents").toStringList();

	for (const auto &file : recent_files) {
		if (QFile(file).exists()) {
			m_main->ajoute_fichier_recent(file.toStdString());
		}
	}
}
