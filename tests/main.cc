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

#include <girafeenfeu/test_unitaire/test_unitaire.h>

#include "core/kamikaze_main.h"
#include "core/sauvegarde.h"
#include "core/scene.h"

void test_lecture_fichier(ControleurUnitaire &controleur)
{
	Main racine;
	racine.initialize();
	racine.charge_greffons();

	auto scene = Scene();

	auto contexte = Context();
	contexte.scene = &scene;
	contexte.primitive_factory = racine.primitive_factory();
	contexte.usine_operateur = racine.usine_operateur();

	auto erreur = kamikaze::ouvre_projet("projets/projet_1_objet.kmkz", contexte);

	CU_VERIFIE_CONDITION(controleur, erreur == kamikaze::erreur_fichier::AUCUNE_ERREUR);
	CU_VERIFIE_CONDITION(controleur, scene.nodes().size() == 1);

	erreur = kamikaze::ouvre_projet("projets/projet_corrompu.kmkz", contexte);

	CU_VERIFIE_CONDITION(controleur, erreur == kamikaze::erreur_fichier::CORROMPU);
	CU_VERIFIE_CONDITION(controleur, scene.nodes().size() == 0);

	erreur = kamikaze::ouvre_projet("projets/projet_introuvable.kmkz", contexte);

	CU_VERIFIE_CONDITION(controleur, erreur == kamikaze::erreur_fichier::NON_TROUVE);
}

int main()
{
	ControleurUnitaire controlleur;

	controlleur.ajoute_fonction(test_lecture_fichier);

	controlleur.performe_controles();
	controlleur.imprime_resultat();

	return 0;
}
