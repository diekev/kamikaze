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
 * The Original Code is Copyright (C) 2016 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#pragma once

#include <kamikaze/factory.h>

#include <stack>
#include <unordered_map>

class Context;
class Main;
class ParamCallback;

struct DonneesCommande {
	int souris = 0;
	int modificateur = 0;
	int cle = 0;
	bool double_clique = false;
	float x = 0;
	float y = 0;
	std::string metadonnee = "";

	DonneesCommande() = default;
};

class Commande {
public:
	virtual ~Commande() = default;

	virtual bool evalue_predicat(Main *main, const Context &context, const std::string &metadonnee);

	virtual void execute(Main *main, const Context &context, const DonneesCommande &donnees) = 0;

	virtual void demarre_execution_modale(Main *main, const Context &context, const DonneesCommande &donnees);

	virtual void ajourne_execution_modale(Main *main, const Context &context, const DonneesCommande &donnees);

	virtual void termine_execution_modale(Main *main, const Context &context, const DonneesCommande &donnees);

	virtual void defait() = 0;
	virtual void refait() = 0;
};

class CommandManager final {
	std::stack<Commande *> m_undo_commands;
	std::stack<Commande *> m_redo_commands;

public:
	~CommandManager();

	void execute(Main *main, Commande *command, const Context &context, const DonneesCommande &donnees);
	void defait();
	void refait();
};

struct DescriptionCommande {
	typedef Commande *(*fonction_usine)();

	std::string categorie;
	int souris = 0;
	int modificateur = 0;
	int cle = 0;
	bool double_clique = false;

	fonction_usine construction_commande = nullptr;
};

template <typename T>
static inline constexpr auto description_commande(
		const std::string &categorie, int souris, int modificateur, int cle, bool double_clique)
{
	DescriptionCommande description;
	description.cle = cle;
	description.souris = souris;
	description.modificateur = modificateur;
	description.categorie = categorie;
	description.double_clique = double_clique;
	description.construction_commande = []() -> Commande* { return new T(); };

	return description;
}

class UsineCommande {
	std::unordered_map<std::string, DescriptionCommande> m_tableau;

public:
	void enregistre_type(const std::string &nom, const DescriptionCommande &description);

	Commande *operator()(const std::string &nom);

	Commande *trouve_commande(const std::string &categorie, const DonneesCommande &donnees_commande);
};
