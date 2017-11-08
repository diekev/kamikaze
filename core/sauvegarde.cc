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

#include "sauvegarde.h"

#include <iostream>
#include <sstream>

#include <kamikaze/operateur.h>

#include "interne/tinyxml2.h"

#include "scene.h"

namespace kamikaze {

static std::string id_depuis_pointeur(void *pointeur)
{
	std::stringstream ss;
	ss << pointeur;
	return ss.str();
}

static void sauvegarde_proprietes(
		tinyxml2::XMLDocument &doc,
		tinyxml2::XMLElement *element,
		Persona *persona)
{
	tinyxml2::XMLElement *racine_propriete = doc.NewElement("proprietes");
	element->InsertEndChild(racine_propriete);

	for (const auto &prop : persona->props()) {
		auto element_prop = doc.NewElement("propriete");
		element_prop->SetAttribute("nom", prop.name.c_str());
		element_prop->SetAttribute("min", prop.min);
		element_prop->SetAttribute("max", prop.max);
		element_prop->SetAttribute("visible", prop.visible);
		element_prop->SetAttribute("type", static_cast<int>(prop.type));

		auto element_donnees = doc.NewElement("donnees");

		switch (prop.type) {
			case property_type::prop_bool:
			{
				auto donnees = std::experimental::any_cast<bool>(prop.data);
				element_donnees->SetAttribute("valeur", donnees);
				break;
			}
			case property_type::prop_enum:
			case property_type::prop_int:
			{
				auto donnees = std::experimental::any_cast<int>(prop.data);
				element_donnees->SetAttribute("valeur", donnees);
				break;
			}
			case property_type::prop_float:
			{
				auto donnees = std::experimental::any_cast<float>(prop.data);
				element_donnees->SetAttribute("valeur", donnees);
				break;
			}
			case property_type::prop_vec3:
			{
				glm::vec3 donnees = std::experimental::any_cast<glm::vec3>(prop.data);

				element_donnees->SetAttribute("valeurx", donnees.x);
				element_donnees->SetAttribute("valeury", donnees.y);
				element_donnees->SetAttribute("valeurz", donnees.z);
				break;
			}
			case property_type::prop_list:
			case property_type::prop_output_file:
			case property_type::prop_input_file:
			case property_type::prop_string:
			{
				std::string donnees = std::experimental::any_cast<std::string>(prop.data);
				element_donnees->SetAttribute("valeur", donnees.c_str());
				break;
			}
		}

		element_prop->InsertEndChild(element_donnees);

		racine_propriete->InsertEndChild(element_prop);
	}
}

void sauvegarde_projet(const std::string &chemin, const Scene *scene)
{
	tinyxml2::XMLDocument doc;
	doc.InsertFirstChild(doc.NewDeclaration());

	tinyxml2::XMLElement *racine_scene = doc.NewElement("scene");
	racine_scene->SetAttribute("image_courante", scene->currentFrame());
	racine_scene->SetAttribute("image_depart", scene->startFrame());
	racine_scene->SetAttribute("image_fin", scene->endFrame());
	racine_scene->SetAttribute("ips", scene->framesPerSecond());
	racine_scene->SetAttribute("drapeaux", scene->flags());

	doc.InsertEndChild(racine_scene);

	tinyxml2::XMLElement *racine_objet = doc.NewElement("objets");
	racine_scene->InsertEndChild(racine_objet);

	/* Écriture des objets. */
	for (const auto &noeud_scene : scene->nodes()) {
		const auto objet = static_cast<Object *>(noeud_scene.get());

		auto element_objet = doc.NewElement("objet");
		element_objet->SetAttribute("nom", objet->name().c_str());
		element_objet->SetAttribute("posx", objet->xpos());
		element_objet->SetAttribute("posy", objet->ypos());
		element_objet->SetAttribute("drapeaux", objet->flags());

		sauvegarde_proprietes(doc, element_objet, objet);

		tinyxml2::XMLElement *racine_graphe = doc.NewElement("graphe");
		element_objet->InsertEndChild(racine_graphe);

		for (const auto &noeud : objet->graph()->noeuds()) {
			/* Noeud */
			auto element_noeud = doc.NewElement("noeud");
			element_noeud->SetAttribute("nom", noeud->nom().c_str());
			element_noeud->SetAttribute("drapeaux", noeud->drapeaux());
			element_noeud->SetAttribute("posx", noeud->posx());
			element_noeud->SetAttribute("posy", noeud->posy());
			element_noeud->SetAttribute("est_sortie", noeud.get() == objet->graph()->sortie());

			/* Prises d'entrée. */
			auto racine_prise_entree = doc.NewElement("prises_entree");

			for (const auto &prise : noeud->entrees()) {
				auto element_prise = doc.NewElement("entree");
				element_prise->SetAttribute("nom", prise->nom.c_str());
				element_prise->SetAttribute("id", id_depuis_pointeur(prise).c_str());
				element_prise->SetAttribute("connection", id_depuis_pointeur(prise->lien).c_str());

				racine_prise_entree->InsertEndChild(element_prise);
			}

			element_noeud->InsertEndChild(racine_prise_entree);

			/* Prises de sortie */
			auto racine_prise_sortie = doc.NewElement("prises_sortie");

			for (const auto &prise : noeud->sorties()) {
				/* REMARQUE : par optimisation on ne sauvegarde que les
				 * connections depuis les prises d'entrées. */
				auto element_prise = doc.NewElement("sortie");
				element_prise->SetAttribute("nom", prise->nom.c_str());
				element_prise->SetAttribute("id", id_depuis_pointeur(prise).c_str());

				racine_prise_sortie->InsertEndChild(element_prise);
			}

			element_noeud->InsertEndChild(racine_prise_sortie);

			/* Opérateur */
			auto operateur = noeud->operateur();
			auto element_operateur = doc.NewElement("operateur");
			element_operateur->SetAttribute("nom", operateur->nom());

			sauvegarde_proprietes(doc, element_operateur, operateur);

			element_noeud->InsertEndChild(element_operateur);
			racine_graphe->InsertEndChild(element_noeud);
		}

		racine_objet->InsertEndChild(element_objet);
	}

	const auto resultat = doc.SaveFile(chemin.c_str());

	if (resultat != tinyxml2::XML_SUCCESS) {
		std::cerr << "Une erreur s'est produite lors de l'écriture du fichier !\n";
	}
}

/* ************************************************************************** */

static void lecture_propriete(
		tinyxml2::XMLElement *element,
		Persona *persona)
{
	const auto type_prop = element->Attribute("type");
	const auto nom_prop = element->Attribute("nom");

	const auto element_donnees = element->FirstChildElement("donnees");

	switch (static_cast<property_type>(atoi(type_prop))) {
		case property_type::prop_bool:
		{
			const auto donnees = element_donnees->Attribute("valeur");
			persona->valeur_propriete_bool(nom_prop, atoi(donnees));
			break;
		}
		case property_type::prop_enum:
		case property_type::prop_int:
		{
			const auto donnees = element_donnees->Attribute("valeur");
			persona->valeur_propriete_int(nom_prop, atoi(donnees));
			break;
		}
		case property_type::prop_float:
		{
			const auto donnees = element_donnees->Attribute("valeur");
			persona->valeur_propriete_float(nom_prop, atof(donnees));
			break;
		}
		case property_type::prop_vec3:
		{
			const auto donnee_x = atof(element_donnees->Attribute("valeurx"));
			const auto donnee_y = atof(element_donnees->Attribute("valeury"));
			const auto donnee_z = atof(element_donnees->Attribute("valeurz"));
			const auto donnees = glm::vec3{donnee_x, donnee_y, donnee_z};
			persona->valeur_propriete_vec3(nom_prop, donnees);
			break;
		}
		case property_type::prop_list:
		case property_type::prop_output_file:
		case property_type::prop_input_file:
		case property_type::prop_string:
		{
			const auto donnees = element_donnees->Attribute("valeur");
			persona->valeur_propriete_string(nom_prop, donnees);
			break;
		}
	}
}

static void lecture_proprietes(
		tinyxml2::XMLElement *element,
		Persona *persona)
{
	const auto racine_propriete = element->FirstChildElement("proprietes");
	auto element_propriete = racine_propriete->FirstChildElement("propriete");

	for (; element_propriete != nullptr; element_propriete = element_propriete->NextSiblingElement("propriete")) {
		lecture_propriete(element_propriete, persona);
	}
}

struct DonneesConnections {
	/* Tableau faisant correspondre les ids des prises connectées entre elles.
	 * La clé du tableau est l'id de la prise d'entrée, la valeur, celle de la
	 * prise de sortie. */
	std::unordered_map<std::string, std::string> tableau_connection_id;

	std::unordered_map<std::string, PriseEntree *> tableau_id_prise_entree;
	std::unordered_map<std::string, PriseSortie *> tableau_id_prise_sortie;
};

static void lecture_noeud(
		tinyxml2::XMLElement *element_noeud,
		const Context &contexte,
		Object *objet,
		DonneesConnections &donnees_connection)
{
	const auto nom_noeud = element_noeud->Attribute("nom");
	const auto posx = element_noeud->Attribute("posx");
	const auto posy = element_noeud->Attribute("posy");
	const auto drapeaux = element_noeud->Attribute("drapeaux");
	const auto est_sortie = element_noeud->Attribute("est_sortie");

	Noeud *noeud;

	if (atoi(est_sortie)) {
		noeud = objet->graph()->sortie();
	}
	else {
		noeud = new Noeud;
		noeud->nom(nom_noeud);

		const auto element_operateur = element_noeud->FirstChildElement("operateur");
		const auto nom_operateur = element_operateur->Attribute("nom");

		Operateur *operateur = (*contexte.usine_operateur)(nom_operateur, noeud, contexte);
		lecture_proprietes(element_operateur, operateur);
		noeud->synchronise_donnees();

		objet->ajoute_noeud(noeud);
	}

	noeud->posx(atoi(posx));
	noeud->posy(atoi(posy));
	noeud->ajoute_drapeau(atoi(drapeaux));

	const auto racine_prise_entree = element_noeud->FirstChildElement("prises_entree");
	auto element_prise_entree = racine_prise_entree->FirstChildElement("entree");

	for (; element_prise_entree != nullptr; element_prise_entree = element_prise_entree->NextSiblingElement("entree")) {
		const auto nom_prise = element_prise_entree->Attribute("nom");
		const auto id_prise = element_prise_entree->Attribute("id");
		const auto connection = element_prise_entree->Attribute("connection");

		donnees_connection.tableau_connection_id[id_prise] = connection;
		donnees_connection.tableau_id_prise_entree[id_prise] = noeud->entree(nom_prise);
	}

	const auto racine_prise_sortie = element_noeud->FirstChildElement("prises_sortie");
	auto element_prise_sortie = racine_prise_sortie->FirstChildElement("sortie");

	for (; element_prise_sortie != nullptr; element_prise_sortie = element_prise_sortie->NextSiblingElement("sortie")) {
		const auto nom_prise = element_prise_sortie->Attribute("nom");
		const auto id_prise = element_prise_sortie->Attribute("id");

		donnees_connection.tableau_id_prise_sortie[id_prise] = noeud->sortie(nom_prise);
	}
}

static void lecture_graphe(
		tinyxml2::XMLElement *element_objet,
		const Context &contexte,
		Object *objet)
{
	auto racine_graphe = element_objet->FirstChildElement("graphe");
	auto element_noeud = racine_graphe->FirstChildElement("noeud");

	DonneesConnections donnees_connections;

	for (; element_noeud != nullptr; element_noeud = element_noeud->NextSiblingElement("noeud")) {
		lecture_noeud(element_noeud, contexte, objet, donnees_connections);
	}

	/* Création des connections. */
	for (const auto &connection : donnees_connections.tableau_connection_id) {
		const auto &id_de = connection.second;
		const auto &id_a = connection.first;

		const auto &pointer_de = donnees_connections.tableau_id_prise_sortie[id_de];
		const auto &pointer_a = donnees_connections.tableau_id_prise_entree[id_a];

		if (pointer_de && pointer_a) {
			objet->graph()->connecte(pointer_de, pointer_a);
		}
	}
}

void ouvre_projet(const std::string &chemin, const Context &contexte)
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile(chemin.c_str());

	Scene *scene = contexte.scene;
	scene->supprime_tout();

	/* Lecture de la scène. */
	const auto racine_scene = doc.FirstChildElement("scene");
	const auto image_depart = racine_scene->Attribute("image_depart");
	const auto image_fin = racine_scene->Attribute("image_fin");
	const auto image_courante = racine_scene->Attribute("image_courante");
	const auto ips = racine_scene->Attribute("ips");

	scene->currentFrame(atoi(image_courante));
	scene->startFrame(atoi(image_depart));
	scene->endFrame(atoi(image_fin));
	scene->framesPerSecond(atof(ips));

	/* Lecture des objets. */
	const auto racine_objets = racine_scene->FirstChildElement("objets");
	auto element_objet = racine_objets->FirstChildElement("objet");

	for (; element_objet != nullptr; element_objet = element_objet->NextSiblingElement("objet")) {
		const auto nom_objet = element_objet->Attribute("nom");
		const auto posx = element_objet->Attribute("posx");
		const auto posy = element_objet->Attribute("posy");
		const auto drapeaux = element_objet->Attribute("drapeaux");

		Object *objet = new Object(contexte);
		objet->name(nom_objet);
		objet->xpos(atoi(posx));
		objet->ypos(atoi(posy));
		objet->set_flags(atoi(drapeaux));

		lecture_proprietes(element_objet, objet);

		scene->addObject(objet);

		/* Lecture du graphe. */
		lecture_graphe(element_objet, contexte, objet);

		objet->updateMatrix();
	}

	scene->updateForNewFrame(contexte);
}

}  /* namespace kamikaze */
