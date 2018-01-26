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
#if 0
		QMessageBox boite_message;

		switch (erreur) {
			case kamikaze::erreur_fichier::CORROMPU:
				boite_message.critical(nullptr, "Error", "Le fichier est corrompu !");
				break;
			case kamikaze::erreur_fichier::NON_OUVERT:
				boite_message.critical(nullptr, "Error", "Le fichier n'est pas ouvert !");
				break;
			case kamikaze::erreur_fichier::NON_TROUVE:
				boite_message.critical(nullptr, "Error", "Le fichier n'a pas été trouvé !");
				break;
			case kamikaze::erreur_fichier::INCONNU:
				boite_message.critical(nullptr, "Error", "Erreur inconnu !");
				break;
			case kamikaze::erreur_fichier::GREFFON_MANQUANT:
				boite_message.critical(nullptr, "Error",
									   "Le fichier ne pas être ouvert car il"
									   " y a un greffon manquant !");
				break;
		}

		boite_message.setFixedSize(500, 200);
#endif
		return;
	}

	main->chemin_projet(chemin_projet);
	main->projet_ouvert(true);

#if 0
	ajoute_fichier_recent(chemin_projet.c_str(), true);

	setWindowTitle(chemin_projet.c_str());
#endif
}

class CommandeOuvrir final : public Command {
public:
	void execute(Main *main, const Context &contexte) override
	{
		const auto chemin_projet = main->requiers_dialogue(FICHIER_OUVERTURE);

		if (chemin_projet.empty()) {
			return;
		}

		ouvre_fichier_implementation(main, contexte, chemin_projet);
	}

	void undo() override {}
	void redo() override {}
};

/* ************************************************************************** */

static void sauve_fichier_sous(Main *main, const Context &context)
{
	const auto &chemin_projet = main->requiers_dialogue(FICHIER_SAUVEGARDE);

	main->chemin_projet(chemin_projet);
	main->projet_ouvert(true);

	kamikaze::sauvegarde_projet(chemin_projet, *main, context.scene);
}

class CommandeSauvegarder final : public Command {
public:
	void execute(Main *main, const Context &context) override
	{
		if (main->projet_ouvert()) {
			kamikaze::sauvegarde_projet(main->chemin_projet(), *main, context.scene);
		}
		else {
			sauve_fichier_sous(main, context);
		}
	}

	void undo() override {}
	void redo() override {}
};

/* ************************************************************************** */

class CommandeSauvegarderSous final : public Command {
public:
	void execute(Main *main, const Context &context) override
	{
		sauve_fichier_sous(main, context);
	}

	void undo() override {}
	void redo() override {}
};

/* ************************************************************************** */

class CommandeDefaire final : public Command {
public:
	void execute(Main *main, const Context &/*context*/) override
	{
		/* À FAIRE */
		main->gestionnaire_commande()->undo();
	}

	void undo() override {}
	void redo() override {}
};

/* ************************************************************************** */

class CommandeRefaire final : public Command {
public:
	void execute(Main *main, const Context &/*context*/) override
	{
		/* À FAIRE */
		main->gestionnaire_commande()->redo();
	}

	void undo() override {}
	void redo() override {}
};

/* ************************************************************************** */

void enregistre_commandes_projet(CommandFactory *usine)
{
	REGISTER_COMMAND(usine, "ouvrir_fichier", CommandeOuvrir);
	REGISTER_COMMAND(usine, "sauvegarder", CommandeSauvegarder);
	REGISTER_COMMAND(usine, "sauvegarder_sous", CommandeSauvegarderSous);
	REGISTER_COMMAND(usine, "défaire", CommandeDefaire);
	REGISTER_COMMAND(usine, "refaire", CommandeRefaire);
}
