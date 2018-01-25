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

#include <kamikaze/outils/mathématiques.h>

/* ************************************************************************** */

static const char *NOM_GRAVITE = "Gravité";
static const char *AIDE_GRAVITE = "Applique une force de gravité aux primitives d'entrées.";

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

class OperateurGravite : public Operateur {
	glm::vec3 m_gravite = glm::vec3{0.0f, -9.80665f, 0.0f};
	PrimitiveCollection *m_collection_original = nullptr;
	PrimitiveCollection *m_derniere_collection = nullptr;
	int m_image_debut = 0;

public:
	OperateurGravite(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);
	}

	~OperateurGravite()
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

	const char *nom() override
	{
		return NOM_GRAVITE;
	}

	void execute(const Context &contexte, double temps) override
	{
		if (temps == m_image_debut) {
			m_collection->free_all();
			entree(0)->requiers_collection(m_collection, contexte, temps);

			delete m_collection_original;
			m_collection_original = m_collection->copy();
		}
		else {
			m_collection = m_derniere_collection->copy();
		}

		execute_algorithme(contexte, temps);

		/* Sauvegarde la collection */
		delete m_derniere_collection;
		m_derniere_collection = m_collection->copy();
	}

	void execute_algorithme(const Context &/*contexte*/, double /*temps*/)
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

void enregistre_operateurs_physiques(UsineOperateur *usine)
{
	const auto categorie = "Physique";

	usine->enregistre_type(
				NOM_GRAVITE,
				cree_description<OperateurGravite>(
					NOM_GRAVITE,
					AIDE_GRAVITE,
					categorie));
}
