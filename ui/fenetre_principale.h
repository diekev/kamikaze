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

class Main;
class QProgressBar;
class RepondantCommande;

namespace kangao {

class GestionnaireInterface;

}  /* namespace kangao */

class FenetrePrincipale : public QMainWindow {
	Q_OBJECT

	Main *m_main = nullptr;

	Context m_contexte;
	EvaluationContext m_contexte_evaluation;

	RepondantCommande *m_repondant_commande = nullptr;
	kangao::GestionnaireInterface *m_gestionnaire = nullptr;

	QMenu *m_menu_ajout_noeud = nullptr;

	QProgressBar *m_barre_progres = nullptr;

	QDockWidget *m_dock_canevas = nullptr;
	bool m_possede_canevas = false;

public:
	explicit FenetrePrincipale(Main *main, QWidget *parent = nullptr);
	~FenetrePrincipale();

public Q_SLOTS:
	/* Progress Bar */
	void taskStarted();
	void updateProgress(float progress);
	void taskEnded();
	void nodeProcessed();

private:
	void genere_menu_fichier();
	void genere_menu_fenetre();
	void genere_menu_noeud();
	void genere_menu_prereglages();

	void charge_reglages();
	void ecrit_reglages() const;

	void closeEvent(QCloseEvent *) override;

private Q_SLOTS:
	void ajout_editeur_arborescence();
	void ajout_editeur_canevas();
	void ajout_editeur_graphe();
	void ajout_editeur_ligne_temps();
	void ajout_editeur_propriete();
	void ajout_editeurs_arborescence_graphe();

	void mis_a_jour_menu_fichier_recent();
};
