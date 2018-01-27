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

#include "undo.h"

#include "util/util_memory.h"

CommandManager::~CommandManager()
{
	release_stack_memory(m_undo_commands);
	release_stack_memory(m_redo_commands);
}

void CommandManager::execute(Main *main, Commande *command, const Context &context, const std::string &metadonnee)
{
	command->execute(main, context, metadonnee);
	m_undo_commands.push(command);
}

static void undo_redo_ex(std::stack<Commande *> &pop_stack,
                         std::stack<Commande *> &push_stack,
                         bool redo)
{
	if (pop_stack.empty()) {
		return;
	}

	auto command = pop_stack.top();
	pop_stack.pop();

	if (redo) {
		command->refait();
	}
	else {
		command->defait();
	}

	push_stack.push(command);
}

void CommandManager::defait()
{
	undo_redo_ex(m_undo_commands, m_redo_commands, false);
}

void CommandManager::refait()
{
	undo_redo_ex(m_redo_commands, m_undo_commands, true);
}

bool Commande::evalue_predicat(Main *main, const Context &context, const std::string &metadonnee)
{
	return true;
}

void UsineCommande::enregistre_type(const std::string &nom, const DescriptionCommande &description)
{
	const auto iter = m_tableau.find(nom);
	assert(iter == m_tableau.end());

	m_tableau[nom] = description;
}

Commande *UsineCommande::operator()(const std::string &nom)
{
	const auto iter = m_tableau.find(nom);
	assert(iter != m_tableau.end());

	const DescriptionCommande &desc = iter->second;

	return desc.construction_commande();
}

Commande *UsineCommande::trouve_commande(const std::string &categorie, const DonneesCommande &donnees_commande)
{
	for (const auto &donnees : m_tableau) {
		const DescriptionCommande &desc = donnees.second;

		if (desc.categorie != categorie) {
			continue;
		}

		if (desc.souris != donnees_commande.souris) {
			continue;
		}

		if (desc.modificateur != donnees_commande.modificateur) {
			continue;
		}

		if (desc.cle != donnees_commande.cle) {
			continue;
		}

		return desc.construction_commande();
	}

	return nullptr;
}
