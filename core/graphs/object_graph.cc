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

#include "object_graph.h"

#include <algorithm>
#include <iostream>

#include "operateurs/operateurs_standards.h"

Graph::Graph(const Context &contexte)
	: m_besoin_actualisation(false)
{
	auto noeud_sortie = new Noeud();
	noeud_sortie->nom("Sortie");

	auto operateur = new OperateurSortie(noeud_sortie, contexte);
	static_cast<void>(operateur);

	noeud_sortie->synchronise_donnees();

	ajoute(noeud_sortie);
}

void Graph::ajoute(Noeud *noeud)
{
	m_noeuds.push_back(std::unique_ptr<Noeud>(noeud));
}

void Graph::enleve(Noeud *noeud)
{
	auto iter = std::find_if(m_noeuds.begin(), m_noeuds.end(),
							 [noeud](const std::unique_ptr<Noeud> &node_ptr)
	{
		return node_ptr.get() == noeud;
	});

	if (iter == m_noeuds.end()) {
		std::cerr << "Impossible de trouver le noeud dans le graphe !\n";
		return;
	}

	/* Déconnecte les entrées. */
	for (PriseEntree *entree : noeud->entrees()) {
		if (entree->lien) {
			deconnecte(entree->lien, entree);
		}
	}

	/* Déconnecte les sorties. */
	for (PriseSortie *sortie : noeud->sorties()) {
		for (PriseEntree *entree : sortie->liens) {
			deconnecte(sortie, entree);
		}
	}

	m_noeuds.erase(iter);

	m_besoin_actualisation = true;
}

void Graph::connecte(PriseSortie *de, PriseEntree *a)
{
	if (a->lien != nullptr) {
		std::cerr << "L'entrée est déjà connectée !\n";
		return;
	}

	a->lien = de;
	de->liens.push_back(a);

	auto lien = new LienNoeud;
	lien->entree = a;
	lien->sortie = de;

	m_liens.push_back(lien);

	m_besoin_actualisation = true;
}

void Graph::deconnecte(PriseSortie *de, PriseEntree *a)
{
	auto iter = std::find(de->liens.begin(), de->liens.end(), a);

	if (iter == de->liens.end()) {
		std::cerr << "Il n'y a pas de lien entre les prises !\n";
		return;
	}

	de->liens.erase(iter);

	/* S'il reste une seule connection, supprime le tampon. */
	if (de->liens.size() <= 1) {
		de->parent->operateur()->a_tampon(false);
	}

	a->lien = nullptr;

	/* Supprime le lien. */
	auto iter_lien = std::find_if(m_liens.begin(), m_liens.end(),
								  [=](LienNoeud *lien)
	{
		return lien->entree == a && lien->sortie == de;
	});

	LienNoeud *lien = *iter_lien;

	if (lien->a_drapeau(NOEUD_SELECTIONE)) {
		enleve_selection(lien);
	}

	delete lien;

	m_liens.erase(iter_lien);

	m_besoin_actualisation = true;
}

const std::vector<std::unique_ptr<Noeud> > &Graph::noeuds() const
{
	return m_noeuds;
}

const std::vector<LienNoeud *> &Graph::liens() const
{
	return m_liens;
}

void Graph::noeud_actif(Noeud *noeud)
{
	m_noeud_actif = noeud;
}

Noeud *Graph::noeud_actif() const
{
	if (m_noeuds_selectiones.empty()) {
		return nullptr;
	}

	return m_noeuds_selectiones.back();
}

Noeud *Graph::sortie() const
{
	return static_cast<Noeud *>(m_noeuds.front().get());
}

void Graph::ajoute_selection(Noeud *noeud)
{
	noeud->ajoute_drapeau(NOEUD_SELECTIONE);
	m_noeuds_selectiones.push_back(noeud);
}

void Graph::enleve_selection(Noeud *noeud)
{
	noeud->enleve_drapeau(NOEUD_SELECTIONE);

	m_noeuds_selectiones.erase(
				std::find(m_noeuds_selectiones.begin(),
						  m_noeuds_selectiones.end(),
						  noeud));
}

void Graph::ajoute_selection(LienNoeud *lien)
{
	lien->ajoute_drapeau(NOEUD_SELECTIONE);
	m_liens_selectiones.push_back(lien);
}

void Graph::enleve_selection(LienNoeud *lien)
{
	lien->enleve_drapeau(NOEUD_SELECTIONE);

	m_liens_selectiones.erase(
				std::find(m_liens_selectiones.begin(),
						  m_liens_selectiones.end(),
						  lien));
}

void Graph::supprime_selection()
{
	for (auto &noeud : m_noeuds_selectiones) {
		enleve(noeud);
	}

	m_noeuds_selectiones.clear();

	for (auto &lien : m_liens_selectiones) {
		delete lien;
	}

	m_liens_selectiones.clear();
}

void Graph::deselectionne_tout()
{
	for (auto &noeud : m_noeuds_selectiones) {
		noeud->enleve_drapeau(NOEUD_SELECTIONE);
	}

	for (auto &lien : m_liens_selectiones) {
		lien->enleve_drapeau(NOEUD_SELECTIONE);
	}

	m_noeuds_selectiones.clear();
	m_liens_selectiones.clear();
}

void Graph::zoom(float valeur)
{
	m_zoom = valeur;
}

float Graph::zoom() const
{
	return m_zoom;
}

bool Graph::selection_vide() const
{
	return m_noeuds_selectiones.empty();
}
