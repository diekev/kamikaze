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

#pragma once

#include "persona.h"

#include <set>
#include <unordered_map>

class Context;
class PrimitiveCollection;
class PriseEntree;
class Noeud;
class UsineOperateur;

extern "C" {

/**
 * @brief nouvel_operateur_kamikaze Interface pour enregistrer un nouvel
 * opérateur depuis un greffon. Il n'y pas de limite sur le nombre d'opérateurs
 * à enregistrer depuis un simple appel à cette fonction.
 *
 * @param usine L'usine dans laquelle enregistrer l'opérateur.
 */
void nouvel_operateur_kamikaze(UsineOperateur *usine);

}

/* ************************************************************************** */

/**
 * Enveloppe autour d'une PriseEntree pour restreindre l'interface de celle-ci.
 */
class EntreeOperateur {
	PriseEntree *m_prise = nullptr;

public:
	EntreeOperateur() = default;

	/**
	 * Construit une entrée d'opérateur avec la prise d'entrée de noeud donné en
	 * paramètre.
	 */
	EntreeOperateur(PriseEntree *prise);

	/**
	 * Exécute l'opérateur du noeud connecté à la prise d'entrée enveloppée dans
	 * cette classe et retourne un pointeur vers la collection résultante de
	 * l'exécution de l'opérateur. Si la collection passée en paramètre en
	 * non-nulle, le pointeur retourné sera un pointeur vers celle-ci.
	 */
	PrimitiveCollection *requiers_collection(
			PrimitiveCollection *collection,
			const Context &contexte,
			double temps);
};

/* ************************************************************************** */

/**
 * Type d'opérateur :
 * - STATIC : l'opérateur ne modifie pas les données à travers le temps.
 * - DYNAMIQUE : l'opérateur modifie les données à travers le temps, par exemple
 *               à travers une simulation de physique.
 */
enum type_operateur {
	STATIC    = 0,
	DYNAMIQUE = 1,
};

/**
 * L'Operateur enveloppe la logique de manipulation des Primitives. Chaque
 * opérateur se trouve dans son propre noeud. C'est à travers l'opérateur que le
 * graphe d'un objet est évalué de manière récursive en commençant l'évaluation
 * depuis l'opérateur du noeud actif et en remontant le graphe depuis les prises
 * d'entrées du noeud. Chaque opérateur est responsable d'appeler l'exécution
 * des noeuds se trouvant en son amont à travers la méthode virtuelle 'execute'.
 */
class Operateur : public Persona {
	int m_nombre_entrees = 0;
	int m_nombre_sorties = 0;
	bool m_besoin_execution = true;
	bool m_a_tampon = false;

	std::vector<EntreeOperateur> m_donnees_entree{};
	std::vector<std::string> m_avertissements{};

	double m_temps_execution = 0.0;
	double m_temps_agrege = 0.0;
	double m_min_temps_execution = std::numeric_limits<double>::max();
	double m_min_temps_agrege = std::numeric_limits<double>::max();

	int m_nombre_executions = 0;

protected:
	PrimitiveCollection *m_collection = nullptr;

public:
	/* Prévention de la création d'un opérateur sans un noeud. */
	Operateur() = delete;

	/**
	 * Constuit un opérateur dont le noeud passé en paramètre en est le parent.
	 */
	Operateur(Noeud *noeud, const Context &contexte);

	/**
	 * Détruit l'opérateur. Le destruteur détruit également la collection
	 * contenu dans l'opérateur.
	 */
	~Operateur();

	/**
	 * Retourne de type de cet opérateur.
	 */
	virtual type_operateur type() const;

	/**
	 * Retourne l'entrée se trouvent à l'index donné en paramètre.
	 */
	EntreeOperateur *entree(size_t index);

	/**
	 * Crée un certain nombre d'entrées.
	 */
	void entrees(size_t nombre);

	/**
	 * Retourne le nombre d'entrées de cet opérateur.
	 */
	int entrees() const;

	/**
	 * Crée un certain nombre de sorties.
	 */
	void sorties(size_t nombre);

	/**
	 * Retourne le nombre de sorties de cet opérateur.
	 */
	int sorties() const;

	/**
	 * Retourne le nom de l'entrée à l'index donné en paramètre.
	 */
	virtual const char *nom_entree(size_t /*index*/) { return ""; }

	/**
	 * Retourne le nom de la sortie à l'index donné en paramètre.
	 */
	virtual const char *nom_sortie(size_t /*index*/) { return ""; }

	/**
	 * Crée une prise d'entrée à l'index et avec la prise passée en paramètre.
	 */
	void donnee_entree(size_t index, PriseEntree *prise);

	/**
	 * Exécute cet opérateur dans le contexte et au temps passés en paramètre.
	 */
	virtual void execute(const Context &contexte, double temps) = 0;

	/**
	 * Retourne la collection de cet opérateur.
	 */
	PrimitiveCollection *collection() const;

	/**
	 * Retourne si oui ou non l'opérateur à besoin d'exécuter.
	 */
	bool besoin_execution() const;

	/**
	 * Force ou préviens l'exécution future de l'opérateur selon la valeur
	 * passée en paramètre.
	 */
	void besoin_execution(bool ouinon);

	/**
	 * Retourne le temps d'exécution agrégé de l'opérateur. Le temps d'exécution
	 * agrégé est le temps d'exécution du graphe depuis la racine jusqu'au noeud
	 * parent de l'opérateur.
	 */
	double temps_agrege() const;

	/**
	 * Met à jour le temps d'exécution agrégé de l'opérateur selon la valeur
	 * passée en paramètre.
	 */
	void temps_agrege(double temps);

	/**
	 * Retourne le temps d'exécution de l'opérateur.
	 */
	double temps_execution() const;

	/**
	 * Met à jour le temps d'exécution de l'opérateur selon la valeur passée en
	 * paramètre.
	 */
	void temps_execution(double temps);

	/**
	 * Retourne le temps d'exécution agrégé minimumde l'opérateur.
	 */
	double min_temps_agrege() const;

	/**
	 * Retourne le temps d'exécution minimum de l'opérateur.
	 */
	double min_temps_execution() const;

	/**
	 * Retourne le nombre de fois que l'opérateur a été exécuté.
	 */
	int nombre_executions() const;

	/**
	 * Incrémente de 1 le nombre de fois que l'opérateur a été exécuté.
	 */
	void incremente_nombre_execution();

	/**
	 * Ajoute un avertissement à la liste d'avertissements de cet opérateur.
	 */
	void ajoute_avertissement(const std::string &avertissement);

	/**
	 * Retourne la liste d'avertissements de cet opérateur.
	 */
	const std::vector<std::string> &avertissements() const;

	/**
	 * Retourne si oui ou non cet opérateur a des avertissements.
	 */
	bool a_avertissements() const;

	/**
	 * Supprime tous les avertissements de la liste d'avertissements de cet
	 * opérateur.
	 */
	void supprime_avertissements();

	/**
	 * Décide si oui ou non la collection devra être mis en tampon.
	 */
	void a_tampon(bool ouinon);

	/**
	 * Retourne si oui ou non la collection devra être mis en tampon.
	 */
	bool a_tampon() const;
};

/* ************************************************************************** */

/**
 * Exécute un opérateur dans le contexte et au temps passés en paramètre. Cette
 * fonction sert d'appel de base pour débuter l'exécution du graphe contenant le
 * noeud parent de l'opérateur. Tous les noeuds menant au parent de l'opérateur
 * seront exécutés de manière récursive, et les temps d'exécutions des noeuds
 * seront mis à jour.
 */
void execute_operateur(Operateur *operateur, const Context &contexte, double temps);

/* ************************************************************************** */

/**
 * Cette classe contient les informations pour un opérateur.
 */
struct DescOperateur {
	typedef Operateur *(*fonction_usine)(Noeud *, const Context &);

	std::string nom = "";
	std::string categorie = "";
	std::string text_aide = "";
	fonction_usine construction_operateur = nullptr;

	DescOperateur() = default;

	DescOperateur(
			const std::string &opnom,
			const std::string &ophelp,
			const std::string &opcategorie,
			fonction_usine func);
};

/* ************************************************************************** */

/**
 * Crée une description pour un opérateur.
 *
 * @param nom       Le nom de l'opérateur.
 * @param aide      Une description de comprendre ce que l'opérateur fait.
 * @param categorie La catégorie à laquelle appartient l'opérateur, utilisée
 *                  pour placer la description dans le bon menu.
 */
template <typename T>
static inline constexpr auto cree_description(const std::string &nom,
											  const std::string &aide,
											  const std::string &categorie)
{
	return DescOperateur(nom, aide, categorie,
						 [](Noeud *noeud, const Context &contexte) -> Operateur*
						 { return new T(noeud, contexte); });
}

/* ************************************************************************** */

/**
 * Une usine qui fabrique des opérateurs.
 */
class UsineOperateur final {
	std::unordered_map<std::string, DescOperateur> m_tableau;
	std::set<std::string> m_categories;

public:
	/**
	 * Enregistre un nouveau type d'opérateur dans l'usine.
	 *
	 * @param nom  Le nom de l'opérateur.
	 * @param desc La description de l'opération.
	 */
	size_t enregistre_type(const std::string &nom, DescOperateur desc);

	/**
	 * Retourne un pointeur vers un opérateur créé selon les paramètres. Le
	 * pointeur est considérer appartenir à l'appeleur et celui-ci sera
	 * responsable de libérer la mémoire allouée pour l'opérateur avec un appel
	 * vers 'delete'.
	 *
	 * @param nom      Le nom de l'opérateur à créer.
	 * @param noeud    Le noeud qui contiendra l'opérateur.
	 * @param contexte Le contexte dans lequel l'opérateur est créé.
	 */
	Operateur *operator()(const std::string &nom, Noeud *noeud, const Context &contexte);

	/**
	 * Retourne le nombre d'entrées dans le tableau de l'usine.
	 */
	inline size_t nombre_entrees() const
	{
		return m_tableau.size();
	}

	/**
	 * Retourne l'ensemble de catégories connues de l'usine.
	 */
	const std::set<std::string> &categories() const;

	/**
	 * Retourne un vecteur contenant les descriptions présentent dans le tableau
	 * de l'usine dont la catégorie correspond à la catégorie passer en
	 * paramètre. Retourne un vecteur vide si aucune description n'a de
	 * catégorie correspondante.
	 */
	std::vector<DescOperateur> cles(const std::string &categorie) const;

	/**
	 * Retourne 'true' si la clé donnée en paramètre est enregistrée dans le
	 * tableau de l'usine. Sinon, retourne 'false'.
	 */
	bool est_enregistre(const std::string &cle) const;
};
