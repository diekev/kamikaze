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

void enregistre_commandes_graphes(CommandFactory *usine)
{
	ENREGISTRE_COMMANDE(usine, "dessine_graphe_objet", CommandeDessineGrapheObjet);
	ENREGISTRE_COMMANDE(usine, "dessine_graphe_dependance", CommandeDessineGrapheDependance);
}
