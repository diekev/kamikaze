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

#include "commandes_graphes.h"

#include "graphs/graph_dumper.h"

#include "kamikaze_main.h"
#include "undo.h"

/* ************************************************************************** */

class CommandeDessineGrapheObjet final : public Commande {
public:
	void execute(Main */*main*/, const Context &context, const std::string &/*metadonnee*/) override
	{
		auto scene = context.scene;
		auto scene_node = scene->active_node();

		if (!scene_node) {
			return;
		}

		auto object = static_cast<Object *>(scene_node);

		GraphDumper gd(object->graph());
		gd("/tmp/object_graph.gv");

		if (system("dot /tmp/object_graph.gv -Tpng -o object_graph.png") == -1) {
			std::cerr << "Cannot create graph image from dot\n";
		}
	}

	void defait() override {}
	void refait() override {}
};

/* ************************************************************************** */

class CommandeDessineGrapheDependance final : public Commande {
public:
	void execute(Main */*main*/, const Context &context, const std::string &/*metadonnee*/) override
	{
		auto scene = context.scene;
		DepsGraphDumper gd(scene->depsgraph());
		gd("/tmp/depsgraph.gv");

		if (system("dot /tmp/depsgraph.gv -Tpng -o depsgraph.png") == -1) {
			std::cerr << "Cannot create graph image from dot\n";
		}
	}

	void defait() override {}
	void refait() override {}
};

/* ************************************************************************** */

class CommandeGrapheZoom final : public Commande {
	void execute(Main */*main*/, const Context &context, const std::string &metadonnee) override
	{
		auto zoom = 1.0f;

		if (metadonnee == "10%") {
			zoom = 0.1f;
		}
		else if (metadonnee == "25%") {
			zoom = 0.25f;
		}
		else if (metadonnee == "50%") {
			zoom = 0.5f;
		}
		else if (metadonnee == "75%") {
			zoom = 0.75f;
		}
		else if (metadonnee == "90%") {
			zoom = 0.90f;
		}
		else if (metadonnee == "100%") {
			zoom = 1.0f;
		}
		else if (metadonnee == "150%") {
			zoom = 1.5f;
		}
		else if (metadonnee == "200%") {
			zoom = 2.0f;
		}
		else if (metadonnee == "300%") {
			zoom = 3.0f;
		}

		auto scene = context.scene;
		auto objet = static_cast<Object *>(scene->active_node());
		auto graphe = objet->graph();
		graphe->zoom(zoom);
	}

	void defait() override {}
	void refait() override {}
};

/* ************************************************************************** */

class CommandeGrapheSupprimeSelection final : public Commande {
	void execute(Main */*main*/, const Context &context, const std::string &/*metadonnee*/) override
	{
		/* À FAIRE */
		auto scene = context.scene;
		auto objet = static_cast<Object *>(scene->active_node());
		auto graphe = objet->graph();

		graphe->deselectionne_tout();
	}

	bool evalue_predicat(Main */*main*/, const Context &context, const std::string &/*metadonnee*/) override
	{
		auto scene = context.scene;
		auto objet = static_cast<Object *>(scene->active_node());

		if (objet == nullptr) {
			return false;
		}

		/* Un objet est sélectionné, mais nous ne sommes pas en mode édition. */
		if (context.eval_ctx->edit_mode == false) {
			return true;
		}

		auto graphe = objet->graph();

		return !graphe->selection_vide();
	}

	void defait() override {}
	void refait() override {}
};

/* ************************************************************************** */

class CommandeGrapheCentre final : public Commande {
	void execute(Main */*main*/, const Context &context, const std::string &/*metadonnee*/) override
	{
		auto scene = context.scene;
		auto objet = static_cast<Object *>(scene->active_node());
		auto graphe = objet->graph();

		for (auto &noeud : graphe->noeuds()) {
			noeud->posx(0);
			noeud->posy(0);
		}
	}

	void defait() override {}
	void refait() override {}
};

/* ************************************************************************** */

class CommandeGrapheBasculeExpansion final : public Commande {
	void execute(Main */*main*/, const Context &context, const std::string &metadonnee) override
	{
		auto scene = context.scene;
		auto objet = static_cast<Object *>(scene->active_node());
		auto graphe = objet->graph();

		for (auto &noeud : graphe->noeuds()) {
			if (metadonnee == "contracte") {
				noeud->enleve_drapeau(NOEUD_DILATE);
				noeud->ajoute_drapeau(NOEUD_CONTRACTE);
			}
			else {
				noeud->enleve_drapeau(NOEUD_CONTRACTE);
				noeud->ajoute_drapeau(NOEUD_DILATE);
			}
		}
	}

	void defait() override {}
	void refait() override {}
};

/* ************************************************************************** */

void enregistre_commandes_graphes(UsineCommande *usine)
{
	usine->enregistre_type("dessine_graphe_objet",
						   description_commande<CommandeDessineGrapheObjet>(
							   "graphe", 0, 0, 0));

	usine->enregistre_type("dessine_graphe_dependance",
						   description_commande<CommandeDessineGrapheDependance>(
							   "graphe", 0, 0, 0));

	usine->enregistre_type("graphe.zoom",
						   description_commande<CommandeGrapheZoom>(
							   "graphe", 0, 0, 0));

	usine->enregistre_type("graphe.supprime_selection",
						   description_commande<CommandeGrapheSupprimeSelection>(
							   "graphe", 0, 0, 0));

	usine->enregistre_type("graphe.centre",
						   description_commande<CommandeGrapheCentre>(
							   "graphe", 0, 0, 0));

	usine->enregistre_type("graphe.bascule_expansion",
						   description_commande<CommandeGrapheBasculeExpansion>(
							   "graphe", 0, 0, 0));
}
