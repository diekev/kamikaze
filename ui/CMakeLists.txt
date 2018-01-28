# ***** BEGIN GPL LICENSE BLOCK *****
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# The Original Code is Copyright (C) 2016 Kévin Dietrich.
# All rights reserved.
#
# ***** END GPL LICENSE BLOCK *****

set(INC_SYS
	${CMAKE_CURRENT_SOURCE_DIR}/../
	${CMAKE_CURRENT_SOURCE_DIR}/../core
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/nodes/
	${CMAKE_CURRENT_BINARY_DIR}
	${EGO_INCLUDE_DIRS}
	${FILESYSTEM_INCLUDE_DIRS}
	${KAMIKAZE_INCLUDE_DIRS}
	${KANGAO_INCLUDE_DIRS}
	${QT5_CORE_INCLUDE_DIRS}
	${QT5_GUI_INCLUDE_DIRS}
	${QT5_OPENGL_INCLUDE_DIRS}
	${QT5_WIDGETS_INCLUDE_DIRS}
)

set(FICHIERS_KANGAO
	scripts_kangao/disposition_ligne_temps.kangao
	scripts_kangao/menu_debogage.kangao
	scripts_kangao/menu_editeur_noeud.kangao
	scripts_kangao/menu_edition.kangao
	scripts_kangao/menu_fichier.kangao
	scripts_kangao/menu_objet.kangao
	scripts_kangao/menu_prereglage.kangao
	scripts_kangao/operateur_bruit.kangao
	scripts_kangao/operateur_commutateur.kangao
	scripts_kangao/operateur_couleur.kangao
	scripts_kangao/operateur_creation_boite.kangao
	scripts_kangao/operateur_creation_cercle.kangao
	scripts_kangao/operateur_creation_cone.kangao
	scripts_kangao/operateur_creation_courbes.kangao
	scripts_kangao/operateur_creation_grille.kangao
	scripts_kangao/operateur_creation_icosphere.kangao
	scripts_kangao/operateur_creation_nuage_point.kangao
	scripts_kangao/operateur_creation_torus.kangao
	scripts_kangao/operateur_creation_tube.kangao
	scripts_kangao/operateur_dispersion_points.kangao
	scripts_kangao/operateur_normal.kangao
	scripts_kangao/operateur_randomise_attribut.kangao
	scripts_kangao/operateur_suppression_attribut.kangao
	scripts_kangao/operateur_transformation.kangao
	scripts_kangao/proprietes_objet.kangao
)

add_compile_options(-fPIC)

qt5_wrap_cpp(
	test_hdr_moc

	editeur_arborescence.h
	editeur_canevas.h
	editeur_graphe.h
	editeur_ligne_temps.h
	editeur_proprietes.h
	fenetre_principale.h
)

add_library(kmk_ui STATIC
	base_editeur.h
	editeur_arborescence.h
	editeur_canevas.h
	editeur_graphe.h
	editeur_ligne_temps.h
	editeur_proprietes.h
	fenetre_principale.h
	repondant_commande.h
	utils_ui.h

	nodes/node_compound.h
	nodes/node_connection.h
	nodes/node_constants.h
	nodes/node_node.h
	nodes/node_port.h
	nodes/node_scene.h
	nodes/vue_editeur_noeud.h

	${test_hdr_moc}

	base_editeur.cc
	editeur_arborescence.cc
	editeur_canevas.cc
	editeur_graphe.cc
	editeur_ligne_temps.cc
	editeur_proprietes.cc
	fenetre_principale.cc
	repondant_commande.cc
	utils_ui.cc

	nodes/node_compound.cc
	nodes/node_connection.cc
	nodes/node_node.cc
	nodes/node_port.cc
	nodes/vue_editeur_noeud.cc

	${FICHIERS_KANGAO}
)

target_include_directories(kmk_ui PUBLIC "${INC_SYS}")

install(
	FILES ${FICHIERS_KANGAO}
	DESTINATION interface
)
