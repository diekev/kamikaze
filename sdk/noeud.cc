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
 * The Original Code is Copyright (C) 2017 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include "noeud.h"

#include "operateur.h"

/* ************************************************************************** */

PriseEntree::PriseEntree(const std::string &nom_prise)
	: parent(nullptr)
	, lien(nullptr)
	, nom(nom_prise)
{}

/* ************************************************************************** */

PriseSortie::PriseSortie(const std::string &nom_prise)
	: parent(nullptr)
	, nom(nom_prise)
{}

/* ************************************************************************** */

Noeud::~Noeud()
{
	delete m_operateur;

	for (auto &entree : m_entrees) {
		delete entree;
	}

	for (auto &sortie : m_sorties) {
		delete sortie;
	}
}

std::string Noeud::nom() const
{
	return m_nom;
}

void Noeud::nom(std::string nom)
{
	m_nom = std::move(nom);
}

void Noeud::ajoute_entree(const std::string &nom)
{
	auto prise = new PriseEntree(nom);
	prise->parent = this;

	m_entrees.push_back(prise);
}

void Noeud::ajoute_sortie(const std::string &nom)
{
	auto prise = new PriseSortie(nom);
	prise->parent = this;

	m_sorties.push_back(prise);
}

PriseEntree *Noeud::entree(int index)
{
	return m_entrees[index];
}

PriseEntree *Noeud::entree(const std::string &nom)
{
	for (const auto &entree : m_entrees) {
		if (entree->nom == nom) {
			return entree;
		}
	}

	return nullptr;
}

PriseSortie *Noeud::sortie(int index)
{
	return m_sorties[index];
}

PriseSortie *Noeud::sortie(const std::string &nom)
{
	for (const auto &sortie : m_sorties) {
		if (sortie->nom == nom) {
			return sortie;
		}
	}

	return nullptr;
}

std::vector<PriseEntree *> Noeud::entrees() const noexcept
{
	return m_entrees;
}

std::vector<PriseSortie *> Noeud::sorties() const noexcept
{
	return m_sorties;
}

bool Noeud::est_connecte() const
{
	return a_entree_connectee() || a_sortie_connectee();
}

bool Noeud::a_entree_connectee() const
{
	auto entree_connectee = false;

	for (const auto &entree : m_entrees) {
		if (entree->lien != nullptr) {
			entree_connectee = true;
			break;
		}
	}

	return entree_connectee;
}

bool Noeud::a_sortie_connectee() const
{
	auto sortie_connectee = false;

	for (const auto &sortie : m_sorties) {
		if (!sortie->liens.empty()) {
			sortie_connectee = true;
			break;
		}
	}

	return sortie_connectee;
}

void Noeud::synchronise_donnees()
{
	auto op = this->operateur();

	for (int i = 0; i < op->entrees(); ++i) {
		this->ajoute_entree(op->nom_entree(i));
	}

	for (int i = 0; i < op->sorties(); ++i) {
		this->ajoute_sortie(op->nom_sortie(i));
	}

	auto index = 0;

	for (auto socket : this->entrees()) {
		op->donnee_entree(index++, socket);
	}
}

void Noeud::operateur(Operateur *operateur)
{
	m_operateur = operateur;
}

Operateur *Noeud::operateur() const
{
	return m_operateur;
}

double Noeud::posx() const
{
	return m_posx;
}

void Noeud::posx(double pos)
{
	m_posx = pos;
}

double Noeud::posy() const
{
	return m_posy;
}

void Noeud::posy(double pos)
{
	m_posy = pos;
}

/* ************************************************************************** */

#if 0
static void etiquette_sale_amont(Noeud *noeud)
{
	if (noeud == nullptr) {
		return;
	}

	noeud->operateur()->besoin_execution(true);

	for (const PriseEntree *entree : noeud->entrees()) {
		if (entree->lien->liens.size() > 1) {
			break;
		}

		etiquette_sale_amont(entree->lien->parent);
	}
}

static void etiquette_sale_aval(Noeud *noeud)
{
	if (noeud == nullptr) {
		return;
	}

	noeud->operateur()->besoin_execution(true);

	for (const PriseSortie *sortie : noeud->sorties()) {
		for (const PriseEntree *entree : sortie->liens) {
			etiquette_sale_aval(entree->parent);
		}
	}
}
#endif
