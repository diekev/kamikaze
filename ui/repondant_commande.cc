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

#include "repondant_commande.h"

#include <iostream>

#include "kamikaze_main.h"
#include "undo.h"

RepondantCommande::RepondantCommande(Main *main, Context *contexte)
	: m_main(main)
	, m_contexte(contexte)
{}

bool RepondantCommande::appele_commande(const std::string &categorie, const DonneesCommande &donnees_commande)
{
#if 0
	std::cerr << "Appele commande pour catégorie : " << categorie << " :\n";
	std::cerr << "\tclé : " << donnees_commande.cle << '\n';
	std::cerr << "\tmodificateur : " << donnees_commande.modificateur << '\n';
	std::cerr << "\tsouris : " << donnees_commande.souris << '\n';
	std::cerr << "\tx : " << donnees_commande.x << '\n';
	std::cerr << "\ty : " << donnees_commande.y << '\n';
#endif

	auto commande = m_main->usine_commandes()->trouve_commande(categorie, donnees_commande);

	if (commande == nullptr) {
		return false;
	}

	execute_commande(commande, donnees_commande);

	return true;
}

bool RepondantCommande::appele_commande_modale(const std::string &categorie, const DonneesCommande &donnees_commande)
{
#if 0
	std::cerr << "Appele commande souris pour catégorie : " << categorie << " :\n";
	std::cerr << "\tclé : " << donnees_commande.cle << '\n';
	std::cerr << "\tmodificateur : " << donnees_commande.modificateur << '\n';
	std::cerr << "\tsouris : " << donnees_commande.souris << '\n';
	std::cerr << "\tx : " << donnees_commande.x << '\n';
	std::cerr << "\ty : " << donnees_commande.y << '\n';
#endif

	auto commande = m_main->usine_commandes()->trouve_commande(categorie, donnees_commande);

	if (commande == nullptr) {
		return false;
	}

	m_commande_modale = commande;

	m_commande_modale->demarre_execution_modale(m_main, *m_contexte, donnees_commande);

	return true;
}

void RepondantCommande::ajourne_commande_modale(const DonneesCommande &donnees_commande)
{
	if (!m_commande_modale) {
		return;
	}

	m_commande_modale->ajourne_execution_modale(m_main, *m_contexte, donnees_commande);
}

void RepondantCommande::acheve_commande_modale(const DonneesCommande &donnees_commande)
{
	if (!m_commande_modale) {
		return;
	}

	m_commande_modale->termine_execution_modale(m_main, *m_contexte, donnees_commande);
	m_commande_modale = nullptr;
}

void RepondantCommande::execute_commande(Commande *commande, const DonneesCommande &donnees)
{
	m_main->gestionnaire_commande()->execute(m_main, commande, *m_contexte, donnees);
}

void RepondantCommande::repond_clique(const std::string &identifiant, const std::string &metadonnee)
{
	auto commande = (*m_main->usine_commandes())(identifiant);

	DonneesCommande donnees;
	donnees.metadonnee = metadonnee;

	m_main->gestionnaire_commande()->execute(m_main, commande, *m_contexte, donnees);
}

bool RepondantCommande::evalue_predicat(const std::string &identifiant, const std::string &metadonnee)
{
	auto commande = (*m_main->usine_commandes())(identifiant);
	return commande->evalue_predicat(m_main, *m_contexte, metadonnee);
}
