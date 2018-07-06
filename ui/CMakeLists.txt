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
	${DANJO_INCLUDE_DIRS}
	${QT5_CORE_INCLUDE_DIRS}
	${QT5_GUI_INCLUDE_DIRS}
	${QT5_OPENGL_INCLUDE_DIRS}
	${QT5_WIDGETS_INCLUDE_DIRS}
)

set(FICHIERS_DANJO
	scripts_danjo/disposition_ligne_temps.jo
	scripts_danjo/menu_debogage.jo
	scripts_danjo/menu_editeur_noeud.jo
	scripts_danjo/menu_edition.jo
	scripts_danjo/menu_fichier.jo
	scripts_danjo/menu_objet.jo
	scripts_danjo/menu_prereglage.jo
	scripts_danjo/operateur_bruit.jo
	scripts_danjo/operateur_commutateur.jo
	scripts_danjo/operateur_couleur.jo
	scripts_danjo/operateur_creation_boite.jo
	scripts_danjo/operateur_creation_cercle.jo
	scripts_danjo/operateur_creation_cone.jo
	scripts_danjo/operateur_creation_courbes.jo
	scripts_danjo/operateur_creation_grille.jo
	scripts_danjo/operateur_creation_icosphere.jo
	scripts_danjo/operateur_creation_nuage_point.jo
	scripts_danjo/operateur_creation_torus.jo
	scripts_danjo/operateur_creation_tube.jo
	scripts_danjo/operateur_dispersion_points.jo
	scripts_danjo/operateur_normal.jo
	scripts_danjo/operateur_randomise_attribut.jo
	scripts_danjo/operateur_suppression_attribut.jo
	scripts_danjo/operateur_transformation.jo
	scripts_danjo/proprietes_objet.jo
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

	nodes/node_compound.cc
	nodes/node_connection.cc
	nodes/node_node.cc
	nodes/node_port.cc
	nodes/vue_editeur_noeud.cc

	${FICHIERS_DANJO}
)

target_include_directories(kmk_ui PUBLIC "${INC_SYS}")

install(
	FILES ${FICHIERS_DANJO}
	DESTINATION interface
)
