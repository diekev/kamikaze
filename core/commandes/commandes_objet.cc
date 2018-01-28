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

#include "commandes_objet.h"

#include <kamikaze/context.h>
#include <kamikaze/noeud.h>
#include <kamikaze/operateur.h>
#include <kamikaze/primitive.h>

#include "graphs/object_graph.h"
#include "operateurs/operateurs_standards.h"

#include "object.h"
#include "scene.h"

/* *************************** add object command *************************** */

class CommandeAjoutObjet : public Commande {
	Object *m_object = nullptr;
	Scene *m_scene = nullptr;

public:
	CommandeAjoutObjet() = default;
	~CommandeAjoutObjet() = default;

	void execute(Main *main, const Context &context, const DonneesCommande &donnees) override;

	void defait() override {}
	void refait() override {}
};

void CommandeAjoutObjet::execute(Main */*main*/, const Context &context, const DonneesCommande &donnees)
{
	m_scene = context.scene;

	m_object = new Object(context);
	m_object->name(donnees.metadonnee);

	assert(m_scene != nullptr);
	m_scene->addObject(m_object);
}

/* **************************** add node command **************************** */

class CommandeAjoutNoeud : public Commande {
	Object *m_object = nullptr;
	Scene *m_scene = nullptr;

public:
	CommandeAjoutNoeud() = default;
	~CommandeAjoutNoeud() = default;

	void execute(Main *main, const Context &context, const DonneesCommande &donnees) override;

	void defait() override {}
	void refait() override {}
};

void CommandeAjoutNoeud::execute(Main */*main*/, const Context &context, const DonneesCommande &donnees)
{
	m_scene = context.scene;
	auto scene_node = m_scene->active_node();

	if (scene_node == nullptr) {
		return;
	}

	m_object = static_cast<Object *>(scene_node);

	assert(m_object != nullptr);

	auto noeud = new Noeud();
	noeud->nom(donnees.metadonnee);

	auto operateur = (*context.usine_operateur)(donnees.metadonnee, noeud, context);
	static_cast<void>(operateur);

	noeud->synchronise_donnees();

	m_object->ajoute_noeud(noeud);

	m_scene->notify_listeners(event_type::node | event_type::added);
}

/* **************************** add torus command **************************** */

class CommandeObjetPrereglage : public Commande {
	Object *m_object = nullptr;
	Scene *m_scene = nullptr;

public:
	CommandeObjetPrereglage() = default;
	~CommandeObjetPrereglage() = default;

	void execute(Main *main, const Context &context, const DonneesCommande &donnees) override;

	void defait() override {}
	void refait() override {}
};

void CommandeObjetPrereglage::execute(Main */*main*/, const Context &context, const DonneesCommande &donnees)
{
	m_scene = context.scene;

	if (context.eval_ctx->edit_mode) {
		auto scene_node = m_scene->active_node();

		/* Sanity check. */
		if (scene_node == nullptr) {
			return;
		}

		m_object = static_cast<Object *>(scene_node);
	}
	else {
		m_object = new Object(context);
		m_object->name(donnees.metadonnee);
	}

	assert(m_object != nullptr);

	auto noeud = new Noeud();
	noeud->posx(-300);
	noeud->posy(-100);

	(*context.usine_operateur)(donnees.metadonnee, noeud, context);

	noeud->synchronise_donnees();

	m_object->ajoute_noeud(noeud);

	auto graph = m_object->graph();
	graph->ajoute_selection(noeud);
	graph->connecte(noeud->sortie(0), graph->sortie()->entree(0));

	if (!context.eval_ctx->edit_mode) {
		m_scene->addObject(m_object);
		m_scene->evalObjectDag(context, m_object);
	}
	else {
		m_scene->notify_listeners(event_type::node | event_type::added);
	}
}

/* ************************************************************************** */

class CommandeEntreObjet : public Commande {
public:
	CommandeEntreObjet() = default;
	~CommandeEntreObjet() = default;

	void execute(Main */*main*/, const Context &context, const DonneesCommande &/*donnees*/) override
	{
		context.eval_ctx->edit_mode = true;
		context.scene->notify_listeners(event_type::object | event_type::selected);
	}

	void defait() override {}
	void refait() override {}
};

/* ************************************************************************** */

class CommandeSorsObjet : public Commande {
public:
	CommandeSorsObjet() = default;
	~CommandeSorsObjet() = default;

	void execute(Main */*main*/, const Context &context, const DonneesCommande &/*donnees*/) override
	{
		context.eval_ctx->edit_mode = false;
		context.scene->notify_listeners(event_type::object | event_type::selected);
	}

	void defait() override {}
	void refait() override {}
};

/* ************************************************************************** */

void enregistre_commandes_objet(UsineCommande *usine)
{
	usine->enregistre_type("ajouter_objet",
						   description_commande<CommandeAjoutObjet>(
							   "objet", 0, 0, 0, false));

	usine->enregistre_type("ajouter_noeud",
						   description_commande<CommandeAjoutNoeud>(
							   "objet", 0, 0, 0, false));

	usine->enregistre_type("ajouter_prereglage",
						   description_commande<CommandeObjetPrereglage>(
							   "objet", 0, 0, 0, false));

	usine->enregistre_type("objet.entre",
						   description_commande<CommandeEntreObjet>(
							   "objet", 0, 0, 0, false));

	usine->enregistre_type("objet.sors",
						   description_commande<CommandeSorsObjet>(
							   "objet", 0, 0, 0, false));
}
