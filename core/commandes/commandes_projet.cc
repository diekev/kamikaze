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

#include "commandes_projet.h"

#include "kamikaze_main.h"
#include "sauvegarde.h"
#include "undo.h"

/* ************************************************************************** */

static void ouvre_fichier_implementation(Main *main, const Context &contexte, const std::string &chemin_projet)
{
	const auto erreur = kamikaze::ouvre_projet(chemin_projet, *main, contexte);

	if (erreur != kamikaze::erreur_fichier::AUCUNE_ERREUR) {
		switch (erreur) {
			case kamikaze::erreur_fichier::CORROMPU:
				main->affiche_erreur("Le fichier est corrompu !");
				break;
			case kamikaze::erreur_fichier::NON_OUVERT:
				main->affiche_erreur("Le fichier n'est pas ouvert !");
				break;
			case kamikaze::erreur_fichier::NON_TROUVE:
				main->affiche_erreur("Le fichier n'a pas été trouvé !");
				break;
			case kamikaze::erreur_fichier::INCONNU:
				main->affiche_erreur("Erreur inconnu !");
				break;
			case kamikaze::erreur_fichier::GREFFON_MANQUANT:
				main->affiche_erreur("Le fichier ne pas être ouvert car il"
									   " y a un greffon manquant !");
				break;
		}

		return;
	}

	main->chemin_projet(chemin_projet);
	main->projet_ouvert(true);

#if 0
	ajoute_fichier_recent(chemin_projet.c_str(), true);

	setWindowTitle(chemin_projet.c_str());
#endif
}

class CommandeOuvrir final : public Commande {
public:
	void execute(Main *main, const Context &contexte, const std::string &/*metadonnee*/) override
	{
		const auto chemin_projet = main->requiers_dialogue(FICHIER_OUVERTURE);

		if (chemin_projet.empty()) {
			return;
		}

		ouvre_fichier_implementation(main, contexte, chemin_projet);
	}

	void defait() override {}
	void refait() override {}
};

/* ************************************************************************** */

static void sauve_fichier_sous(Main *main, const Context &context)
{
	const auto &chemin_projet = main->requiers_dialogue(FICHIER_SAUVEGARDE);

	main->chemin_projet(chemin_projet);
	main->projet_ouvert(true);

	kamikaze::sauvegarde_projet(chemin_projet, *main, context.scene);
}

class CommandeSauvegarder final : public Commande {
public:
	void execute(Main *main, const Context &context, const std::string &/*metadonnee*/) override
	{
		if (main->projet_ouvert()) {
			kamikaze::sauvegarde_projet(main->chemin_projet(), *main, context.scene);
		}
		else {
			sauve_fichier_sous(main, context);
		}
	}

	void defait() override {}
	void refait() override {}
};

/* ************************************************************************** */

class CommandeSauvegarderSous final : public Commande {
public:
	void execute(Main *main, const Context &context, const std::string &/*metadonnee*/) override
	{
		sauve_fichier_sous(main, context);
	}

	void defait() override {}
	void refait() override {}
};

/* ************************************************************************** */

class CommandeDefaire final : public Commande {
public:
	void execute(Main *main, const Context &/*context*/, const std::string &/*metadonnee*/) override
	{
		/* À FAIRE */
		main->gestionnaire_commande()->defait();
	}

	void defait() override {}
	void refait() override {}
};

/* ************************************************************************** */

class CommandeRefaire final : public Commande {
public:
	void execute(Main *main, const Context &/*context*/, const std::string &/*metadonnee*/) override
	{
		/* À FAIRE */
		main->gestionnaire_commande()->refait();
	}

	void defait() override {}
	void refait() override {}
};

/* ************************************************************************** */

void enregistre_commandes_projet(CommandFactory *usine)
{
	ENREGISTRE_COMMANDE(usine, "ouvrir_fichier", CommandeOuvrir);
	ENREGISTRE_COMMANDE(usine, "sauvegarder", CommandeSauvegarder);
	ENREGISTRE_COMMANDE(usine, "sauvegarder_sous", CommandeSauvegarderSous);
	ENREGISTRE_COMMANDE(usine, "défaire", CommandeDefaire);
	ENREGISTRE_COMMANDE(usine, "refaire", CommandeRefaire);
}
