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

#include "kamikaze_main.h"
#include "undo.h"

RepondantCommande::RepondantCommande(Main *main, Context *contexte)
	: m_main(main)
	, m_contexte(contexte)
{}

bool RepondantCommande::appele_commande(const std::string &categorie, const DonneesCommande &donnees_commande)
{
	std::cerr << "Appele commande pour catégorie : " << categorie << " :\n";
	std::cerr << "\tclé : " << donnees_commande.cle << '\n';
	std::cerr << "\tmodificateur : " << donnees_commande.modificateur << '\n';
	std::cerr << "\tsouris : " << donnees_commande.souris << '\n';
	std::cerr << "\tx : " << donnees_commande.x << '\n';
	std::cerr << "\ty : " << donnees_commande.y << '\n';

	auto commande = m_main->usine_commandes()->trouve_commande(categorie, donnees_commande);

	if (commande == nullptr) {
		return false;
	}

	execute_commande(commande, donnees_commande);

	return true;
}

void RepondantCommande::execute_commande(Commande *commande, const DonneesCommande &donnees)
{
	m_main->gestionnaire_commande()->execute(m_main, commande, *m_contexte, "");
}

void RepondantCommande::repond_clique(const std::string &identifiant, const std::string &metadonnee)
{
	auto commande = (*m_main->usine_commandes())(identifiant);
	m_main->gestionnaire_commande()->execute(m_main, commande, *m_contexte, metadonnee);
}

bool RepondantCommande::evalue_predicat(const std::string &identifiant, const std::string &metadonnee)
{
	auto commande = (*m_main->usine_commandes())(identifiant);
	return commande->evalue_predicat(m_main, *m_contexte, metadonnee);
}
