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
 * The Original Code is Copyright (C) 2018 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include "vue_editeur_noeud.h"

#include <danjo/danjo.h>

#include <QKeyEvent>
#include <QMenu>

#include "core/undo.h"

#include "repondant_commande.h"

/* ************************************************************************** */

VueEditeurNoeud::VueEditeurNoeud(
		RepondantCommande *repondant,
		danjo::GestionnaireInterface *gestionnaire,
		QWidget *parent)
	: QGraphicsView(parent)
	, m_gestionnaire(gestionnaire)
	, m_repondant_commande(repondant)
{
	danjo::DonneesInterface donnees;
	donnees.manipulable = nullptr;
	donnees.conteneur = nullptr;
	donnees.repondant_bouton = m_repondant_commande;

	const auto texte_entree = danjo::contenu_fichier("interface/menu_editeur_noeud.danjo");
	m_menu_contexte = m_gestionnaire->compile_menu(donnees, texte_entree.c_str());

	setDragMode(QGraphicsView::ScrollHandDrag);
}

VueEditeurNoeud::VueEditeurNoeud(QGraphicsScene *scene, QWidget *parent)
	: QGraphicsView(scene, parent)
{
	setDragMode(QGraphicsView::ScrollHandDrag);
}

void VueEditeurNoeud::menu_ajout_noeud(QMenu *menu)
{
	m_menu_ajout_noeud = menu;
}

void VueEditeurNoeud::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_A) {
		/* À FAIRE : vérifie si nous sommes en mode édition? */
		m_menu_ajout_noeud->popup(QCursor::pos());
	}
	else {
		DonneesCommande donnees;
		donnees.cle = event->key();

		m_repondant_commande->appele_commande("graphe", donnees);
	}
}

void VueEditeurNoeud::wheelEvent(QWheelEvent *event)
{
	this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

	const auto factor = 1.15f;
	const auto zoom = ((event->delta() > 0) ? factor : 1.0f / factor);

	this->scale(zoom, zoom);
}

void VueEditeurNoeud::mouseMoveEvent(QMouseEvent *event)
{
	const auto position = mapToScene(event->pos());

	DonneesCommande donnees;
	donnees.souris = Qt::LeftButton;
	donnees.x = position.x();
	donnees.y = position.y();

	m_repondant_commande->ajourne_commande_modale(donnees);
}

void VueEditeurNoeud::mousePressEvent(QMouseEvent *event)
{
	switch (event->button()) {
		case Qt::LeftButton:
		{
			const auto position = mapToScene(event->pos());

			DonneesCommande donnees;
			donnees.souris = Qt::LeftButton;
			donnees.x = position.x();
			donnees.y = position.y();

			m_repondant_commande->appele_commande_modale("graphe", donnees);
			break;
		}
		case Qt::MiddleButton:
		{
			/* À FAIRE : montre infos noeud. */
			break;
		}
		case Qt::RightButton:
		{
			m_gestionnaire->ajourne_menu("Éditeur Noeud");
			m_menu_contexte->popup(event->globalPos());
			break;
		}
		default:
			break;
	}
}

void VueEditeurNoeud::mouseDoubleClickEvent(QMouseEvent *event)
{
	const auto position = mapToScene(event->pos());

	DonneesCommande donnees;
	donnees.double_clique = true;
	donnees.souris = Qt::LeftButton;
	donnees.x = position.x();
	donnees.y = position.y();

	m_repondant_commande->appele_commande("graphe", donnees);
}

void VueEditeurNoeud::mouseReleaseEvent(QMouseEvent *event)
{
	const auto position = mapToScene(event->pos());

	DonneesCommande donnees;
	donnees.x = position.x();
	donnees.y = position.y();

	m_repondant_commande->acheve_commande_modale(donnees);
}
