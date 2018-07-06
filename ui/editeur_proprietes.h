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

#include "base_editeur.h"

class QScrollArea;
class QVBoxLayout;

namespace danjo {

class Manipulable;

}  /* namespace danjo */

class EditeurProprietes : public BaseEditeur {
	Q_OBJECT

	QWidget *m_widget;
	QWidget *m_conteneur_disposition;
	QWidget *m_conteneur_alarmes;

	QScrollArea *m_scroll;
	QVBoxLayout *m_disposition_widget;

	danjo::Manipulable *m_manipulable = nullptr;

public:
	explicit EditeurProprietes(QWidget *parent = nullptr);
	~EditeurProprietes() = default;

	void update_state(event_type event) override;

private:
	void dessine_interface(danjo::Manipulable *manipulable, const char *chemin_interface);

	void ajourne_manipulable() override;

	void efface_disposition();

	void evalue_graphe();

	void ajourne_objet();
};
