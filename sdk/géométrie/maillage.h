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

#include "../primitive.h"

struct Sommet;
struct Arete;
struct Polygone;

struct Sommet {
	glm::vec3 pos;

	/* Une des aretes émanants du sommet. */
	Arete *arete;
};

struct Arete {
	/* Sommet à la fin de l'arête. */
	Sommet *sommet;

	/* Arête adjacent orientée opposément. */
	Arete *pair;

	/* Polygone que l'arête borde. */
	Polygone *polygone;

	/* Arête suivante autour du polygone. */
	Arete *suivante;
};

struct Polygone {
	/* Une des arêtes bordant le polygone. */
	Arete *arete;
};

class Maillage : public Primitive {
	std::vector<Sommet *> m_sommets;
	std::vector<Arete *> m_aretes;
	std::vector<Polygone *> m_polygones;

public:
	void cree_polygone(const glm::vec3 &pos0, const glm::vec3 &pos1, const glm::vec3 &pos2, const glm::vec3 &pos3)
	{
		cree_arete(pos0, pos1);
		cree_arete(pos1, pos2);
		cree_arete(pos2, pos3);
		cree_arete(pos3, pos0);
	}

	void cree_arete(const glm::vec3 &pos0, const glm::vec3 &pos1)
	{
		auto sommet0 = new Sommet;
		sommet0->pos = pos0;
		m_sommets.push_back(sommet0);

		auto sommet1 = new Sommet;
		sommet1->pos = pos1;
		m_sommets.push_back(sommet1);

		auto arete0 = new Arete;
		sommet0->arete = arete0;
		m_aretes.push_back(arete0);

		auto arete1 = new Arete;
		sommet1->arete = arete1;
		m_aretes.push_back(arete1);

		arete0->pair = arete1;
		arete1->pair = arete0;
	}
};
