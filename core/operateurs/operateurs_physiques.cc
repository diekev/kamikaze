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

		synchronise_donnees();

		/* Sauvegarde la collection */
		delete m_derniere_collection;
		m_derniere_collection = m_collection->copy();
	}

	virtual void execute_algorithme(const Context &contexte, double temps) = 0;

	virtual bool initialise_donnees() = 0;

	virtual void synchronise_donnees() = 0;
};

/* ************************************************************************** */

static const char *NOM_GRAVITE = "Gravité";
static const char *AIDE_GRAVITE = "Applique une force de gravité aux primitives d'entrées.";

class OperateurGravite final : public OperateurPhysique {
public:
	OperateurGravite(Noeud *noeud, const Context &contexte)
		: OperateurPhysique(noeud, contexte)
	{
		add_prop("élasticité", "Élasticité", property_type::prop_float);
		set_prop_min_max(0.0f, 1.0f);
		set_prop_default_value_float(1.0f);
		set_prop_tooltip("Coefficient déterminant la perte d'énergie lors d'une collision.\n"
						 "Une valeur de zéro indique que la particle perd toute son énergie et s'arrête,\n"
						 "alors qu'une valeur de un indique que la particle garde toute son énergie et ne s'arrête jamais.");
	}

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
		const auto elasticite = eval_float("élasticité");

		for (Primitive *prim : primitive_iterator(m_collection, PrimPoints::id)) {
			auto nuage_points = static_cast<PrimPoints *>(prim);
			auto points = nuage_points->points();
			auto nombre_points = points->size();

			auto attr_vel = nuage_points->add_attribute("velocité", ATTR_TYPE_VEC3, nombre_points);

			for (auto i = 0ul; i < nombre_points; ++i) {
				const auto acceleration = m_gravite;

				/* velocite = acceleration * temp_par_image + velocite */
				const auto velocite = attr_vel->vec3(i) + acceleration * temps_par_image;
				attr_vel->vec3(i, velocite);

				/* position = velocite * temps_par_image + position */
				auto &point = (*points)[i];
				point += velocite * temps_par_image;

				/* Calcul la position en espace objet. */
				const auto pos = nuage_points->matrix() * point;

				/* Vérifie l'existence d'une collision avec le plan global. */
				if (verifie_collision(plan_global, pos, velocite)) {
					attr_vel->vec3(i, -elasticite * velocite);
				}
			}
		}
	}

	void synchronise_donnees() override {}
};

/* ************************************************************************** */

static const char *NOM_MASSE_RESSORT = "Masse-Ressort";
static const char *AIDE_MASSE_RESSORT = "Résoud un système de masse-ressort depuis des courbes.";

struct MasseRessort {
	MasseRessort *precedent = nullptr;
	MasseRessort *suivant = nullptr;

	glm::vec3 position_repos = glm::vec3(0.0);
	glm::vec3 position = glm::vec3(0.0);
	glm::vec3 velocite = glm::vec3(0.0);

	MasseRessort() = default;
};

struct DonneesSysteme {
	float masse = 0.0f;
	float masse_inverse = 0.0f;
	float rigidite = 0.0f;
	float amortissement = 0.0f;
	float temps_par_image = 0.0f;
	glm::vec3 gravite = glm::vec3(0.0);

	DonneesSysteme() = default;
};

static void resoud_masse_ressort(MasseRessort *masse_ressort, const DonneesSysteme &donnees)
{
	const auto precedent = masse_ressort->precedent;

	/* force = masse * acceleration */
	auto force = donnees.masse * donnees.gravite;

	/* Ajout d'une force de ressort selon la loi de Hooke :
	 * f = -k * déplacement */
	auto force_ressort = -donnees.rigidite * (masse_ressort->position - precedent->position);
	force += force_ressort;

	/* Amortissement : retrait de la vélocité selon le coefficient
	 * d'amortissement. */
	auto force_amortisseur = masse_ressort->velocite * donnees.amortissement;
	force -= force_amortisseur;

	/* acceleration = force / masse */
	auto acceleration = force * donnees.masse_inverse;

	masse_ressort->velocite = masse_ressort->velocite + acceleration * donnees.temps_par_image;
	masse_ressort->position = masse_ressort->position + masse_ressort->velocite * donnees.temps_par_image;

	if (masse_ressort->suivant != nullptr) {
		resoud_masse_ressort(masse_ressort->suivant, donnees);
	}
}

class OperateurMasseRessort final : public OperateurPhysique {
	std::vector<MasseRessort *> m_masses_ressorts;
	std::vector<MasseRessort *> m_racines;

public:
	OperateurMasseRessort(Noeud *noeud, const Context &contexte);

	~OperateurMasseRessort();

	const char *nom() override;

	void supprime_donnees();

	bool initialise_donnees() override;

	void execute_algorithme(const Context &contexte, double temps) override;

	void synchronise_donnees();
};

OperateurMasseRessort::OperateurMasseRessort(Noeud *noeud, const Context &contexte)
	: OperateurPhysique(noeud, contexte)
{}

OperateurMasseRessort::~OperateurMasseRessort()
{
	supprime_donnees();
}

const char *OperateurMasseRessort::nom()
{
	return NOM_MASSE_RESSORT;
}

void OperateurMasseRessort::supprime_donnees()
{
	for (auto &masse_ressort : m_masses_ressorts) {
		delete masse_ressort;
	}

	m_masses_ressorts.clear();
	m_racines.clear();
}

bool OperateurMasseRessort::initialise_donnees()
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
			masse_ressort->position = masse_ressort->position_repos;

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

void OperateurMasseRessort::execute_algorithme(const Context &/*contexte*/, double /*temps*/)
{
	DonneesSysteme donnees;
	donnees.gravite = m_gravite;
	donnees.amortissement = 1.0f;
	donnees.masse = 5.0f;
	donnees.masse_inverse = 1.0f / donnees.masse;
	donnees.rigidite = 10.0f;
	donnees.temps_par_image = 1.0f / 24.0f;

	for (auto &racine : m_racines) {
		resoud_masse_ressort(racine->suivant, donnees);
	}
}

void OperateurMasseRessort::synchronise_donnees()
{
	auto iterateur_courbes = primitive_iterator(m_collection, SegmentPrim::id);

	auto primitive_courbes = static_cast<SegmentPrim *>(iterateur_courbes.get());
	auto liste_points = primitive_courbes->points();

	for (size_t i = 0; i < m_masses_ressorts.size(); ++i) {
		(*liste_points)[i] = m_masses_ressorts[i]->position;
	}
}

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
