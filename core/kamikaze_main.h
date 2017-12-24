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

#pragma once

#include <kamikaze/operateur.h>
#include <kamikaze/primitive.h>

#include <numero7/systeme_fichier/shared_library.h>

#include "scene.h"

class Main final {
	std::vector<numero7::systeme_fichier::shared_library> m_greffons;

	std::unique_ptr<PrimitiveFactory> m_primitive_factory;
	std::unique_ptr<UsineOperateur> m_usine_operateur;
	std::unique_ptr<Scene> m_scene;

	std::string m_chemin_projet{};

	bool m_projet_ouvert = false;

public:
	Main();

	/* Disallow copy. */
	Main(const Main &other) = delete;
	Main &operator=(const Main &other) = delete;

	void initialize();
	void charge_greffons();

	PrimitiveFactory *primitive_factory() const;
	UsineOperateur *usine_operateur() const;
	Scene *scene() const;

	std::string chemin_projet() const;

	void chemin_projet(const std::string &chemin);

	bool projet_ouvert() const;

	void projet_ouvert(bool ouinon);

	const std::vector<numero7::systeme_fichier::shared_library> &greffons() const;
};
