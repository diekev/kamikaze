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

#include <glm/glm.hpp>

class BruitPerlin3D {
protected:
	static const unsigned int N = 128;
	glm::vec3 m_basis[N];
	int m_perm[N];

public:
	BruitPerlin3D(unsigned int seed = 171717);

	void reinitialise(unsigned int seed);

	float operator()(float x, float y, float z) const;

	float operator()(const glm::vec3 &x) const;

	unsigned int index_hash(int i, int j, int k) const;
};

class BruitFlux3D : public BruitPerlin3D {
protected:
	glm::vec3 original_basis[N];
	float taux_tournoiement[N];
	glm::vec3 axe_tournoiement[N];

public:
	BruitFlux3D(unsigned int graine = 171717, float variation_tournoiement = 0.2);

	/* La période de répétition est approximativement de 1. */
	void change_temps(float temps);
};

float bruit_simplex_3d(float x, float y, float z);
