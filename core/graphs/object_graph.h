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
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#pragma once

#include <kamikaze/noeud.h>
#include <kamikaze/primitive.h>
#include <memory>
#include <vector>

class Context;

enum {
	NOEUD_SELECTIONE = (1 << 0),
	NOEUD_DILATE     = (1 << 1),
	NOEUD_CONTRACTE  = (1 << 2),
};

class Graph {
	std::vector<std::unique_ptr<Noeud>> m_noeuds{};
	std::vector<Noeud *> m_noeuds_selectiones{};

	Noeud *m_noeud_actif = nullptr;

	bool m_besoin_actualisation;

	float m_zoom;

public:
	explicit Graph(const Context &contexte);

	void ajoute(Noeud *noeud);
	void enleve(Noeud *noeud);

	void connecte(PriseSortie *de, PriseEntree *a);
	void deconnecte(PriseSortie *de, PriseEntree *a);

	const std::vector<std::unique_ptr<Noeud>> &noeuds() const;

	void noeud_actif(Noeud *noeud);

	Noeud *noeud_actif() const;

	Noeud *sortie() const;

	void ajoute_selection(Noeud *noeud);

	void enleve_selection(Noeud *noeud);

	void zoom(float valeur);

	float zoom() const;
};
