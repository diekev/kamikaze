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

#include "operateurs_physiques.h"

#include <kamikaze/operateur.h>
#include <kamikaze/prim_points.h>
#include <kamikaze/segmentprim.h>

#include <kamikaze/outils/mathématiques.h>

/* ************************************************************************** */

struct PlanPhysique {
	glm::vec3 pos = glm::vec3{0.0f, 0.0f, 0.0f};
	glm::vec3 nor = glm::vec3{0.0f, 1.0f, 0.0f};
};

PlanPhysique plan_global;

static bool verifie_collision(const PlanPhysique &plan, const glm::vec3 &pos, const glm::vec3 &vel)
{
	const auto &XPdotN = glm::dot(pos - plan.pos, plan.nor);

	/* Est-on à une distance epsilon du plan ? */
	if (XPdotN >= std::numeric_limits<float>::epsilon()) {
		return false;
	}

	/* Va-t-on vers le plan ? */
	if (glm::dot(plan.nor, vel) >= 0.0f) {
		return false;
	}

	return true;
}

/* ************************************************************************** */

/**
 * La classe OperateurPhysique représente la classe de base pour tous les
 * opérateur faisant des simulations de physique.
 */
class OperateurPhysique : public Operateur {
protected:
	glm::vec3 m_gravite = glm::vec3{0.0f, -9.80665f, 0.0f};
	PrimitiveCollection *m_collection_original = nullptr;
	PrimitiveCollection *m_derniere_collection = nullptr;
	int m_image_debut = 0;

public:
	OperateurPhysique(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);
	}

	virtual ~OperateurPhysique()
	{
		delete m_collection_original;
		delete m_derniere_collection;
	}

	const char *nom_entree(size_t /*index*/) override
	{
		return "Entrée";
	}

	const char *nom_sortie(size_t /*index*/) override
	{
		return "Sortie";
	}

	void execute(const Context &contexte, double temps) override
	{
		if (temps == m_image_debut) {
			m_collection->free_all();
			entree(0)->requiers_collection(m_collection, contexte, temps);

			delete m_collection_original;
			m_collection_original = m_collection->copy();

			if (!initialise_donnees()) {
				return;
			}
		}
		else {
			m_collection = m_derniere_collection->copy();
		}

		execute_algorithme(contexte, temps);

		/* Sauvegarde la collection */
		delete m_derniere_collection;
		m_derniere_collection = m_collection->copy();
	}

	virtual void execute_algorithme(const Context &contexte, double temps) = 0;

	virtual bool initialise_donnees() = 0;
};

/* ************************************************************************** */

static const char *NOM_GRAVITE = "Gravité";
static const char *AIDE_GRAVITE = "Applique une force de gravité aux primitives d'entrées.";

class OperateurGravite final : public OperateurPhysique {
	glm::vec3 m_gravite = glm::vec3{0.0f, -9.80665f, 0.0f};
	PrimitiveCollection *m_collection_original = nullptr;
	PrimitiveCollection *m_derniere_collection = nullptr;
	int m_image_debut = 0;

public:
	OperateurGravite(Noeud *noeud, const Context &contexte)
		: OperateurPhysique(noeud, contexte)
	{}

	~OperateurGravite() = default;

	const char *nom() override
	{
		return NOM_GRAVITE;
	}

	bool initialise_donnees() override
	{
		return true;
	}

	void execute_algorithme(const Context &/*contexte*/, double /*temps*/) override
	{
		/* À FAIRE : passe le temps par image en paramètre. */
		const auto temps_par_image = 1.0f / 24.0f;
		const auto gravite = m_gravite * temps_par_image;

		for (Primitive *prim : primitive_iterator(m_collection, PrimPoints::id)) {
			auto nuage_points = static_cast<PrimPoints *>(prim);
			auto points = nuage_points->points();
			auto nombre_points = points->size();

			auto attr_vel = nuage_points->add_attribute("velocité", ATTR_TYPE_VEC3, nombre_points);

			for (auto i = 0ul; i < nombre_points; ++i) {
				auto &point = (*points)[i];
				const auto velocite = attr_vel->vec3(i) + gravite;
				attr_vel->vec3(i, velocite);

				point += velocite;

				/* Calcul la position en espace objet. */
				const auto pos = nuage_points->matrix() * point;

				/* Vérifie l'existence d'une collision avec le plan global. */
				if (verifie_collision(plan_global, pos, velocite)) {
					attr_vel->vec3(i, -velocite);
				}
			}
		}
	}
};

/* ************************************************************************** */

static const char *NOM_MASSE_RESSORT = "Masse-Ressort";
static const char *AIDE_MASSE_RESSORT = "Résoud un système de masse-ressort depuis des courbes.";

struct MasseRessort {
	MasseRessort *precedent = nullptr;
	MasseRessort *suivant = nullptr;

	glm::vec3 position_repos = glm::vec3(0.0);
	glm::vec3 position_courante = glm::vec3(0.0);
	glm::vec3 velocite = glm::vec3(0.0);
	glm::vec3 accelaration = glm::vec3(0.0);

	MasseRessort() = default;
};

struct DonneesSysteme {
	float masse = 0.0f;
	float rigidite = 0.0f;
	float amortissement = 0.0f;
	float temps_par_image = 0.0f;

	DonneesSysteme() = default;
};

static void resoud_masse_ressort(MasseRessort *racine, const DonneesSysteme &donnees)
{

}

class OperateurMasseRessort : public OperateurPhysique {
	std::vector<MasseRessort *> m_masses_ressorts;
	std::vector<MasseRessort *> m_racines;

public:
	OperateurMasseRessort(Noeud *noeud, const Context &contexte)
		: OperateurPhysique(noeud, contexte)
	{}

	~OperateurMasseRessort()
	{
		supprime_donnees();
	}

	const char *nom() override
	{
		return NOM_MASSE_RESSORT;
	}

	void supprime_donnees()
	{
		for (auto &masse_ressort : m_masses_ressorts) {
			delete masse_ressort;
		}

		m_masses_ressorts.clear();
		m_racines.clear();
	}

	bool initialise_donnees() override
	{
		supprime_donnees();

		auto iterateur_courbes = primitive_iterator(m_collection, SegmentPrim::id);

		if (iterateur_courbes.get() == nullptr) {
			ajoute_avertissement("Il n'y a pas de primitive à segments en entrée !");
			return false;
		}

		auto primitive_courbes = static_cast<SegmentPrim *>(iterateur_courbes.get());

		const auto liste_points = primitive_courbes->points();
		const auto nombre_courbes = primitive_courbes->nombre_courbes();
		const auto points_par_courbes = primitive_courbes->points_par_courbe();

		m_masses_ressorts.reserve(nombre_courbes * points_par_courbes);
		m_racines.reserve(nombre_courbes);

		MasseRessort *dernier = nullptr;

		for (size_t i = 0, index = 0; i < nombre_courbes; ++i) {
			for (size_t j = 0; j < points_par_courbes; ++j, ++index) {
				auto masse_ressort = new MasseRessort;
				masse_ressort->position_repos = (*liste_points)[index];

				m_masses_ressorts.push_back(masse_ressort);

				if (j == 0) {
					m_racines.push_back(masse_ressort);
				}
				else {
					masse_ressort->precedent = dernier;
					dernier->suivant = masse_ressort;
				}

				dernier = masse_ressort;
			}
		}

		return true;
	}

	void execute_algorithme(const Context &contexte, double temps) override
	{
		for (auto &racine : m_racines) {

		}
	}
};

/* ************************************************************************** */

void enregistre_operateurs_physiques(UsineOperateur *usine)
{
	const auto categorie = "Physique";

	usine->enregistre_type(
				NOM_GRAVITE,
				cree_description<OperateurGravite>(
					NOM_GRAVITE,
					AIDE_GRAVITE,
					categorie));

	usine->enregistre_type(
				NOM_MASSE_RESSORT,
				cree_description<OperateurMasseRessort>(
					NOM_MASSE_RESSORT,
					AIDE_MASSE_RESSORT,
					categorie));
}
