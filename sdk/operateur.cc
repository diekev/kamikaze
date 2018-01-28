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

#include "operateur.h"

#include <iostream>

#include <tbb/tick_count.h>

#include "context.h"
#include "noeud.h"
#include "primitive.h"

/* ************************************************************************** */

void execute_operateur(Operateur *operateur, const Context &contexte, double temps)
{
	if (operateur->a_tampon() && !operateur->besoin_execution()) {
		return;
	}

	operateur->supprime_avertissements();

	auto t0 = tbb::tick_count::now();

	try {
		operateur->execute(contexte, temps);
	}
	catch (const std::exception &e) {
		operateur->ajoute_avertissement(e.what());
	}

	auto t1 = tbb::tick_count::now();
	auto delta = (t1 - t0).seconds();

	operateur->incremente_nombre_execution();

	operateur->temps_agrege(delta);

	/* Pour calculer le temps d'exécution de l'opérateur, on soustrait le temps
	 * d'exécution agrégé des noeuds en amont de celui du neoud courant. */
	auto temps_agrege_parent = 0.0;

	for (int i = 0; i < operateur->entrees(); ++i) {
		auto entree = operateur->entree(i);

		temps_agrege_parent += entree->temps_execution_parent();
	}

	operateur->temps_execution(delta - temps_agrege_parent);

	operateur->besoin_execution(false);
}

/* ************************************************************************** */

EntreeOperateur::EntreeOperateur(PriseEntree *prise)
	: m_prise(prise)
{}

PrimitiveCollection *EntreeOperateur::requiers_collection(PrimitiveCollection *collection, const Context &contexte, double temps)
{
	if (!est_connectee()) {
		return nullptr;
	}

	auto operateur = m_prise->lien->parent->operateur();

	execute_operateur(operateur, contexte, temps);

	auto collection_operateur = operateur->collection();

	if (collection_operateur == nullptr) {
		return nullptr;
	}

	if (collection == nullptr) {
		return collection_operateur;
	}

	/* S'il y a plusieurs liens, copie la collection afin d'éviter tout conflit. */
	if (m_prise->lien->liens.size() > 1) {
		collection_operateur->copy_collection(*collection);
		operateur->a_tampon(true);
	}
	else {
		/* Autrement, copie la collection et vide l'original. */
		collection->merge_collection(*collection_operateur);
		operateur->a_tampon(false);
	}

	return collection;
}

double EntreeOperateur::temps_execution_parent() const
{
	if (!est_connectee()) {
		return 0.0;
	}

	return m_prise->lien->parent->operateur()->temps_agrege();
}

bool EntreeOperateur::est_connectee() const
{
	return (m_prise != nullptr && m_prise->lien != nullptr);
}

/* ************************************************************************** */

Operateur::Operateur(Noeud *noeud, const Context &contexte)
{
	noeud->operateur(this);
	m_collection = new PrimitiveCollection(contexte.primitive_factory);
}

Operateur::~Operateur()
{
	delete m_collection;
}

type_operateur Operateur::type() const
{
	return type_operateur::STATIC;
}

EntreeOperateur *Operateur::entree(size_t index)
{
	if (index >= m_donnees_entree.size()) {
		return nullptr;
	}

	return &m_donnees_entree[index];
}

void Operateur::entrees(size_t nombre)
{
	m_nombre_entrees = nombre;
	m_donnees_entree.resize(nombre);
}

int Operateur::entrees() const
{
	return m_nombre_entrees;
}

void Operateur::sorties(size_t nombre)
{
	m_nombre_sorties = nombre;
}

int Operateur::sorties() const
{
	return m_nombre_sorties;
}

void Operateur::donnee_entree(size_t index, PriseEntree *prise)
{
	if (index >= m_donnees_entree.size()) {
		std::cerr << "Débordement lors de l'assignement de données d'entrées"
				  << "(nombre d'entrées: " << m_donnees_entree.size()
				  << ", index: " << index << ")\n";
		return;
	}

	m_donnees_entree[index] = EntreeOperateur(prise);
}

PrimitiveCollection *Operateur::collection() const
{
	return m_collection;
}

bool Operateur::besoin_execution() const
{
	return m_besoin_execution;
}

void Operateur::besoin_execution(bool ouinon)
{
	m_besoin_execution = ouinon;
}

double Operateur::temps_agrege() const
{
	return m_temps_agrege;
}

void Operateur::temps_agrege(double temps)
{
	m_min_temps_agrege = std::min(m_temps_agrege, temps);
	m_temps_agrege = temps;
}

double Operateur::temps_execution() const
{
	return m_temps_execution;
}

void Operateur::temps_execution(double temps)
{
	m_min_temps_execution = std::min(m_temps_execution, temps);
	m_temps_execution = temps;
}

double Operateur::min_temps_agrege() const
{
	return m_min_temps_agrege;
}

double Operateur::min_temps_execution() const
{
	return m_min_temps_execution;
}

int Operateur::nombre_executions() const
{
	return m_nombre_executions;
}

void Operateur::incremente_nombre_execution()
{
	m_nombre_executions += 1;
}

void Operateur::ajoute_avertissement(const std::string &avertissement)
{
	m_avertissements.push_back(avertissement);
}

const std::vector<std::string> &Operateur::avertissements() const
{
	return m_avertissements;
}

bool Operateur::a_avertissements() const
{
	return !m_avertissements.empty();
}

void Operateur::supprime_avertissements()
{
	m_avertissements.clear();
}

void Operateur::a_tampon(bool ouinon)
{
	m_a_tampon = ouinon;
}

bool Operateur::a_tampon() const
{
	return m_a_tampon;
}

std::string Operateur::chemin_icone() const
{
	return m_chemin_icone;
}

void Operateur::chemin_icone(const std::string &chemin)
{
	m_chemin_icone = chemin;
}

const char *Operateur::chemin_interface() const
{
	return "";
}

/* ************************************************************************** */

DescOperateur::DescOperateur(const std::string &opname, const std::string &ophelp, const std::string &opcategorie, DescOperateur::fonction_usine func)
	: nom(opname)
	, categorie(opcategorie)
	, text_aide(ophelp)
	, construction_operateur(func)
{}

/* ************************************************************************** */

size_t UsineOperateur::enregistre_type(const std::string &nom, DescOperateur desc)
{
	const auto iter = m_tableau.find(nom);
	assert(iter == m_tableau.end());

	m_categories.emplace(desc.categorie);
	m_tableau[nom] = desc;

	return nombre_entrees();
}

Operateur *UsineOperateur::operator()(const std::string &nom, Noeud *noeud, const Context &contexte)
{
	const auto iter = m_tableau.find(nom);
	assert(iter != m_tableau.end());

	const DescOperateur &desc = iter->second;

	return desc.construction_operateur(noeud, contexte);
}

const std::set<std::string> &UsineOperateur::categories() const
{
	return m_categories;
}

std::vector<DescOperateur> UsineOperateur::cles(const std::string &categorie) const
{
	std::vector<DescOperateur> v;
	v.reserve(nombre_entrees());

	for (const auto &entree : m_tableau) {
		DescOperateur desc = entree.second;

		if (desc.categorie == categorie) {
			v.push_back(entree.second);
		}
	}

	return v;
}

bool UsineOperateur::est_enregistre(const std::string &cle) const
{
	return (m_tableau.find(cle) != m_tableau.end());
}
