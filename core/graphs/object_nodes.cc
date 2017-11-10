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

#include "object_nodes.h"

#include <kamikaze/bruit.h>
#include <kamikaze/context.h>
#include <kamikaze/mesh.h>
#include <kamikaze/primitive.h>
#include <kamikaze/prim_points.h>
#include <kamikaze/segmentprim.h>
#include <kamikaze/util_parallel.h>
#include <kamikaze/utils_glm.h>

#include <random>
#include <sstream>

#include "ui/paramfactory.h"

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

		add_prop("size", "Size", property_type::prop_vec3);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_vec3(glm::vec3{1.0f, 1.0f, 1.0f});

		add_prop("center", "Center", property_type::prop_vec3);
		set_prop_min_max(-10.0f, 10.0f);
		set_prop_default_value_vec3(glm::vec3{0.0f, 0.0f, 0.0f});

		add_prop("uniform_scale", "Uniform Scale", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(1.0f);
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

		const auto dimension = eval_vec3("size");
		const auto center = eval_vec3("center");
		const auto uniform_scale = eval_float("uniform_scale");

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

		EnumProperty xform_enum_prop;
		xform_enum_prop.insert("Pre Transform", 0);
		xform_enum_prop.insert("Post Transform", 1);

		add_prop("xform_order", "Transform Order", property_type::prop_enum);
		set_prop_enum_values(xform_enum_prop);

		EnumProperty rot_enum_prop;
		rot_enum_prop.insert("X Y Z", 0);
		rot_enum_prop.insert("X Z Y", 1);
		rot_enum_prop.insert("Y X Z", 2);
		rot_enum_prop.insert("Y Z X", 3);
		rot_enum_prop.insert("Z X Y", 4);
		rot_enum_prop.insert("Z Y X", 5);

		add_prop("rot_order", "Rotation Order", property_type::prop_enum);
		set_prop_enum_values(rot_enum_prop);

		add_prop("translate", "Translate", property_type::prop_vec3);
		set_prop_min_max(-10.0f, 10.0f);
		set_prop_default_value_vec3(glm::vec3{0.0f, 0.0f, 0.0f});

		add_prop("rotate", "Rotate", property_type::prop_vec3);
		set_prop_min_max(0.0f, 360.0f);
		set_prop_default_value_vec3(glm::vec3{0.0f, 0.0f, 0.0f});

		add_prop("scale", "Scale", property_type::prop_vec3);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_vec3(glm::vec3{1.0f, 1.0f, 1.0f});

		add_prop("pivot", "Pivot", property_type::prop_vec3);
		set_prop_min_max(-10.0f, 10.0f);
		set_prop_default_value_vec3(glm::vec3{0.0f, 0.0f, 0.0f});

		add_prop("uniform_scale", "Uniform Scale", property_type::prop_float);
		set_prop_min_max(0.0f, 1000.0f);
		set_prop_default_value_float(1.0f);

		add_prop("invert_xform", "Invert Transformation", property_type::prop_bool);
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

		const auto translate = eval_vec3("translate");
		const auto rotate = eval_vec3("rotate");
		const auto scale = eval_vec3("scale");
		const auto pivot = eval_vec3("pivot");
		const auto uniform_scale = eval_float("uniform_scale");
		const auto transform_type = eval_int("xform_order");
		const auto rot_order = eval_int("rot_order");

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

		const auto X = rot_ord[rot_order][0];
		const auto Y = rot_ord[rot_order][1];
		const auto Z = rot_ord[rot_order][2];

		for (auto &prim : primitive_iterator(this->m_collection)) {
			auto matrix = glm::mat4(1.0f);

			switch (transform_type) {
				case 0: /* Pre Transform */
					matrix = pre_translate(matrix, pivot);
					matrix = pre_rotate(matrix, glm::radians(rotate[X]), axis[X]);
					matrix = pre_rotate(matrix, glm::radians(rotate[Y]), axis[Y]);
					matrix = pre_rotate(matrix, glm::radians(rotate[Z]), axis[Z]);
					matrix = pre_scale(matrix, scale * uniform_scale);
					matrix = pre_translate(matrix, -pivot);
					matrix = pre_translate(matrix, translate);
					matrix = matrix * prim->matrix();
					break;
				case 1: /* Post Transform */
					matrix = post_translate(matrix, pivot);
					matrix = post_rotate(matrix, glm::radians(rotate[X]), axis[X]);
					matrix = post_rotate(matrix, glm::radians(rotate[Y]), axis[Y]);
					matrix = post_rotate(matrix, glm::radians(rotate[Z]), axis[Z]);
					matrix = post_scale(matrix, scale * uniform_scale);
					matrix = post_translate(matrix, -pivot);
					matrix = post_translate(matrix, translate);
					matrix = prim->matrix() * matrix;
					break;
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

		add_prop("center", "Center", property_type::prop_vec3);
		set_prop_min_max(-10.0f, 10.0f);
		set_prop_default_value_vec3(glm::vec3{0.0f, 0.0f, 0.0f});

		add_prop("major_radius", "Major Radius", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(1.0f);

		add_prop("minor_radius", "Minor Radius", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(0.25f);

		add_prop("major_segment", "Major Segment", property_type::prop_int);
		set_prop_min_max(4, 100);
		set_prop_default_value_int(48);

		add_prop("minor_segment", "Minor Segment", property_type::prop_int);
		set_prop_min_max(4, 100);
		set_prop_default_value_int(24);

		add_prop("uniform_scale", "Uniform Scale", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(1.0f);
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

		const auto center = eval_vec3("center");
		const auto uniform_scale = eval_float("uniform_scale");

		const auto major_radius = eval_float("major_radius") * uniform_scale;
		const auto minor_radius = eval_float("minor_radius") * uniform_scale;
		const auto major_segment = eval_int("major_segment");
		const auto minor_segment = eval_int("minor_segment");

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

		add_prop("center", "Center", property_type::prop_vec3);
		set_prop_min_max(-10.0f, 10.0f);
		set_prop_default_value_vec3(glm::vec3{0.0f, 0.0f, 0.0f});

		add_prop("size", "Size", property_type::prop_vec3);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_vec3(glm::vec3{1.0f, 1.0f, 1.0f});

		add_prop("rows", "Rows", property_type::prop_int);
		set_prop_min_max(2, 100);
		set_prop_default_value_int(2);

		add_prop("columns", "Columns", property_type::prop_int);
		set_prop_min_max(2, 100);
		set_prop_default_value_int(2);
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

		const auto size = eval_vec3("size");
		const auto center = eval_vec3("center");

		const auto rows = eval_int("rows");
		const auto columns = eval_int("columns");

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

		add_prop("vertices", "Vertices", property_type::prop_int);
		set_prop_min_max(3, 500);
		set_prop_default_value_int(32);

		add_prop("radius", "Radius", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(1.0f);
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

		const auto segs = eval_int("vertices");
		const auto dia = eval_float("radius");

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

		add_prop("vertices", "Vertices", property_type::prop_int);
		set_prop_min_max(3, 500);
		set_prop_default_value_int(32);

		add_prop("radius", "Radius", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(1.0f);

		add_prop("depth", "Depth", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(1.0f);
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

		const auto segs = eval_int("vertices");
		const auto dia = eval_float("radius");
		const auto depth = eval_float("depth");

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

		add_prop("vertices", "Vertices", property_type::prop_int);
		set_prop_min_max(3, 500);
		set_prop_default_value_int(32);

		add_prop("minor_radius", "Minor Radius", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(0.0f);

		add_prop("major_radius", "Major Radius", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(1.0f);

		add_prop("depth", "Depth", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(1.0f);
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

		const auto segs = eval_int("vertices");
		const auto dia1 = eval_float("major_radius");
		const auto dia2 = eval_float("minor_radius");
		const auto depth = eval_float("depth");

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

		add_prop("radius", "Radius", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(1.0f);
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

		const auto dia = eval_float("radius");
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

static inline glm::vec3 get_normal(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2)
{
	const auto n0 = v0 - v1;
	const auto n1 = v2 - v1;

	return glm::cross(n1, n0);
}

static const char *NOM_NORMAL = "Normal";
static const char *AIDE_NORMAL = "Éditer les normales.";

class OperateurNormal : public Operateur {
public:
	OperateurNormal(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		add_prop("flip", "Flip", property_type::prop_bool);
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

		const auto flip = eval_bool("flip");

		for (auto &prim : primitive_iterator(this->m_collection, Mesh::id)) {
			auto mesh = static_cast<Mesh *>(prim);
			auto normals = mesh->attribute("normal", ATTR_TYPE_VEC3);
			auto points = mesh->points();

			normals->resize(points->size());

			auto polys = mesh->polys();

			for (size_t i = 0, ie = points->size(); i < ie ; ++i) {
				normals->vec3(i, glm::vec3(0.0f));
			}

			parallel_for(tbb::blocked_range<size_t>(0, polys->size()),
						 [&](const tbb::blocked_range<size_t> &r)
			{
				for (auto i = r.begin(), ie = r.end(); i < ie ; ++i) {
					const auto &quad = (*polys)[i];

					const auto v0 = (*points)[quad[0]];
					const auto v1 = (*points)[quad[1]];
					const auto v2 = (*points)[quad[2]];

					const auto normal = get_normal(v0, v1, v2);

					normals->vec3(quad[0], normals->vec3(quad[0]) + normal);
					normals->vec3(quad[1], normals->vec3(quad[1]) + normal);
					normals->vec3(quad[2], normals->vec3(quad[2]) + normal);

					if (quad[3] != INVALID_INDEX) {
						normals->vec3(quad[3], normals->vec3(quad[3]) + normal);
					}
				}
			});

			if (flip) {
				for (size_t i = 0, ie = points->size(); i < ie ; ++i) {
					normals->vec3(i, -glm::normalize(normals->vec3(i)));
				}
			}
		}
	}
};

/* ************************************************************************** */

static const char *NOM_BRUIT = "Bruit";
static const char *AIDE_BRUIT = "Ajouter du bruit.";

enum {
	DIRECTION_X = 0,
	DIRECTION_Y = 1,
	DIRECTION_Z = 2,
	DIRECTION_TOUTE = 3,
	DIRECTION_NORMALE = 4,
};

enum {
	BRUIT_SIMPLEX = 0,
	BRUIT_PERLIN = 1,
	BRUIT_FLUX = 2,
};

class OperateurBruit : public Operateur {
	BruitPerlin3D m_bruit_perlin;
	BruitFlux3D m_bruit_flux;

public:
	OperateurBruit(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		EnumProperty prop_enum_bruit;
		prop_enum_bruit.insert("Simplex", BRUIT_SIMPLEX);
		prop_enum_bruit.insert("Perlin", BRUIT_PERLIN);
		prop_enum_bruit.insert("Flux", BRUIT_FLUX);

		add_prop("bruit", "Bruit", property_type::prop_enum);
		set_prop_enum_values(prop_enum_bruit);

		EnumProperty prop_enum;
		prop_enum.insert("X", DIRECTION_X);
		prop_enum.insert("Y", DIRECTION_Y);
		prop_enum.insert("Z", DIRECTION_Z);
		prop_enum.insert("Toute", DIRECTION_TOUTE);
		prop_enum.insert("Normale", DIRECTION_NORMALE);

		add_prop("direction", "Direction", property_type::prop_enum);
		set_prop_enum_values(prop_enum);

		add_prop("octaves", "Octaves", property_type::prop_int);
		set_prop_min_max(1, 10);
		set_prop_default_value_int(1);

		add_prop("frequency", "Frequency", property_type::prop_float);
		set_prop_min_max(0.0f, 1.0f);
		set_prop_default_value_float(1.0f);

		add_prop("amplitude", "Amplitude", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(1.0f);

		add_prop("persistence", "Persistence", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(1.0f);

		add_prop("lacunarity", "Lacunarity", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(2.0f);

		add_prop("temps", "Temps", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(2.0f);
	}

	bool update_properties() override
	{
		const auto bruit = eval_enum("bruit");

		set_prop_visible("temps", bruit == BRUIT_FLUX);

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

		const auto octaves = eval_int("octaves");
		const auto lacunarity = eval_float("lacunarity");
		const auto persistence = eval_float("persistence");
		const auto ofrequency = eval_float("frequency");
		const auto oamplitude = eval_float("amplitude");
		const auto direction = eval_enum("direction");
		const auto bruit = eval_enum("bruit");

		if (bruit == BRUIT_FLUX) {
			const auto temps_bruit = eval_float("temps");
			m_bruit_flux.change_temps(temps_bruit);
		}

		for (auto prim : primitive_iterator(this->m_collection)) {
			PointList *points;

			Attribute *normales = nullptr;

			if (prim->typeID() == Mesh::id) {
				auto mesh = static_cast<Mesh *>(prim);
				points = mesh->points();
				normales = mesh->attribute("normal", ATTR_TYPE_VEC3);

				if (direction == DIRECTION_NORMALE && normales == nullptr) {
					this->ajoute_avertissement("Absence de normales pour calculer le bruit !");
					continue;
				}
			}
			else if (prim->typeID() == PrimPoints::id) {
				auto prim_points = static_cast<PrimPoints *>(prim);
				points = prim_points->points();

				if (direction == DIRECTION_NORMALE) {
					this->ajoute_avertissement("On ne peut calculer le bruit suivant la normale sur un nuage de points !");
					continue;
				}
			}
			else {
				continue;
			}

			for (size_t i = 0, e = points->size(); i < e; ++i) {
				auto &point = (*points)[i];
				const auto x = point.x;
				const auto y = point.y;
				const auto z = point.z;
				auto valeur = 0.0f;

				auto frequency = ofrequency;
				auto amplitude = oamplitude;

				for (size_t j = 0; j < octaves; ++j) {
					if (bruit == BRUIT_SIMPLEX) {
						valeur += (amplitude * bruit_simplex_3d(x * frequency, y * frequency, z * frequency));
					}
					else if (bruit == BRUIT_PERLIN) {
						valeur += (amplitude * m_bruit_perlin(x * frequency, y * frequency, z * frequency));
					}
					else {
						valeur += (amplitude * m_bruit_flux(x * frequency, y * frequency, z * frequency));
					}

					frequency *= lacunarity;
					amplitude *= persistence;
				}

				switch (direction) {
					case DIRECTION_X:
						point.x += valeur;
						break;
					case DIRECTION_Y:
						point.y += valeur;
						break;
					case DIRECTION_Z:
						point.z += valeur;
						break;
					default:
					case DIRECTION_TOUTE:
						point.x += valeur;
						point.y += valeur;
						point.z += valeur;
						break;
					case DIRECTION_NORMALE:
					{
						const auto normale = normales->vec3(i);
						point.x += valeur * normale.x;
						point.y += valeur * normale.y;
						point.z += valeur * normale.z;
						break;
					}
				}
			}
		}
	}
};

/* ************************************************************************** */

static const char *NOM_COULEUR = "Couleur";
static const char *AIDE_COULEUR = "Ajouter de la couleur.";

enum {
	COLOR_NODE_VERTEX    = 0,
	COLOR_NODE_PRIMITIVE = 1,
};

enum {
	COLOR_NODE_UNIQUE = 0,
	COLOR_NODE_RANDOM = 1,
};

class OperateurCouleur : public Operateur {
public:
	OperateurCouleur(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		EnumProperty scope_enum_prop;
		scope_enum_prop.insert("Vertex", COLOR_NODE_VERTEX);
		scope_enum_prop.insert("Primitive", COLOR_NODE_PRIMITIVE);

		add_prop("scope", "Scope", property_type::prop_enum);
		set_prop_enum_values(scope_enum_prop);

		EnumProperty color_enum_prop;
		color_enum_prop.insert("Unique", COLOR_NODE_UNIQUE);
		color_enum_prop.insert("Random", COLOR_NODE_RANDOM);

		add_prop("fill_method", "Fill Method", property_type::prop_enum);
		set_prop_enum_values(color_enum_prop);

		add_prop("color", "Color", property_type::prop_vec3);
		set_prop_min_max(0.0f, 1.0f);
		set_prop_default_value_vec3(glm::vec3{1.0f, 1.0f, 1.0f});

		add_prop("seed", "Seed", property_type::prop_int);
		set_prop_min_max(1, 100000);
		set_prop_default_value_int(1);
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

	bool update_properties() override
	{
		auto method = eval_int("fill_method");

		if (method == COLOR_NODE_UNIQUE) {
			set_prop_visible("color", true);
			set_prop_visible("seed", false);
		}
		else if (method == COLOR_NODE_RANDOM) {
			set_prop_visible("seed", true);
			set_prop_visible("color", false);
		}

		return true;
	}

	void execute(const Context &contexte, double temps) override
	{
		entree(0)->requiers_collection(m_collection, contexte, temps);

		const auto &method = eval_int("fill_method");
		const auto &scope = eval_int("scope");
		const auto &seed = eval_int("seed");

		std::mt19937 rng(19937 + seed);
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		for (auto prim : primitive_iterator(this->m_collection)) {
			Attribute *colors;

			if (prim->typeID() == Mesh::id) {
				auto mesh = static_cast<Mesh *>(prim);
				colors = mesh->add_attribute("color", ATTR_TYPE_VEC3, mesh->points()->size());
			}
			else if (prim->typeID() == PrimPoints::id) {
				auto prim_points = static_cast<PrimPoints *>(prim);
				colors = prim_points->add_attribute("color", ATTR_TYPE_VEC3, prim_points->points()->size());
			}
			else {
				continue;
			}

			if (method == COLOR_NODE_UNIQUE) {
				const auto &color = eval_vec3("color");

				for (size_t i = 0, e = colors->size(); i < e; ++i) {
					colors->vec3(i, color);
				}
			}
			else if (method == COLOR_NODE_RANDOM) {
				if (scope == COLOR_NODE_VERTEX) {
					for (size_t i = 0, e = colors->size(); i < e; ++i) {
						colors->vec3(i, glm::vec3{dist(rng), dist(rng), dist(rng)});
					}
				}
				else if (scope == COLOR_NODE_PRIMITIVE) {
					const auto &color = glm::vec3{dist(rng), dist(rng), dist(rng)};

					for (size_t i = 0, e = colors->size(); i < e; ++i) {
						colors->vec3(i, color);
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

		add_prop("points_count", "Points Count", property_type::prop_int);
		set_prop_min_max(1, 100000);
		set_prop_default_value_int(1000);

		add_prop("bbox_min", "BBox Min", property_type::prop_vec3);
		set_prop_min_max(-10.0f, 10.0f);
		set_prop_default_value_vec3(glm::vec3{-1.0f, -1.0f, -1.0f});

		add_prop("bbox_max", "BBox Max", property_type::prop_vec3);
		set_prop_min_max(-10.0f, 10.0f);
		set_prop_default_value_vec3(glm::vec3{1.0f, 1.0f, 1.0f});
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

		const auto &point_count = eval_int("points_count");

		auto point_list = points->points();
		point_list->resize(point_count);

		const auto &bbox_min = eval_vec3("bbox_min");
		const auto &bbox_max = eval_vec3("bbox_max");

		std::uniform_real_distribution<float> dist_x(bbox_min[0], bbox_max[0]);
		std::uniform_real_distribution<float> dist_y(bbox_min[1], bbox_max[1]);
		std::uniform_real_distribution<float> dist_z(bbox_min[2], bbox_max[2]);
		std::mt19937 rng_x(19937);
		std::mt19937 rng_y(19937 + 1);
		std::mt19937 rng_z(19937 + 2);

		for (size_t i = 0; i < point_count; ++i) {
			const auto &point = glm::vec3(dist_x(rng_x), dist_y(rng_y), dist_z(rng_z));
			(*point_list)[i] = point;
		}

		points->tagUpdate();
	}
};

/* ************************************************************************** */

static const char *NOM_CREATION_ATTRIBUT = "Création attribut";
static const char *AIDE_CREATION_ATTRIBUT = "Création d'un attribut.";

class OperateurCreationAttribut : public Operateur {
public:
	OperateurCreationAttribut(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		add_prop("attribute_name", "Name", property_type::prop_string);
		set_prop_tooltip("Name of the attribute to create.");

		EnumProperty type_enum;
		type_enum.insert("Byte", ATTR_TYPE_BYTE);
		type_enum.insert("Integer", ATTR_TYPE_INT);
		type_enum.insert("Float", ATTR_TYPE_FLOAT);
		type_enum.insert("String", ATTR_TYPE_STRING);
		type_enum.insert("2D Vector", ATTR_TYPE_VEC2);
		type_enum.insert("3D Vector", ATTR_TYPE_VEC3);
		type_enum.insert("4D Vector", ATTR_TYPE_VEC4);
		type_enum.insert("3x3 Matrix", ATTR_TYPE_MAT3);
		type_enum.insert("4x4 Matrix", ATTR_TYPE_MAT4);

		add_prop("attribute_type", "Attribute Type", property_type::prop_enum);
		set_prop_enum_values(type_enum);
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

		auto name = eval_string("attribute_name");
		auto attribute_type = static_cast<AttributeType>(eval_enum("attribute_type"));

		for (Primitive *prim : primitive_iterator(m_collection)) {
			if (prim->has_attribute(name, attribute_type)) {
				std::stringstream ss;
				ss << prim->name() << " already has an attribute named " << name;

				this->ajoute_avertissement(ss.str());
				continue;
			}

			size_t attrib_size = 1;

			if (prim->typeID() == Mesh::id) {
				auto mesh = static_cast<Mesh *>(prim);

				if (attribute_type == ATTR_TYPE_VEC3) {
					attrib_size = mesh->points()->size();
				}
			}
			else if (prim->typeID() == PrimPoints::id) {
				auto point_cloud = static_cast<PrimPoints *>(prim);

				if (attribute_type == ATTR_TYPE_VEC3) {
					attrib_size = point_cloud->points()->size();
				}
			}

			prim->add_attribute(name, attribute_type, attrib_size);
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

		add_prop("attribute_name", "Name", property_type::prop_string);
		set_prop_tooltip("Name of the attribute to delete.");

		EnumProperty type_enum;
		type_enum.insert("Byte", ATTR_TYPE_BYTE);
		type_enum.insert("Integer", ATTR_TYPE_INT);
		type_enum.insert("Float", ATTR_TYPE_FLOAT);
		type_enum.insert("String", ATTR_TYPE_STRING);
		type_enum.insert("2D Vector", ATTR_TYPE_VEC2);
		type_enum.insert("3D Vector", ATTR_TYPE_VEC3);
		type_enum.insert("4D Vector", ATTR_TYPE_VEC4);
		type_enum.insert("3x3 Matrix", ATTR_TYPE_MAT3);
		type_enum.insert("4x4 Matrix", ATTR_TYPE_MAT4);

		add_prop("attribute_type", "Attribute Type", property_type::prop_enum);
		set_prop_enum_values(type_enum);
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

		auto name = eval_string("attribute_name");
		auto attribute_type = static_cast<AttributeType>(eval_enum("attribute_type"));

		for (Primitive *prim : primitive_iterator(m_collection)) {
			if (!prim->has_attribute(name, attribute_type)) {
				continue;
			}

			prim->remove_attribute(name, attribute_type);
		}
	}
};

/* ************************************************************************** */

static const char *NOM_RANDOMISATION_ATTRIBUT = "Randomisation attribut";
static const char *AIDE_RANDOMISATION_ATTRIBUT = "Randomisation d'un attribut.";

enum {
	DIST_CONSTANT = 0,
	DIST_UNIFORM,
	DIST_GAUSSIAN,
	DIST_EXPONENTIAL,
	DIST_LOGNORMAL,
	DIST_CAUCHY,
	DIST_DISCRETE,
};

class OperateurRandomisationAttribut : public Operateur {
public:
	OperateurRandomisationAttribut(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		add_prop("attribute_name", "Name", property_type::prop_string);
		set_prop_tooltip("Name of the attribute to randomise.");

		EnumProperty type_enum;
		type_enum.insert("Byte", ATTR_TYPE_BYTE);
		type_enum.insert("Integer", ATTR_TYPE_INT);
		type_enum.insert("Float", ATTR_TYPE_FLOAT);
		type_enum.insert("String", ATTR_TYPE_STRING);
		type_enum.insert("2D Vector", ATTR_TYPE_VEC2);
		type_enum.insert("3D Vector", ATTR_TYPE_VEC3);
		type_enum.insert("4D Vector", ATTR_TYPE_VEC4);
		type_enum.insert("3x3 Matrix", ATTR_TYPE_MAT3);
		type_enum.insert("4x4 Matrix", ATTR_TYPE_MAT4);

		add_prop("attribute_type", "Attribute Type", property_type::prop_enum);
		set_prop_enum_values(type_enum);

		EnumProperty dist_enum;
		dist_enum.insert("Constant", DIST_CONSTANT);
		dist_enum.insert("Uniform", DIST_UNIFORM);
		dist_enum.insert("Gaussian", DIST_GAUSSIAN);

		add_prop("distribution", "Distribution", property_type::prop_enum);
		set_prop_enum_values(dist_enum);

		/* Constant Value */
		add_prop("value", "Value", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(1.0f);

		/* Min/Max Value */
		add_prop("min_value", "Min Value", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(0.0f);

		add_prop("max_value", "Max Value", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(1.0f);

		/* Gaussian Distribution */
		add_prop("mean", "Mean", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(0.0f);

		add_prop("stddev", "Standard Deviation", property_type::prop_float);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_default_value_float(1.0f);
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

	bool update_properties() override
	{
		const auto distribution = eval_enum("distribution");

		set_prop_visible("value", distribution == DIST_CONSTANT);
		set_prop_visible("min_value", distribution == DIST_UNIFORM);
		set_prop_visible("max_value", distribution == DIST_UNIFORM);
		set_prop_visible("mean", distribution == DIST_GAUSSIAN);
		set_prop_visible("stddev", distribution == DIST_GAUSSIAN);

		return true;
	}

	void execute(const Context &contexte, double temps) override
	{
		entree(0)->requiers_collection(m_collection, contexte, temps);

		auto name = eval_string("attribute_name");
		auto attribute_type = static_cast<AttributeType>(eval_enum("attribute_type"));
		auto distribution = eval_enum("distribution");
		auto value = eval_float("value");
		auto min_value = eval_float("min_value");
		auto max_value = eval_float("max_value");
		auto mean = eval_float("mean");
		auto stddev = eval_float("stddev");

		std::mt19937 rng(19993754);

		if (attribute_type != ATTR_TYPE_VEC3) {
			std::stringstream ss;
			ss << "Only 3D Vector attributes are supported for now!";

			this->ajoute_avertissement(ss.str());
			return;
		}

		for (Primitive *prim : primitive_iterator(m_collection)) {
			auto attribute = prim->attribute(name, attribute_type);

			if (!attribute) {
				std::stringstream ss;
				ss << prim->name() << " does not have an attribute named \"" << name
				   << "\" of type " << static_cast<int>(attribute_type);

				this->ajoute_avertissement(ss.str());
				continue;
			}

			switch (distribution) {
				case DIST_CONSTANT:
				{
					for (size_t i = 0; i < attribute->size(); ++i) {
						attribute->vec3(i, glm::vec3{value, value, value});
					}

					break;
				}
				case DIST_UNIFORM:
				{
					std::uniform_real_distribution<float> dist(min_value, max_value);

					for (size_t i = 0; i < attribute->size(); ++i) {
						attribute->vec3(i, glm::vec3{dist(rng), dist(rng), dist(rng)});
					}

					break;
				}
				case DIST_GAUSSIAN:
				{
					std::normal_distribution<float> dist(mean, stddev);

					for (size_t i = 0; i < attribute->size(); ++i) {
						attribute->vec3(i, glm::vec3{dist(rng), dist(rng), dist(rng)});
					}

					break;
				}
			}
		}
	}
};

/* ************************************************************************** */

static const char *NOM_CREATION_SEGMENTS = "Création courbes";
static const char *AIDE_CREATION_SEGMENTS = "Création de courbes.";

template <typename T, glm::precision P>
auto interp(const glm::detail::tvec3<T, P> &a, const glm::detail::tvec3<T, P> &b, const T &t)
{
	return a * (static_cast<T>(1) - t) + b * t;
}

enum {
	CREER_COURBES_VERTS = 0,
	CREER_COURBES_POLYS = 1,
};

class OperateurCreationCourbes : public Operateur {
public:
	OperateurCreationCourbes(Noeud *noeud, const Context &contexte)
		: Operateur(noeud, contexte)
	{
		entrees(1);
		sorties(1);

		/* méthode */
		EnumProperty prop_enum;
		prop_enum.insert("Vertices", CREER_COURBES_VERTS);
		prop_enum.insert("Polygones", CREER_COURBES_POLYS);

		add_prop("méthode", "Méthode", property_type::prop_enum);
		set_prop_enum_values(prop_enum);
		set_prop_default_value_int(0);

		/* graine */
		add_prop("graine", "Graine", property_type::prop_int);
		set_prop_min_max(1, 100000);
		set_prop_default_value_int(1);

		/* nombre courbes */
		add_prop("nombre_courbes", "Nombre Courbes", property_type::prop_int);
		set_prop_min_max(1, 1000);
		set_prop_default_value_int(100);
		set_prop_tooltip("Nombre de courbes par polygone.");

		add_prop("segments", "Segments", property_type::prop_int);
		set_prop_default_value_int(1);
		set_prop_min_max(1, 10);
		set_prop_tooltip("Nombre de segments dans chaque courbe.");

		add_prop("taille", "Taille", property_type::prop_float);
		set_prop_default_value_float(1);
		set_prop_min_max(0.0f, 10.0f);
		set_prop_tooltip("Taille de chaque segment.");

		add_prop("normale", "Normale", property_type::prop_vec3);
		set_prop_default_value_vec3(glm::vec3{0.0f, 1.0f, 0.0f});
		set_prop_min_max(-1.0f, 1.0f);
		set_prop_tooltip("Direction de la courbe.");
	}

	bool update_properties() override
	{
		auto methode = eval_enum("méthode");

		set_prop_visible("graine", methode == CREER_COURBES_POLYS);
		set_prop_visible("nombre_courbes", methode == CREER_COURBES_POLYS);

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

		const auto segment_number = eval_int("segments");
		const auto segment_normal = eval_vec3("normale");
		const auto segment_size = eval_float("taille");
		const auto methode = eval_enum("méthode");

		auto segment_prim = static_cast<SegmentPrim *>(m_collection->build("SegmentPrim"));
		auto output_edges = segment_prim->edges();
		auto output_points = segment_prim->points();

		auto num_points = 0ul;
		auto total_points = 0ul;

		if (methode == CREER_COURBES_VERTS) {
			total_points = input_points->size() * (segment_number + 1);

			output_edges->reserve(input_points->size() * segment_number);
			output_points->reserve(total_points);
			auto head = 0;

			for (size_t i = 0; i < input_points->size(); ++i) {
				auto point = (*input_points)[i];

				output_points->push_back(point);
				++num_points;

				for (int j = 0; j < segment_number; ++j, ++num_points) {
					point += (segment_size * segment_normal);
					output_points->push_back(point);

					output_edges->push_back(glm::uvec2{head, ++head});
				}

				++head;
			}
		}
		else if (methode == CREER_COURBES_POLYS) {
			const auto polys = input_mesh->polys();

			const auto nombre_courbes = eval_int("nombre_courbes");
			const auto nombre_polys = polys->size();

			total_points = nombre_polys * nombre_courbes * (segment_number + 1);

			output_edges->reserve((nombre_courbes * segment_number) * nombre_polys);
			output_points->reserve(total_points);

			const auto graine = eval_int("graine");
			auto head = 0;

			std::mt19937 rng(19937 + graine);
			std::uniform_real_distribution<float> dist(0.0f, 1.0f);

			for (size_t i = 0; i < nombre_polys; ++i) {
				const auto poly = (*polys)[i];
				const auto v1 = (*input_points)[poly[0]];
				const auto v2 = (*input_points)[poly[1]];
				const auto v3 = (*input_points)[poly[2]];
				const auto v4 = (poly[3] != INVALID_INDEX) ? (*input_points)[poly[3]] : glm::vec3(0.0f);

				for (size_t j = 0; j < nombre_courbes; ++j) {
					const auto t1 = dist(rng);
					const auto t2 = dist(rng);
					const auto t3 = dist(rng);

					auto pos = interp(v1, v2, t1);
					pos += interp(v2, v3, t2);

					if (poly[3] != INVALID_INDEX) {
						pos += interp(v3, v4, t3);

						const auto t4 = dist(rng);
						pos += interp(v4, v1, t4);
					}
					else {
						pos += interp(v3, v1, t3);
					}

					output_points->push_back(pos);
					++num_points;

					for (int k = 0; k < segment_number; ++k, ++num_points) {
						pos += (segment_size * segment_normal);
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

		add_prop("prise", "Prise", property_type::prop_int);
		set_prop_tooltip("L'index de la prise à évaluer");
		set_prop_default_value_int(0);
		set_prop_min_max(0, 1);
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
		const auto prise = eval_int("prise");
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

		/* graine */
		add_prop("graine", "Graine", property_type::prop_int);
		set_prop_min_max(1, 100000);
		set_prop_default_value_int(1);

		/* nombre courbes */
		add_prop("nombre_points_polys", "Nombre Points Par Polygones", property_type::prop_int);
		set_prop_min_max(1, 1000);
		set_prop_default_value_int(100);
		set_prop_tooltip("Nombre de points par polygone.");
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

		const auto input_mesh = static_cast<Mesh *>(iter.get());
		const auto points_entrees = input_mesh->points();
		const auto polygones = input_mesh->polys();
		const auto nombre_polys = polygones->size();

		auto nuage_points = static_cast<PrimPoints *>(m_collection->build("PrimPoints"));
		auto points_sorties = nuage_points->points();

		const auto nombre_points_polys = eval_int("nombre_points_polys");
		const auto nombre_points = nombre_polys * nombre_points_polys;

		points_sorties->reserve(nombre_points);

		const auto graine = eval_int("graine");

		std::mt19937 rng(19937 + graine);
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		for (size_t i = 0, f = nombre_polys; i < f; ++i) {
			const auto poly = (*polygones)[i];
			const auto v1 = (*points_entrees)[poly[0]];
			const auto v2 = (*points_entrees)[poly[1]];
			const auto v3 = (*points_entrees)[poly[2]];
			const auto v4 = (poly[3] != INVALID_INDEX) ? (*points_entrees)[poly[3]] : glm::vec3(0.0f);

			for (size_t j = 0; j < nombre_points_polys; ++j) {
				const auto t1 = dist(rng);
				const auto t2 = dist(rng);
				const auto t3 = dist(rng);

				auto pos = interp(v1, v2, t1);
				pos += interp(v2, v3, t2);

				if (poly[3] != INVALID_INDEX) {
					pos += interp(v3, v4, t3);

					const auto t4 = dist(rng);
					pos += interp(v4, v1, t4);
				}
				else {
					pos += interp(v3, v1, t3);
				}

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

	/* Opérateurs physiques. */

	categorie = "Physique";

	usine->enregistre_type(NOM_GRAVITE,
						   cree_description<OperateurGravite>(NOM_GRAVITE,
															  AIDE_GRAVITE,
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
