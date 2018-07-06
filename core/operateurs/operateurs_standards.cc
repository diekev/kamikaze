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
 * The Original Code is Copyright (C) 2016 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include "operateurs_standards.h"

#include <kamikaze/bruit.h>
#include <kamikaze/context.h>
#include <kamikaze/mesh.h>
#include <kamikaze/primitive.h>
#include <kamikaze/prim_points.h>
#include <kamikaze/segmentprim.h>

#include <kamikaze/outils/géométrie.h>
#include <kamikaze/outils/interpolation.h>
#include <kamikaze/outils/mathématiques.h>
#include <kamikaze/outils/parallélisme.h>

#include <random>
#include <sstream>

/* ************************************************************************** */

static const char *NOM_SORTIE = "Sortie";
static const char *AIDE_SORTIE = "Créer un noeud de sortie.";

OperateurSortie::OperateurSortie(Noeud *noeud, const Context &contexte)
	: Operateur(noeud, contexte)
{
	entrees(1);
}

const char *OperateurSortie::nom_entree(size_t)
{
	return "Entrée";
}

const char *OperateurSortie::nom()
{
	return NOM_SORTIE;
}

void OperateurSortie::execute(const Context &contexte, double temps)
{
	m_collection->free_all();
	entree(0)->requiers_collection(m_collection, contexte, temps);
}

/* ************************************************************************** */

static const char *NOM_CREATION_BOITE = "Création boîte";
static const char *AIDE_CREATION_BOITE = "Créer une boîte.";

class OperateurCreationBoite final : public Operateur {
public:
	OperateurCreationBoite(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		sorties(1);

		ajoute_propriete("taille", danjo::TypePropriete::VECTEUR, glm::vec3(1.0));
		ajoute_propriete("centre", danjo::TypePropriete::VECTEUR, glm::vec3(0.0));
		ajoute_propriete("échelle", danjo::TypePropriete::DECIMAL, 1.0f);
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_creation_boite.danjo";
	}

	const char *nom_sortie(size_t /*index*/)
	{
		return "Sortie";
	}

	const char *nom() override
	{
		return NOM_CREATION_BOITE;
	}

	void execute(const Context &/*contexte*/, double /*temps*/)
	{
		m_collection->free_all();

		auto prim = m_collection->build("Mesh");
		auto mesh = static_cast<Mesh *>(prim);

		PointList *points = mesh->points();
		points->reserve(8);

		const auto dimension = evalue_vecteur("taille");
		const auto center = evalue_vecteur("centre");
		const auto uniform_scale = evalue_decimal("échelle");

		/* todo: expose this to the UI */
		const auto &x_div = 2;
		const auto &y_div = 2;
		const auto &z_div = 2;

		auto vec = glm::vec3{ 0.0f, 0.0f, 0.0f };

		const auto size = dimension * uniform_scale;

		const auto &start_x = -(size.x / 2.0f) + center.x;
		const auto &start_y = -(size.y / 2.0f) + center.y;
		const auto &start_z = -(size.z / 2.0f) + center.z;

		const auto &x_increment = size.x / (x_div - 1);
		const auto &y_increment = size.y / (y_div - 1);
		const auto &z_increment = size.z / (z_div - 1);

		for (auto x = 0; x < x_div; ++x) {
			vec[0] = start_x + x * x_increment;

			for (auto y = 0; y < y_div; ++y) {
				vec[1] = start_y + y * y_increment;

				for (auto z = 0; z < z_div; ++z) {
					vec[2] = start_z + z * z_increment;

					points->push_back(vec);
				}
			}
		}

		PolygonList *polys = mesh->polys();
		polys->resize(6);
		polys->push_back(glm::uvec4(1, 3, 2, 0));
		polys->push_back(glm::uvec4(3, 7, 6, 2));
		polys->push_back(glm::uvec4(7, 5, 4, 6));
		polys->push_back(glm::uvec4(5, 1, 0, 4));
		polys->push_back(glm::uvec4(0, 2, 6, 4));
		polys->push_back(glm::uvec4(5, 7, 3, 1));

		mesh->tagUpdate();
	}
};

/* ************************************************************************** */

static const char *NOM_TRANSFORMATION = "Transformation";
static const char *AIDE_TRANSFORMATION = "Transformer les matrices des primitives d'entrées.";

class OperateurTransformation : public Operateur {
public:
	OperateurTransformation(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		ajoute_propriete("ordre_transformation", danjo::TypePropriete::ENUM, std::string("pre"));
		ajoute_propriete("ordre_rotation", danjo::TypePropriete::ENUM, std::string("xyz"));
		ajoute_propriete("translation", danjo::TypePropriete::VECTEUR, glm::vec3(0.0));
		ajoute_propriete("rotation", danjo::TypePropriete::VECTEUR, glm::vec3(0.0));
		ajoute_propriete("taille", danjo::TypePropriete::VECTEUR, glm::vec3(1.0));
		ajoute_propriete("pivot", danjo::TypePropriete::VECTEUR, glm::vec3(0.0));
		ajoute_propriete("échelle", danjo::TypePropriete::DECIMAL, 1.0f);
		ajoute_propriete("inverse", danjo::TypePropriete::BOOL, false);
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_transformation.danjo";
	}

	const char *nom_entree(size_t /*index*/)
	{
		return "Entrée";
	}

	const char *nom_sortie(size_t /*index*/)
	{
		return "Sortie";
	}

	const char *nom() override
	{
		return NOM_TRANSFORMATION;
	}

	void execute(const Context &contexte, double temps)
	{
		entree(0)->requiers_collection(m_collection, contexte, temps);

		const auto translate = evalue_vecteur("translation");
		const auto rotate = evalue_vecteur("rotation");
		const auto scale = evalue_vecteur("taille");
		const auto pivot = evalue_vecteur("pivot");
		const auto uniform_scale = evalue_decimal("échelle");
		const auto transform_type = evalue_liste("ordre_transformation");
		const auto rot_order = evalue_liste("ordre_rotation");
		auto index_ordre = -1;

		if (rot_order == "xyz") {
			index_ordre = 0;
		}
		else if (rot_order == "xzy") {
			index_ordre = 1;
		}
		else if (rot_order == "yxz") {
			index_ordre = 2;
		}
		else if (rot_order == "yzx") {
			index_ordre = 3;
		}
		else if (rot_order == "zxy") {
			index_ordre = 4;
		}
		else if (rot_order == "zyx") {
			index_ordre = 5;
		}

		/* determine the rotatation order */
		int rot_ord[6][3] = {
			{ 0, 1, 2 }, // X Y Z
			{ 0, 2, 1 }, // X Z Y
			{ 1, 0, 2 }, // Y X Z
			{ 1, 2, 0 }, // Y Z X
			{ 2, 0, 1 }, // Z X Y
			{ 2, 1, 0 }, // Z Y X
		};

		glm::vec3 axis[3] = {
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
		};

		const auto X = rot_ord[index_ordre][0];
		const auto Y = rot_ord[index_ordre][1];
		const auto Z = rot_ord[index_ordre][2];

		for (auto &prim : primitive_iterator(this->m_collection)) {
			auto matrix = glm::mat4(1.0f);

			if (transform_type == "pre") {
				matrix = pre_translate(matrix, pivot);
				matrix = pre_rotate(matrix, glm::radians(rotate[X]), axis[X]);
				matrix = pre_rotate(matrix, glm::radians(rotate[Y]), axis[Y]);
				matrix = pre_rotate(matrix, glm::radians(rotate[Z]), axis[Z]);
				matrix = pre_scale(matrix, scale * uniform_scale);
				matrix = pre_translate(matrix, -pivot);
				matrix = pre_translate(matrix, translate);
				matrix = matrix * prim->matrix();
			}
			else {
				matrix = post_translate(matrix, pivot);
				matrix = post_rotate(matrix, glm::radians(rotate[X]), axis[X]);
				matrix = post_rotate(matrix, glm::radians(rotate[Y]), axis[Y]);
				matrix = post_rotate(matrix, glm::radians(rotate[Z]), axis[Z]);
				matrix = post_scale(matrix, scale * uniform_scale);
				matrix = post_translate(matrix, -pivot);
				matrix = post_translate(matrix, translate);
				matrix = prim->matrix() * matrix;
			}

			prim->matrix(matrix);
		}
	}
};

/* ************************************************************************** */

static const char *NOM_CREATION_TORUS = "Création torus";
static const char *AIDE_CREATION_TORUS = "Créer un torus.";

class OperateurCreationTorus : public Operateur {
public:
	OperateurCreationTorus(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		sorties(1);

		ajoute_propriete("centre", danjo::TypePropriete::VECTEUR, glm::vec3(0.0));
		ajoute_propriete("rayon_majeur", danjo::TypePropriete::DECIMAL, 1.0f);
		ajoute_propriete("rayon_mineur", danjo::TypePropriete::DECIMAL, 0.25f);
		ajoute_propriete("segments_majeurs", danjo::TypePropriete::ENTIER, 48);
		ajoute_propriete("segments_mineurs", danjo::TypePropriete::ENTIER, 24);
		ajoute_propriete("échelle", danjo::TypePropriete::DECIMAL, 1.0f);
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_creation_torus.danjo";
	}

	const char *nom_sortie(size_t /*index*/)
	{
		return "Sortie";
	}

	const char *nom() override
	{
		return NOM_CREATION_TORUS;
	}

	void execute(const Context &/*contexte*/, double /*temps*/)
	{
		m_collection->free_all();

		auto prim = m_collection->build("Mesh");
		auto mesh = static_cast<Mesh *>(prim);

		const auto center = evalue_vecteur("centre");
		const auto uniform_scale = evalue_decimal("échelle");

		const auto major_radius = evalue_decimal("rayon_majeur") * uniform_scale;
		const auto minor_radius = evalue_decimal("rayon_mineur") * uniform_scale;
		const auto major_segment = evalue_entier("segments_majeurs");
		const auto minor_segment = evalue_entier("segments_mineurs");

		PointList *points = mesh->points();
		PolygonList *polys = mesh->polys();

		constexpr auto tau = static_cast<float>(M_PI) * 2.0f;

		const auto vertical_angle_stride = tau / static_cast<float>(major_segment);
		const auto horizontal_angle_stride = tau / static_cast<float>(minor_segment);

		int f1 = 0, f2, f3, f4;
		const auto tot_verts = major_segment * minor_segment;

		points->reserve(tot_verts);

		for (int i = 0; i < major_segment; ++i) {
			auto theta = vertical_angle_stride * i;

			for (int j = 0; j < minor_segment; ++j) {
				auto phi = horizontal_angle_stride * j;

				auto x = glm::cos(theta) * (major_radius + minor_radius * glm::cos(phi));
				auto y = minor_radius * glm::sin(phi);
				auto z = glm::sin(theta) * (major_radius + minor_radius * glm::cos(phi));

				points->push_back(glm::vec3(x, y, z) + center);

				if (j + 1 == minor_segment) {
					f2 = i * minor_segment;
					f3 = f1 + minor_segment;
					f4 = f2 + minor_segment;
				}
				else {
					f2 = f1 + 1;
					f3 = f1 + minor_segment;
					f4 = f3 + 1;
				}

				if (f2 >= tot_verts) {
					f2 -= tot_verts;
				}
				if (f3 >= tot_verts) {
					f3 -= tot_verts;
				}
				if (f4 >= tot_verts) {
					f4 -= tot_verts;
				}

				if (f2 > 0) {
					polys->push_back(glm::uvec4(f1, f3, f4, f2));
				}
				else {
					polys->push_back(glm::uvec4(f2, f1, f3, f4));
				}

				++f1;
			}
		}

		mesh->tagUpdate();
	}
};

/* ************************************************************************** */

static const char *NOM_CREATION_GRILLE = "Création grille";
static const char *AIDE_CREATION_GRILLE = "Créer une grille.";

class OperateurCreationGrille : public Operateur {
public:
	OperateurCreationGrille(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		sorties(1);

		ajoute_propriete("centre", danjo::TypePropriete::VECTEUR, glm::vec3(0.0));
		ajoute_propriete("taille", danjo::TypePropriete::VECTEUR, glm::vec3(1.0));
		ajoute_propriete("lignes", danjo::TypePropriete::ENTIER, 2);
		ajoute_propriete("colonnes", danjo::TypePropriete::ENTIER, 2);
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_creation_grille.danjo";
	}

	const char *nom_sortie(size_t /*index*/)
	{
		return "Sortie";
	}

	const char *nom() override
	{
		return NOM_CREATION_GRILLE;
	}

	void execute(const Context &/*contexte*/, double /*temps*/)
	{
		m_collection->free_all();

		auto prim = m_collection->build("Mesh");
		auto mesh = static_cast<Mesh *>(prim);

		const auto size = evalue_vecteur("taille");
		const auto center = evalue_vecteur("centre");

		const auto rows = evalue_entier("lignes");
		const auto columns = evalue_entier("colonnes");

		const auto totpoints = rows * columns;

		auto points = mesh->points();
		points->reserve(totpoints);

		auto vec = glm::vec3{ 0.0f, center.y, 0.0f };

		const auto &x_increment = size.x / (rows - 1);
		const auto &y_increment = size.y / (columns - 1);
		const auto &start_x = -(size.x / 2.0f) + center.x;
		const auto &start_y = -(size.y / 2.0f) + center.z;

		for (auto y = 0; y < columns; ++y) {
			vec[2] = start_y + y * y_increment;

			for (auto x = 0; x < rows; ++x) {
				vec[0] = start_x + x * x_increment;

				points->push_back(vec);
			}
		}

		PolygonList *polys = mesh->polys();

		auto quad = glm::uvec4{ 0, 0, 0, 0 };

		/* make a copy for the lambda */
		const auto xtot = rows;

		auto index = [&xtot](const int x, const int y)
		{
			return x + y * xtot;
		};

		for (auto y = 1; y < columns; ++y) {
			for (auto x = 1; x < rows; ++x) {
				quad[0] = index(x - 1, y - 1);
				quad[1] = index(x,     y - 1);
				quad[2] = index(x,     y    );
				quad[3] = index(x - 1, y    );

				polys->push_back(quad);
			}
		}

		mesh->tagUpdate();
	}
};

/* ************************************************************************** */

static const char *NOM_CREATION_CERCLE = "Création cercle";
static const char *AIDE_CREATION_CERCLE = "Créer un cercle.";

class OperateurCreationCercle : public Operateur {
public:
	OperateurCreationCercle(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		sorties(1);

		ajoute_propriete("vertices", danjo::TypePropriete::ENTIER, 32);
		ajoute_propriete("rayon", danjo::TypePropriete::DECIMAL, 1.0f);
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_creation_cercle.danjo";
	}

	const char *nom_sortie(size_t /*index*/) override
	{
		return "Sortie";
	}

	const char *nom() override
	{
		return NOM_CREATION_CERCLE;
	}

	void execute(const Context &contexte, double /*temps*/)
	{
		if (m_collection == nullptr) {
			m_collection = new PrimitiveCollection(contexte.primitive_factory);
		}

		auto prim = m_collection->build("Mesh");
		auto mesh = static_cast<Mesh *>(prim);

		const auto segs = evalue_entier("vertices");
		const auto dia = evalue_decimal("rayon");

		const auto phid = 2.0f * static_cast<float>(M_PI) / segs;
		auto phi = 0.0f;

		PointList *points = mesh->points();
		points->reserve(segs + 1);

		glm::vec3 vec(0.0f, 0.0f, 0.0f);

		points->push_back(vec);

		for (int a = 0; a < segs; ++a, phi += phid) {
			/* Going this way ends up with normal(s) upward */
			vec[0] = -dia * std::sin(phi);
			vec[2] = dia * std::cos(phi);

			points->push_back(vec);
		}

		PolygonList *polys = mesh->polys();

		auto index = points->size() - 1;
		glm::uvec4 poly(0, 0, 0, INVALID_INDEX);

		for (auto i = 1ul; i < points->size(); ++i) {
			poly[1] = index;
			poly[2] = i;

			polys->push_back(poly);

			index = i;
		}

		mesh->tagUpdate();
	}
};

/* ************************************************************************** */

static void create_cylinder(PointList *points, PolygonList *polys, int segs, float dia1, float dia2, float depth)
{
	const auto phid = 2.0f * static_cast<float>(M_PI) / segs;
	auto phi = 0.0f;

	points->reserve((dia2 != 0.0f) ? segs * 2 + 2 : segs + 2);

	glm::vec3 vec(0.0f, 0.0f, 0.0f);

	const auto cent1 = 0;
	vec[1] = -depth;
	points->push_back(vec);

	const auto cent2 = 1;
	vec[1] = depth;
	points->push_back(vec);

	auto firstv1 = 0;
	auto firstv2 = 0;
	auto lastv1 = 0;
	auto lastv2 = 0;
	auto v1 = 0;
	auto v2 = 0;

	for (int a = 0; a < segs; ++a, phi += phid) {
		/* Going this way ends up with normal(s) upward */
		vec[0] = -dia1 * std::sin(phi);
		vec[1] = -depth;
		vec[2] = dia1 * std::cos(phi);

		v1 = points->size();
		points->push_back(vec);

		vec[0] = -dia2 * std::sin(phi);
		vec[1] = depth;
		vec[2] = dia2 * std::cos(phi);

		v2 = points->size();
		points->push_back(vec);

		if (a > 0) {
			/* Poly for the bottom cap. */
			polys->push_back(glm::uvec4{ cent1, lastv1, v1, INVALID_INDEX });

			/* Poly for the top cap. */
			polys->push_back(glm::uvec4{ cent2, v2, lastv2, INVALID_INDEX });

			/* Poly for the side. */
			polys->push_back(glm::uvec4{ lastv1, lastv2, v2, v1 });
		}
		else {
			firstv1 = v1;
			firstv2 = v2;
		}

		lastv1 = v1;
		lastv2 = v2;
	}

	/* Poly for the bottom cap. */
	polys->push_back(glm::uvec4{ cent1, v1, firstv1, INVALID_INDEX });

	/* Poly for the top cap. */
	polys->push_back(glm::uvec4{ cent2, firstv2, v2, INVALID_INDEX });

	/* Poly for the side. */
	polys->push_back(glm::uvec4{ v1, v2, firstv2, firstv1 });
}

static const char *NOM_CREATION_TUBE = "Création tube";
static const char *AIDE_CREATION_TUBE = "Créer un tube.";

class OperateurCreationTube : public Operateur {
public:
	OperateurCreationTube(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		sorties(1);

		ajoute_propriete("vertices", danjo::TypePropriete::ENTIER, 32);
		ajoute_propriete("rayon", danjo::TypePropriete::DECIMAL, 1.0f);
		ajoute_propriete("profondeur", danjo::TypePropriete::DECIMAL, 1.0f);
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_creation_tube.danjo";
	}

	const char *nom_sortie(size_t /*index*/) override
	{
		return "Sortie";
	}

	const char *nom() override
	{
		return NOM_CREATION_TUBE;
	}

	void execute(const Context &/*contexte*/, double /*temps*/)
	{
		m_collection->free_all();

		auto prim = m_collection->build("Mesh");
		auto mesh = static_cast<Mesh *>(prim);

		const auto segs = evalue_entier("vertices");
		const auto dia = evalue_decimal("rayon");
		const auto depth = evalue_decimal("profondeur");

		create_cylinder(mesh->points(), mesh->polys(), segs, dia, dia, depth);

		mesh->tagUpdate();
	}
};

/* ************************************************************************** */

static const char *NOM_CREATION_CONE = "Création cone";
static const char *AIDE_CREATION_CONE = "Créer un cone.";

class OperateurCreationCone : public Operateur {
public:
	OperateurCreationCone(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		sorties(1);

		ajoute_propriete("vertices", danjo::TypePropriete::ENTIER, 32);
		ajoute_propriete("rayon_mineur", danjo::TypePropriete::DECIMAL, 0.0f);
		ajoute_propriete("rayon_majeur", danjo::TypePropriete::DECIMAL, 1.0f);
		ajoute_propriete("profondeur", danjo::TypePropriete::DECIMAL, 1.0f);
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_creation_cone.danjo";
	}

	const char *nom_sortie(size_t /*index*/) override
	{
		return "Sortie";
	}

	const char *nom() override
	{
		return NOM_CREATION_CONE;
	}

	void execute(const Context &/*contexte*/, double /*temps*/)
	{
		m_collection->free_all();

		auto prim = m_collection->build("Mesh");
		auto mesh = static_cast<Mesh *>(prim);

		const auto segs = evalue_entier("vertices");
		const auto dia1 = evalue_decimal("rayon_mineur");
		const auto dia2 = evalue_decimal("rayon_majeur");
		const auto depth = evalue_decimal("profondeur");

		create_cylinder(mesh->points(), mesh->polys(), segs, dia1, dia2, depth);

		mesh->tagUpdate();
	}
};

/* ************************************************************************** */

static const float icovert[12][3] = {
	{0.0f, 0.0f, -200.0f},
	{144.72f, -105.144f, -89.443f},
	{-55.277f, -170.128, -89.443f},
	{-178.885f, 0.0f, -89.443f},
	{-55.277f, 170.128f, -89.443f},
	{144.72f, 105.144f, -89.443f},
	{55.277f, -170.128f, 89.443f},
	{-144.72f, -105.144f, 89.443f},
	{-144.72f, 105.144f, 89.443f},
	{55.277f, 170.128f, 89.443f},
	{178.885f, 0.0f, 89.443f},
	{0.0f, 0.0f, 200.0f}
};

static const short icoface[20][3] = {
	{0, 1, 2},
	{1, 0, 5},
	{0, 2, 3},
	{0, 3, 4},
	{0, 4, 5},
	{1, 5, 10},
	{2, 1, 6},
	{3, 2, 7},
	{4, 3, 8},
	{5, 4, 9},
	{1, 10, 6},
	{2, 6, 7},
	{3, 7, 8},
	{4, 8, 9},
	{5, 9, 10},
	{6, 10, 11},
	{7, 6, 11},
	{8, 7, 11},
	{9, 8, 11},
	{10, 9, 11}
};

static const char *NOM_CREATION_ICOSPHERE = "Création icosphère";
static const char *AIDE_CREATION_ICOSPHERE = "Crées une icosphère.";

class OperateurCreationIcoSphere : public Operateur {
public:
	OperateurCreationIcoSphere(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		sorties(1);

		ajoute_propriete("rayon", danjo::TypePropriete::DECIMAL, 1.0f);
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_creation_icosphere.danjo";
	}

	const char *nom() override
	{
		return NOM_CREATION_ICOSPHERE;
	}

	const char *nom_sortie(size_t /*index*/) override
	{
		return "Sortie";
	}

	void execute(const Context &contexte, double /*temps*/)
	{
		m_collection->free_all();

		auto prim = m_collection->build("Mesh");
		auto mesh = static_cast<Mesh *>(prim);

		const auto dia = evalue_decimal("rayon");
		const auto dia_div = dia / 200.0f;

		PointList *points = mesh->points();
		points->reserve(12);

		glm::vec3 vec(0.0f, 0.0f, 0.0f);

		for (int a = 0; a < 12; a++) {
			vec[0] = dia_div * icovert[a][0];
			vec[1] = dia_div * icovert[a][2];
			vec[2] = dia_div * icovert[a][1];

			points->push_back(vec);
		}

		PolygonList *polys = mesh->polys();
		polys->reserve(20);

		glm::uvec4 poly(0, 0, 0, INVALID_INDEX);

		for (auto i = 0; i < 20; ++i) {
			poly[0] = icoface[i][0];
			poly[1] = icoface[i][1];
			poly[2] = icoface[i][2];

			polys->push_back(poly);
		}

		mesh->tagUpdate();
	}
};

/* ************************************************************************** */

static const char *NOM_NORMAL = "Normal";
static const char *AIDE_NORMAL = "Éditer les normales.";

class OperateurNormal : public Operateur {
public:
	OperateurNormal(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		ajoute_propriete("inverse", danjo::TypePropriete::BOOL, false);
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_normal.danjo";
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
		return NOM_NORMAL;
	}

	void execute(const Context &contexte, double temps) override
	{
		entree(0)->requiers_collection(m_collection, contexte, temps);

		const auto flip = evalue_bool("inverse");

		for (auto &prim : primitive_iterator(this->m_collection, Mesh::id)) {
			auto mesh = static_cast<Mesh *>(prim);
			auto normals = mesh->attribute("normal", ATTR_TYPE_VEC3);
			auto points = mesh->points();

			normals->resize(points->size());

			auto polys = mesh->polys();

			for (size_t i = 0, ie = points->size(); i < ie ; ++i) {
				normals->vec3(i, glm::vec3(0.0f));
			}

			calcule_normales(*points, *polys, *normals, flip);
		}
	}
};

/* ************************************************************************** */

static const char *NOM_BRUIT = "Bruit";
static const char *AIDE_BRUIT = "Ajouter du bruit.";

class OperateurBruit : public Operateur {
	BruitPerlin3D m_bruit_perlin;
	BruitFlux3D m_bruit_flux;

public:
	OperateurBruit(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		ajoute_propriete("bruit", danjo::TypePropriete::ENUM, std::string("simplex"));
		ajoute_propriete("direction", danjo::TypePropriete::ENUM, std::string("x"));
		ajoute_propriete("taille", danjo::TypePropriete::DECIMAL, 1.0f);
		ajoute_propriete("octaves", danjo::TypePropriete::ENTIER, 1.0f);
		ajoute_propriete("frequence", danjo::TypePropriete::DECIMAL, 1.0f);
		ajoute_propriete("amplitude", danjo::TypePropriete::DECIMAL, 1.0f);
		ajoute_propriete("persistence", danjo::TypePropriete::DECIMAL, 1.0f);
		ajoute_propriete("lacunarité", danjo::TypePropriete::DECIMAL, 1.0f);
		ajoute_propriete("temps", danjo::TypePropriete::DECIMAL, 1.0f);
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_bruit.danjo";
	}

	bool ajourne_proprietes() override
	{
		const auto bruit = evalue_liste("bruit");

		rend_propriete_visible("temps", bruit == "flux");

		return true;
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
		return NOM_BRUIT;
	}

	void execute(const Context &contexte, double temps) override
	{
		entree(0)->requiers_collection(m_collection, contexte, temps);

		const auto taille = evalue_decimal("taille");
		const auto taille_inverse = (taille > 0.0f) ? 1.0f / taille : 0.0f;
		const auto octaves = evalue_entier("octaves");
		const auto lacunarity = evalue_decimal("lacunarité");
		const auto persistence = evalue_decimal("persistence");
		const auto ofrequency = evalue_decimal("frequence");
		const auto oamplitude = evalue_decimal("amplitude");
		const auto direction = evalue_liste("direction");
		const auto bruit = evalue_liste("bruit");

		if (bruit == "flux") {
			const auto temps_bruit = evalue_decimal("temps");
			m_bruit_flux.change_temps(temps_bruit);
		}

		for (auto prim : primitive_iterator(this->m_collection)) {
			PointList *points;

			Attribute *normales = nullptr;

			if (prim->typeID() == Mesh::id) {
				auto mesh = static_cast<Mesh *>(prim);
				points = mesh->points();
				normales = mesh->attribute("normal", ATTR_TYPE_VEC3);

				if (direction == "normal" && (normales == nullptr || normales->size() == 0)) {
					this->ajoute_avertissement("Absence de normales pour calculer le bruit !");
					continue;
				}
			}
			else if (prim->typeID() == PrimPoints::id) {
				auto prim_points = static_cast<PrimPoints *>(prim);
				points = prim_points->points();

				if (direction == "normal") {
					this->ajoute_avertissement("On ne peut calculer le bruit suivant la normale sur un nuage de points !");
					continue;
				}
			}
			else {
				continue;
			}

			for (size_t i = 0, e = points->size(); i < e; ++i) {
				auto &point = (*points)[i];
				const auto x = point.x * taille_inverse;
				const auto y = point.y * taille_inverse;
				const auto z = point.z * taille_inverse;
				auto valeur = 0.0f;

				auto frequency = ofrequency;
				auto amplitude = oamplitude;

				for (size_t j = 0; j < octaves; ++j) {
					if (bruit == "simplex") {
						valeur += (amplitude * bruit_simplex_3d(x * frequency, y * frequency, z * frequency));
					}
					else if (bruit == "perlin") {
						valeur += (amplitude * m_bruit_perlin(x * frequency, y * frequency, z * frequency));
					}
					else {
						valeur += (amplitude * m_bruit_flux(x * frequency, y * frequency, z * frequency));
					}

					frequency *= lacunarity;
					amplitude *= persistence;
				}

				if (direction == "x") {
					point.x += valeur;
				}
				else if (direction == "y") {
					point.y += valeur;
				}
				else if (direction == "z") {
					point.z += valeur;
				}
				else if (direction == "toutes") {
					point.x += valeur;
					point.y += valeur;
					point.z += valeur;
				}
				else if (direction == "normal") {
					const auto normale = normales->vec3(i);
					point.x += valeur * normale.x;
					point.y += valeur * normale.y;
					point.z += valeur * normale.z;
				}
			}
		}
	}
};

/* ************************************************************************** */

static const char *NOM_COULEUR = "Couleur";
static const char *AIDE_COULEUR = "Ajouter de la couleur.";

class OperateurCouleur : public Operateur {
public:
	OperateurCouleur(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		ajoute_propriete("portée", danjo::TypePropriete::ENUM, std::string("vertices"));
		ajoute_propriete("méthode", danjo::TypePropriete::ENUM, std::string("unique"));
		ajoute_propriete("couleur", danjo::TypePropriete::COULEUR, glm::vec3(0.5, 0.5, 0.5));
		ajoute_propriete("graine", danjo::TypePropriete::ENTIER, 1);
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_couleur.danjo";
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
		return NOM_COULEUR;
	}

	bool ajourne_proprietes() override
	{
		auto method = evalue_liste("méthode");

		if (method == "unique") {
			rend_propriete_visible("couleur", true);
			rend_propriete_visible("graine", false);
		}
		else if (method == "aléatoire") {
			rend_propriete_visible("graine", true);
			rend_propriete_visible("couleur", false);
		}

		return true;
	}

	void execute(const Context &contexte, double temps) override
	{
		entree(0)->requiers_collection(m_collection, contexte, temps);

		const auto &methode = evalue_liste("méthode");
		const auto &portee = evalue_liste("portée");
		const auto &graine = evalue_entier("graine");

		std::mt19937 rng(19937 + graine);
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		for (auto prim : primitive_iterator(this->m_collection)) {
			Attribute *colors;

			if (prim->typeID() == Mesh::id) {
				auto mesh = static_cast<Mesh *>(prim);
				colors = mesh->add_attribute("color", ATTR_TYPE_VEC4, mesh->points()->size());
			}
			else if (prim->typeID() == PrimPoints::id) {
				auto prim_points = static_cast<PrimPoints *>(prim);
				colors = prim_points->add_attribute("color", ATTR_TYPE_VEC4, prim_points->points()->size());
			}
			else {
				continue;
			}

			if (methode == "unique") {
				const auto &color = evalue_couleur("color");

				for (size_t i = 0, e = colors->size(); i < e; ++i) {
					colors->vec4(i, color);
				}
			}
			else if (methode == "aléatoire") {
				if (portee == "vertices") {
					for (size_t i = 0, e = colors->size(); i < e; ++i) {
						colors->vec4(i, glm::vec4{dist(rng), dist(rng), dist(rng), 1.0f});
					}
				}
				else if (portee == "primitive") {
					const auto &color = glm::vec4{dist(rng), dist(rng), dist(rng), 1.0f};

					for (size_t i = 0, e = colors->size(); i < e; ++i) {
						colors->vec4(i, color);
					}
				}
			}
		}
	}
};

/* ************************************************************************** */

static const char *NOM_FUSION_COLLECTION = "Fusion collections";
static const char *AIDE_FUSION_COLLECTION = "Fusionner des collections.";

class OperateurFusionCollection : public Operateur {
public:
	OperateurFusionCollection(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(2);
		sorties(1);
	}

	const char *nom_entree(size_t index) override
	{
		if (index == 0) {
			return "Entrée 1";
		}

		return "Entrée 2";
	}

	const char *nom_sortie(size_t /*index*/) override
	{
		return "Sortie";
	}

	const char *nom() override
	{
		return NOM_FUSION_COLLECTION;
	}

	void execute(const Context &contexte, double temps) override
	{
		entree(0)->requiers_collection(m_collection, contexte, temps);

		auto collection2 = this->entree(1)->requiers_collection(nullptr, contexte, temps);

		if (collection2 == nullptr) {
			return;
		}

		m_collection->merge_collection(*collection2);
	}
};

/* ************************************************************************** */

static const char *NOM_CREATION_NUAGE_POINT = "Création nuage point";
static const char *AIDE_CREATION_NUAGE_POINT = "Création d'un nuage de point.";

class OperateurCreationNuagePoint : public Operateur {
public:
	OperateurCreationNuagePoint(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		sorties(1);

		ajoute_propriete("nombre_points", danjo::TypePropriete::ENTIER, 1000);
		ajoute_propriete("limite_min", danjo::TypePropriete::VECTEUR, glm::vec3(-1.0));
		ajoute_propriete("limite_max", danjo::TypePropriete::VECTEUR, glm::vec3(1.0));
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_creation_nuage_point.danjo";
	}

	const char *nom_sortie(size_t /*index*/) override
	{
		return "Sortie";
	}

	const char *nom() override
	{
		return NOM_CREATION_NUAGE_POINT;
	}

	void execute(const Context &contexte, double /*temps*/) override
	{
		m_collection->free_all();

		auto prim = m_collection->build("PrimPoints");
		auto points = static_cast<PrimPoints *>(prim);

		const auto &nombre_points = evalue_entier("nombre_points");

		auto point_list = points->points();
		point_list->resize(nombre_points);

		const auto &limite_min = evalue_vecteur("limite_min");
		const auto &limite_max = evalue_vecteur("limite_max");

		std::uniform_real_distribution<float> dist_x(limite_min[0], limite_max[0]);
		std::uniform_real_distribution<float> dist_y(limite_min[1], limite_max[1]);
		std::uniform_real_distribution<float> dist_z(limite_min[2], limite_max[2]);
		std::mt19937 rng_x(19937);
		std::mt19937 rng_y(19937 + 1);
		std::mt19937 rng_z(19937 + 2);

		for (size_t i = 0; i < nombre_points; ++i) {
			const auto &point = glm::vec3(dist_x(rng_x), dist_y(rng_y), dist_z(rng_z));
			(*point_list)[i] = point;
		}

		points->tagUpdate();
	}
};

/* ************************************************************************** */

static const char *NOM_CREATION_ATTRIBUT = "Création attribut";
static const char *AIDE_CREATION_ATTRIBUT = "Création d'un attribut.";

static AttributeType type_attribut_depuis_chaine(const std::string &chaine)
{
	if (chaine == "octet") {
		return ATTR_TYPE_BYTE;
	}
	else if (chaine == "entier") {
		return ATTR_TYPE_INT;
	}
	else if (chaine == "décimal") {
		return ATTR_TYPE_FLOAT;
	}
	else if (chaine == "chaine") {
		return ATTR_TYPE_STRING;
	}
	else if (chaine == "vecteur_2d") {
		return ATTR_TYPE_VEC2;
	}
	else if (chaine == "vecteur_3d") {
		return ATTR_TYPE_VEC3;
	}
	else if (chaine == "vecteur_4d") {
		return ATTR_TYPE_VEC4;
	}
	else if (chaine == "matrice_3x3") {
		return ATTR_TYPE_MAT3;
	}
	else if (chaine == "matrice_4x4") {
		return ATTR_TYPE_MAT4;
	}

	return ATTR_TYPE_INT;
}

class OperateurCreationAttribut : public Operateur {
public:
	OperateurCreationAttribut(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		ajoute_propriete("nom_attribut", danjo::TypePropriete::CHAINE_CARACTERE, std::string(""));
		ajoute_propriete("type_attribut", danjo::TypePropriete::ENUM, std::string("octet"));
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_suppression_attribut.danjo";
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
		return NOM_CREATION_ATTRIBUT;
	}

	void execute(const Context &contexte, double temps) override
	{
		entree(0)->requiers_collection(m_collection, contexte, temps);

		auto nom_attribut = evalue_chaine("nom_attribut");
		auto type_attribut = evalue_liste("type_attribut");
		auto type = type_attribut_depuis_chaine(type_attribut);

		for (Primitive *prim : primitive_iterator(m_collection)) {
			if (prim->has_attribute(nom_attribut, type)) {
				std::stringstream ss;
				ss << prim->name() << " already has an attribute named " << nom_attribut;

				this->ajoute_avertissement(ss.str());
				continue;
			}

			size_t attrib_size = 1;

			if (prim->typeID() == Mesh::id) {
				auto mesh = static_cast<Mesh *>(prim);

				if (type_attribut == "vecteur_3d") {
					attrib_size = mesh->points()->size();
				}
			}
			else if (prim->typeID() == PrimPoints::id) {
				auto point_cloud = static_cast<PrimPoints *>(prim);

				if (type_attribut == "vecteur_3d") {
					attrib_size = point_cloud->points()->size();
				}
			}

			prim->add_attribute(nom_attribut, type, attrib_size);
		}
	}
};

/* ************************************************************************** */

static const char *NOM_SUPPRESSION_ATTRIBUT = "Suppression attribut";
static const char *AIDE_SUPPRESSION_ATTRIBUT = "Suppression d'un attribut.";

class OperateurSuppressionAttribut : public Operateur {
public:
	OperateurSuppressionAttribut(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		ajoute_propriete("nom_attribut", danjo::TypePropriete::CHAINE_CARACTERE, std::string(""));
		ajoute_propriete("type_attribut", danjo::TypePropriete::ENUM, std::string("octet"));
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_suppression_attribut.danjo";
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
		return NOM_SUPPRESSION_ATTRIBUT;
	}

	void execute(const Context &contexte, double temps) override
	{
		entree(0)->requiers_collection(m_collection, contexte, temps);

		auto nom_attribut = evalue_chaine("nom_attribut");
		auto type_attribut = evalue_liste("type_attribut");
		auto type = type_attribut_depuis_chaine(type_attribut);

		for (Primitive *prim : primitive_iterator(m_collection)) {
			if (!prim->has_attribute(nom_attribut, type)) {
				continue;
			}

			prim->remove_attribute(nom_attribut, type);
		}
	}
};

/* ************************************************************************** */

static const char *NOM_RANDOMISATION_ATTRIBUT = "Randomisation attribut";
static const char *AIDE_RANDOMISATION_ATTRIBUT = "Randomisation d'un attribut.";

/* À FAIRE : distribution exponentielle, lognormal, cauchy, discrète */

class OperateurRandomisationAttribut : public Operateur {
public:
	OperateurRandomisationAttribut(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		ajoute_propriete("nom_attribut", danjo::TypePropriete::CHAINE_CARACTERE, std::string(""));
		ajoute_propriete("type_attribut", danjo::TypePropriete::ENUM, std::string("octet"));
		ajoute_propriete("distribution", danjo::TypePropriete::ENUM, std::string("constant"));
		ajoute_propriete("valeur", danjo::TypePropriete::DECIMAL, 1.0f);
		ajoute_propriete("valeur_min", danjo::TypePropriete::DECIMAL, 1.0f);
		ajoute_propriete("valeur_max", danjo::TypePropriete::DECIMAL, 1.0f);
		ajoute_propriete("moyenne", danjo::TypePropriete::DECIMAL, 1.0f);
		ajoute_propriete("écart_type", danjo::TypePropriete::DECIMAL, 1.0f);
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_randomise_attribut.danjo";
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
		return NOM_RANDOMISATION_ATTRIBUT;
	}

	bool ajourne_proprietes() override
	{
		const auto distribution = evalue_liste("distribution");

		rend_propriete_visible("valeur", distribution == "constante");
		rend_propriete_visible("min_value", distribution == "uniforme");
		rend_propriete_visible("max_value", distribution == "uniforme");
		rend_propriete_visible("mean", distribution == "gaussienne");
		rend_propriete_visible("stddev", distribution == "gaussienne");

		return true;
	}

	void execute(const Context &contexte, double temps) override
	{
		entree(0)->requiers_collection(m_collection, contexte, temps);

		auto name = evalue_chaine("nom_attribut");
		auto type_attribut = evalue_liste("type_attribut");
		auto distribution = evalue_liste("distribution");
		auto value = evalue_decimal("valeur");
		auto min_value = evalue_decimal("valeur_min");
		auto max_value = evalue_decimal("valeur_max");
		auto mean = evalue_decimal("moyenne");
		auto stddev = evalue_decimal("écart_type");
		auto type = type_attribut_depuis_chaine(type_attribut);

		std::mt19937 rng(19993754);

		if (type != ATTR_TYPE_VEC3) {
			std::stringstream ss;
			ss << "Only 3D Vector attributes are supported for now!";

			this->ajoute_avertissement(ss.str());
			return;
		}

		for (Primitive *prim : primitive_iterator(m_collection)) {
			auto attribute = prim->attribute(name, type);

			if (!attribute) {
				std::stringstream ss;
				ss << prim->name() << " does not have an attribute named \"" << name
				   << "\" of type " << static_cast<int>(type);

				this->ajoute_avertissement(ss.str());
				continue;
			}

			if (distribution == "constante") {
				for (size_t i = 0; i < attribute->size(); ++i) {
					attribute->vec3(i, glm::vec3{value, value, value});
				}
			}
			else if (distribution == "uniforme") {
				std::uniform_real_distribution<float> dist(min_value, max_value);

				for (size_t i = 0; i < attribute->size(); ++i) {
					attribute->vec3(i, glm::vec3{dist(rng), dist(rng), dist(rng)});
				}
			}
			else if (distribution == "gaussienne") {
				std::normal_distribution<float> dist(mean, stddev);

				for (size_t i = 0; i < attribute->size(); ++i) {
					attribute->vec3(i, glm::vec3{dist(rng), dist(rng), dist(rng)});
				}

			}
		}
	}
};

/* ************************************************************************** */

struct Triangle {
	glm::vec3 v0, v1, v2;
};

std::vector<Triangle> convertis_maillage_triangles(const Mesh *maillage_entree)
{
	std::vector<Triangle> triangles;
	const auto points = maillage_entree->points();
	const auto polygones = maillage_entree->polys();

	/* Convertis le maillage en triangles. */
	auto nombre_triangles = 0ul;

	for (auto i = 0ul; i < polygones->size(); ++i) {
		const auto polygone = (*polygones)[i];

		nombre_triangles += ((polygone[3] == INVALID_INDEX) ? 1 : 2);
	}

	triangles.reserve(nombre_triangles);

	for (auto i = 0ul; i < polygones->size(); ++i) {
		const auto polygone = (*polygones)[i];

		Triangle triangle;
		triangle.v0 = (*points)[polygone[0]];
		triangle.v1 = (*points)[polygone[1]];
		triangle.v2 = (*points)[polygone[2]];

		triangles.push_back(triangle);

		if (polygone[3] != INVALID_INDEX) {
			Triangle triangle2;
			triangle2.v0 = (*points)[polygone[0]];
			triangle2.v1 = (*points)[polygone[2]];
			triangle2.v2 = (*points)[polygone[3]];

			triangles.push_back(triangle2);
		}
	}

	return triangles;
}

static const char *NOM_CREATION_SEGMENTS = "Création courbes";
static const char *AIDE_CREATION_SEGMENTS = "Création de courbes.";

class OperateurCreationCourbes : public Operateur {
public:
	OperateurCreationCourbes(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		ajoute_propriete("méthode", danjo::TypePropriete::ENUM, std::string("vertices"));
		ajoute_propriete("graine", danjo::TypePropriete::ENTIER, 1);
		ajoute_propriete("nombre_courbes", danjo::TypePropriete::ENTIER, 100);
		ajoute_propriete("segments", danjo::TypePropriete::ENTIER, 1);
		ajoute_propriete("taille", danjo::TypePropriete::DECIMAL, 1.0f);
		ajoute_propriete("direction", danjo::TypePropriete::ENUM, std::string("normal"));
		ajoute_propriete("normal", danjo::TypePropriete::VECTEUR, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_creation_courbes.danjo";
	}

	bool ajourne_proprietes() override
	{
		auto methode = evalue_liste("méthode");
		rend_propriete_visible("graine", methode == "polygones");
		rend_propriete_visible("nombre_courbes", methode == "polygones");

		const auto direction = evalue_liste("direction");
		rend_propriete_visible("normale", direction == "personnalisée");

		return true;
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
		return NOM_CREATION_SEGMENTS;
	}

	void execute(const Context &contexte, double temps) override
	{
		entree(0)->requiers_collection(m_collection, contexte, temps);

		auto iter = primitive_iterator(m_collection, Mesh::id);

		if (iter.get() == nullptr) {
			this->ajoute_avertissement("No input mesh found!");
			return;
		}

		const auto input_mesh = static_cast<Mesh *>(iter.get());
		const auto input_points = input_mesh->points();

		const auto segment_number = evalue_entier("segments");
		const auto segment_normal = evalue_vecteur("normal");
		const auto segment_size = evalue_decimal("taille");
		const auto methode = evalue_liste("méthode");
		const auto direction = evalue_liste("direction");

		auto segment_prim = static_cast<SegmentPrim *>(m_collection->build("SegmentPrim"));
		auto output_edges = segment_prim->edges();
		auto output_points = segment_prim->points();

		auto num_points = 0ul;
		auto total_points = 0ul;

		const auto direction_normal = (direction == "normal");

		if (methode == "vertices") {
			Attribute *normales = nullptr;

			if (direction_normal) {
				normales = input_mesh->attribute("normal", ATTR_TYPE_VEC3);

				if (normales == nullptr || normales->size() == 0) {
					this->ajoute_avertissement("Il n'y a pas de données de normales sur les vertex d'entrées !");
					return;
				}
			}

			total_points = input_points->size() * (segment_number + 1);

			output_edges->reserve(input_points->size() * segment_number);
			output_points->reserve(total_points);
			auto head = 0;
			glm::vec3 normale;

			for (size_t i = 0; i < input_points->size(); ++i) {
				auto point = (*input_points)[i];
				normale = direction_normal ? normales->vec3(i) : segment_normal;

				output_points->push_back(point);
				++num_points;

				for (int j = 0; j < segment_number; ++j, ++num_points) {
					point += (segment_size * normale);
					output_points->push_back(point);

					output_edges->push_back(glm::uvec2{head, ++head});
				}

				++head;
			}
		}
		else if (methode == "polygones") {
			auto triangles = convertis_maillage_triangles(input_mesh);

			const auto nombre_courbes = evalue_entier("nombre_courbes");
			const auto nombre_polys = triangles.size();

			output_edges->reserve((nombre_courbes * segment_number) * nombre_polys);
			output_points->reserve(total_points);

			total_points = nombre_polys * nombre_courbes * (segment_number + 1);

			const auto graine = evalue_entier("graine");

			std::mt19937 rng(19937 + graine);
			std::uniform_real_distribution<float> dist(0.0f, 1.0f);

			auto head = 0;

			glm::vec3 normale;

			for (const Triangle &triangle : triangles) {
				const auto v0 = triangle.v0;
				const auto v1 = triangle.v1;
				const auto v2 = triangle.v2;

				const auto e0 = v1 - v0;
				const auto e1 = v2 - v0;

				normale = (direction_normal) ? glm::normalize(normale_triangle(v0, v1, v2)) : segment_normal;

				for (size_t j = 0; j < nombre_courbes; ++j) {
					/* Génère des coordonnées barycentriques aléatoires. */
					auto r = dist(rng);
					auto s = dist(rng);

					if (r + s >= 1.0f) {
						r = 1.0f - r;
						s = 1.0f - s;
					}

					auto pos = v0 + r * e0 + s * e1;

					output_points->push_back(pos);
					++num_points;

					for (int k = 0; k < segment_number; ++k, ++num_points) {
						pos += (segment_size * normale);
						output_points->push_back(pos);

						output_edges->push_back(glm::uvec2{head, ++head});
					}

					++head;
				}
			}
		}

		if (num_points != total_points) {
			std::stringstream ss;

			if (num_points < total_points) {
				ss << "Overallocation of points, allocated: " << total_points
				   << ", final total: " << num_points;
			}
			else if (num_points > total_points) {
				ss << "Underallocation of points, allocated: " << total_points
				   << ", final total: " << num_points;
			}

			this->ajoute_avertissement(ss.str());
		}
	}
};


/* ************************************************************************** */

static const char *NOM_TAMPON = "Tampon";
static const char *AIDE_TAMPON = "Met la collection d'entrée dans un tampon pour ne plus la recalculer.";

class OperateurTampon : public Operateur {
	PrimitiveCollection *m_collecion_tampon = nullptr;

public:
	OperateurTampon(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		m_collecion_tampon = new PrimitiveCollection(contexte.primitive_factory);
	}

	~OperateurTampon()
	{
		delete m_collecion_tampon;
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
		return NOM_TAMPON;
	}

	void execute(const Context &contexte, double temps) override
	{
		/* À FAIRE : généraliser à tous les noeuds + meilleur solution. */
		m_collection->free_all();

		if (this->besoin_execution()) {
			m_collecion_tampon->free_all();
			entree(0)->requiers_collection(m_collecion_tampon, contexte, temps);
		}

		auto collection_temporaire = m_collecion_tampon->copy();
		m_collection->merge_collection(*collection_temporaire);
		delete collection_temporaire;
	}
};

/* ************************************************************************** */

static const char *NOM_COMMUTATEUR = "Commutateur";
static const char *AIDE_COMMUTATEUR = "Change la direction de l'évaluation du"
									  "graphe en fonction de la prise choisie.";

class OperateurCommutateur : public Operateur {
public:
	OperateurCommutateur(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(2);
		sorties(1);

		ajoute_propriete("prise", danjo::TypePropriete::ENTIER, 0);
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_commutateur.danjo";
	}

	const char *nom_entree(size_t index) override
	{
		if (index == 0) {
			return "Entrée 0";
		}

		return "Entrée 1";
	}

	const char *nom_sortie(size_t /*index*/) override
	{
		return "Sortie";
	}

	const char *nom() override
	{
		return NOM_COMMUTATEUR;
	}

	void execute(const Context &contexte, double temps) override
	{
		const auto prise = evalue_entier("prise");
		entree(prise)->requiers_collection(m_collection, contexte, temps);
	}
};

/* ************************************************************************** */

static const char *NOM_DISPERSION_POINTS = "Dispersion Points";
static const char *AIDE_DISPERSION_POINTS = "Disperse des points sur une surface.";

class OperateurDispersionPoints : public Operateur {
public:
	OperateurDispersionPoints(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		ajoute_propriete("graine", danjo::TypePropriete::ENTIER, 1);
		ajoute_propriete("nombre_points_polys", danjo::TypePropriete::ENTIER, 100);
	}

	const char *chemin_interface() const override
	{
		return "interface/operateur_dispersion_points.danjo";
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
		return NOM_DISPERSION_POINTS;
	}

	void execute(const Context &contexte, double temps) override
	{
		entree(0)->requiers_collection(m_collection, contexte, temps);

		auto iter = primitive_iterator(m_collection, Mesh::id);

		if (iter.get() == nullptr) {
			this->ajoute_avertissement("Il n'y a pas de mesh dans la collecion d'entrée !");
			return;
		}

		const auto maillage_entree = static_cast<Mesh *>(iter.get());

		auto triangles = convertis_maillage_triangles(maillage_entree);

		auto nuage_points = static_cast<PrimPoints *>(m_collection->build("PrimPoints"));
		auto points_sorties = nuage_points->points();

		const auto nombre_points_polys = evalue_entier("nombre_points_polys");
		const auto nombre_points = triangles.size() * nombre_points_polys;

		points_sorties->reserve(nombre_points);

		const auto graine = evalue_entier("graine");

		std::mt19937 rng(19937 + graine);
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		for (const Triangle &triangle : triangles) {
			const auto v0 = triangle.v0;
			const auto v1 = triangle.v1;
			const auto v2 = triangle.v2;

			const auto e0 = v1 - v0;
			const auto e1 = v2 - v0;

			for (size_t j = 0; j < nombre_points_polys; ++j) {
				/* Génère des coordonnées barycentriques aléatoires. */
				auto r = dist(rng);
				auto s = dist(rng);

				if (r + s >= 1.0f) {
					r = 1.0f - r;
					s = 1.0f - s;
				}

				auto pos = v0 + r * e0 + s * e1;

				points_sorties->push_back(pos);
			}
		}
	}
};

/* ************************************************************************** */
#if 0
static const char *NOM_ = "";
static const char *AIDE_ = "";

class OperateurModele : public Operateur {
public:
	OperateurModele(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);
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
		retrun NOM_;
	}

	void execute(const Context &contexte, double temps) override
	{
		entree(0)->requiers_collection(m_collection, contexte, temps);
	}
};
#endif

/* ************************************************************************** */

void enregistre_operateurs_integres(UsineOperateur *usine)
{
	/* Opérateurs géométrie. */

	auto categorie = "Géométrie";

	usine->enregistre_type(NOM_CREATION_BOITE,
						   cree_description<OperateurCreationBoite>(NOM_CREATION_BOITE,
																	AIDE_CREATION_BOITE,
																	categorie));

	usine->enregistre_type(NOM_CREATION_TORUS,
						   cree_description<OperateurCreationTorus>(NOM_CREATION_TORUS,
																	AIDE_CREATION_TORUS,
																	categorie));

	usine->enregistre_type(NOM_CREATION_CERCLE,
						   cree_description<OperateurCreationCercle>(NOM_CREATION_CERCLE,
																	 AIDE_CREATION_CERCLE,
																	 categorie));

	usine->enregistre_type(NOM_CREATION_GRILLE,
						   cree_description<OperateurCreationGrille>(NOM_CREATION_GRILLE,
																	 AIDE_CREATION_GRILLE,
																	 categorie));

	usine->enregistre_type(NOM_CREATION_TUBE,
						   cree_description<OperateurCreationTube>(NOM_CREATION_TUBE,
																   AIDE_CREATION_TUBE,
																   categorie));

	usine->enregistre_type(NOM_CREATION_CONE,
						   cree_description<OperateurCreationCone>(NOM_CREATION_CONE,
																   AIDE_CREATION_CONE,
																   categorie));

	usine->enregistre_type(NOM_CREATION_ICOSPHERE,
						   cree_description<OperateurCreationIcoSphere>(NOM_CREATION_ICOSPHERE,
																		AIDE_CREATION_ICOSPHERE,
																		categorie));

	usine->enregistre_type(NOM_TRANSFORMATION,
						   cree_description<OperateurTransformation>(NOM_TRANSFORMATION,
																	 AIDE_TRANSFORMATION,
																	 categorie));

	usine->enregistre_type(NOM_NORMAL,
						   cree_description<OperateurNormal>(NOM_NORMAL,
															 AIDE_NORMAL,
															 categorie));

	usine->enregistre_type(NOM_BRUIT,
						   cree_description<OperateurBruit>(NOM_BRUIT,
															AIDE_BRUIT,
															categorie));

	usine->enregistre_type(NOM_COULEUR,
						   cree_description<OperateurCouleur>(NOM_COULEUR,
															  AIDE_COULEUR,
															  categorie));

	usine->enregistre_type(NOM_FUSION_COLLECTION,
						   cree_description<OperateurFusionCollection>(NOM_FUSION_COLLECTION,
																	   AIDE_FUSION_COLLECTION,
																	   categorie));

	usine->enregistre_type(NOM_CREATION_NUAGE_POINT,
						   cree_description<OperateurCreationNuagePoint>(NOM_CREATION_NUAGE_POINT,
																		 AIDE_CREATION_NUAGE_POINT,
																		 categorie));

	usine->enregistre_type(NOM_CREATION_SEGMENTS,
						   cree_description<OperateurCreationCourbes>(NOM_CREATION_SEGMENTS,
																	   AIDE_CREATION_SEGMENTS,
																	   categorie));

	usine->enregistre_type(NOM_DISPERSION_POINTS,
						   cree_description<OperateurDispersionPoints>(NOM_DISPERSION_POINTS,
																	   AIDE_DISPERSION_POINTS,
																	   categorie));

	/* Opérateurs attributs. */

	categorie = "Attributs";

	usine->enregistre_type(NOM_CREATION_ATTRIBUT,
						   cree_description<OperateurCreationAttribut>(NOM_CREATION_ATTRIBUT,
																	   AIDE_CREATION_ATTRIBUT,
																	   categorie));

	usine->enregistre_type(NOM_SUPPRESSION_ATTRIBUT,
						   cree_description<OperateurSuppressionAttribut>(NOM_SUPPRESSION_ATTRIBUT,
																		  AIDE_SUPPRESSION_ATTRIBUT,
																		  categorie));

	usine->enregistre_type(NOM_RANDOMISATION_ATTRIBUT,
						   cree_description<OperateurRandomisationAttribut>(NOM_RANDOMISATION_ATTRIBUT,
																			AIDE_RANDOMISATION_ATTRIBUT,
																			categorie));

	/* Opérateurs autres. */

	categorie = "Autre";

	usine->enregistre_type(NOM_SORTIE,
						   cree_description<OperateurSortie>(NOM_SORTIE,
															 AIDE_SORTIE,
															 categorie));

	usine->enregistre_type(NOM_TAMPON,
						   cree_description<OperateurTampon>(NOM_TAMPON,
															 AIDE_TAMPON,
															 categorie));

	usine->enregistre_type(NOM_COMMUTATEUR,
						   cree_description<OperateurCommutateur>(NOM_COMMUTATEUR,
																  AIDE_COMMUTATEUR,
																  categorie));
}
