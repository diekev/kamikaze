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

#include "widgetbase.h"

class QFrame;
class QHBoxLayout;
class QScrollArea;
class QVBoxLayout;

namespace kangao {

class Manipulable;

}  /* namespace kangao */

class PropertiesWidget : public WidgetBase {
	Q_OBJECT

	QWidget *m_widget;
	QWidget *m_conteneur_disposition;
	QWidget *m_conteneur_alarmes;

	QScrollArea *m_scroll;
	QVBoxLayout *m_disposition_widget;

	kangao::Manipulable *m_manipulable = nullptr;

public:
	explicit PropertiesWidget(QWidget *parent = nullptr);
	~PropertiesWidget() = default;

	void update_state(event_type event) override;

private:
	void drawProperties(class Persona *persona, bool set_context);

	void dessine_interface(kangao::Manipulable *manipulable, const char *chemin_interface);

	void ajourne_manipulable() override;

	void efface_disposition();

	void evalObjectGraph();

	void tagObjectUpdate();
};
