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

#include "object.h"

#include <glm/gtc/matrix_transform.hpp>
#include <kamikaze/context.h>
#include <kamikaze/primitive.h>

#include "operateurs/operateurs_standards.h"

#include "scene.h"
#include "task.h"

#include "ui/paramfactory.h"

Object::Object(const Context &contexte)
	: m_graph(contexte)
{
	add_input("Parent");
	add_output("Child");

	ajoute_propriete("position", kangao::TypePropriete::VECTEUR);
	ajoute_propriete("rotation", kangao::TypePropriete::VECTEUR);
	ajoute_propriete("taille", kangao::TypePropriete::VECTEUR);

	updateMatrix();
}

PrimitiveCollection *Object::collection() const
{
	return m_collection;
}

void Object::collection(PrimitiveCollection *coll)
{
	m_collection = coll;
}

void Object::matrix(const glm::mat4 &m)
{
	m_matrix = m;
}

const glm::mat4 &Object::matrix() const
{
	return m_matrix;
}

void Object::ajoute_noeud(Noeud *noeud)
{
	m_graph.ajoute(noeud);
	m_graph.noeud_actif(noeud);

	/* À FAIRE : quand on ouvre un fichier de sauvegarde, il y a un crash quand
	 * on clique dans l'éditeur de graphe. Ceci n'est sans doute pas la bonne
	 * correction. */
	m_graph.ajoute_selection(noeud);
}

Graph *Object::graph()
{
	return &m_graph;
}

const Graph *Object::graph() const
{
	return &m_graph;
}

void Object::updateMatrix()
{
	const auto m_pos = evalue_vecteur("position");
	const auto m_rotation = evalue_vecteur("rotation");
	const auto m_scale = evalue_vecteur("taille");

	m_matrix = glm::mat4(1.0f);
	m_matrix = glm::translate(m_matrix, m_pos);
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	m_matrix = glm::scale(m_matrix, m_scale);

	m_inv_matrix = glm::inverse(m_matrix);
}

void Object::addChild(Object *child)
{
	m_children.push_back(child);
	child->parent(this);
}

void Object::removeChild(Object *child)
{
	auto iter = std::find(m_children.begin(), m_children.end(), child);
	assert(iter != m_children.end());
	m_children.erase(iter);
	child->parent(nullptr);
}

const std::vector<Object *> &Object::children() const
{
	return m_children;
}

Object *Object::parent() const
{
	return m_parent;
}

void Object::parent(Object *parent)
{
	m_parent = parent;
}
