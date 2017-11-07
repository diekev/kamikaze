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

#include <string>
#include <vector>

class Noeud;
class Operateur;
class PriseEntree;
class PriseSortie;

/**
 * Une prise d'entrée d'un noeud.
 */
struct PriseEntree {
	Noeud *parent = nullptr;
	PriseSortie *lien = nullptr;
	std::string nom = "";

	/**
	 * Construit une prise d'entrée avec le nom passé en paramètre.
	 */
	explicit PriseEntree(const std::string &nom_prise);
};

/**
 * Une prise de sortie d'un noeud.
 */
struct PriseSortie {
	Noeud *parent = nullptr;
	std::vector<PriseEntree *> liens{};
	std::string nom = "";

	/**
	 * Construit une prise de sortie avec le nom passé en paramètre.
	 */
	explicit PriseSortie(const std::string &nom_prise);
};

/**
 * La classe Noeud englobe un opérateur, et est le pont entre l'interface
 * utilisateur et l'opérateur installé dans le noeud.
 */
class Noeud {
	Operateur *m_operateur;
	std::vector<PriseEntree *> m_entrees = {};
	std::vector<PriseSortie *> m_sorties = {};

	std::string m_nom = "";

	/* Interface utilisateur. */
	double m_posx = 0.0;
	double m_posy = 0.0;

	int m_drapeaux = 0;

public:
	/**
	 * Détruit ce noeud.
	 */
	~Noeud();

	/**
	 * Retourne le nom de ce noeud.
	 */
	std::string nom() const;

	/**
	 * Met à jour le nom de ce noeud.
	 */
	void nom(std::string nom);

	/**
	 * Ajoute une entrée à ce noeud dont le nom sera le nom passé en paramètre.
	 */
	void ajoute_entree(const std::string &nom);

	/**
	 * Ajoute une sortie à ce noeud dont le nom sera le nom passé en paramètre.
	 */
	void ajoute_sortie(const std::string &nom);

	/**
	 * Retourn l'entrée correspondant à l'index donné
	 */
	PriseEntree *entree(int index);

	/**
	 * Retourn l'entrée correspondant au nom donné.
	 */
	PriseEntree *entree(const std::string &nom);

	/**
	 * Retourn la sortie correspondant à l'index donné
	 */
	PriseSortie *sortie(int index);

	/**
	 * Retourn la sortie correspondant au nom donné.
	 */
	PriseSortie *sortie(const std::string &nom);

	/**
	 * Retourn le vecteur contenant les entrées de ce noeud.
	 */
	std::vector<PriseEntree *> entrees() const noexcept;

	/**
	 * Retourn le vecteur contenant les sorties de ce noeud.
	 */
	std::vector<PriseSortie *> sorties() const noexcept;

	/**
	 * Retourne si oui ou non le noeud a une connection entrante ou sortante.
	 */
	bool est_connecte() const;

	/**
	 * Retourne si oui ou non une entrée de ce noeud est connectée.
	 */
	bool a_entree_connectee() const;

	/**
	 * Retourne si oui ou non une sortie de ce noeud est connectée.
	 */
	bool a_sortie_connectee() const;

	/**
	 * Synchronise les données entre l'opérateur et le noeud.
	 */
	void synchronise_donnees();

	/**
	 * Installe un opérateur dans ce noeud.
	 */
	void operateur(Operateur *operateur);

	/**
	 * Retourne l'opérateur installé dans ce noeud.
	 */
	Operateur *operateur() const;

	/**
	 * Retourne la position sur l'axe 'x' du noeud.
	 */
	double posx() const;

	/**
	 * Met à jour la position sur l'axe 'x' du noeud.
	 */
	void posx(double pos);

	/**
	 * Retourne la position sur l'axe 'y' du noeud.
	 */
	double posy() const;

	/**
	 * Met à jour la position sur l'axe 'y' du noeud.
	 */
	void posy(double pos);

	/**
	 * Retourne les drapeaux de ce noeud.
	 */
	inline int drapeaux() const
	{
		return m_drapeaux;
	}

	/**
	 * Ajoute un drapeau à ce noeud.
	 */
	inline void ajoute_drapeau(int drapeau)
	{
		m_drapeaux |= drapeau;
	}

	/**
	 * Enleve un drapeau à ce noeud.
	 */
	inline void enleve_drapeau(int drapeau)
	{
		m_drapeaux &= ~drapeau;
	}

	/**
	 * Vérifie si oui ou non ce noeud a le drapeau en question.
	 */
	inline bool a_drapeau(int drapeau) const
	{
		return (m_drapeaux & drapeau) != 0;
	}
};

/**
 * Signifie à tous les noeuds en amont qu'ils ont besoin d'être exécutés.
 */
void signifie_sale_amont(Noeud *noeud);

/**
 * Signifie à tous les noeuds en aval qu'ils ont besoin d'être exécutés.
 */
void signifie_sale_aval(Noeud *noeud);
