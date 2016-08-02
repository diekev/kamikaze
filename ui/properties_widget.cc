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

#include "properties_widget.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QScrollArea>

#include <kamikaze/context.h>
#include <kamikaze/nodes.h>

#include "paramcallback.h"
#include "paramfactory.h"
#include "utils_ui.h"

#include "core/graphs/object_graph.h"
#include "core/object.h"
#include "core/scene.h"

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QWidget(parent)
    , m_widget(new QWidget())
    , m_scroll(new QScrollArea())
    , m_layout(new QGridLayout(m_widget))
    , m_hbox_layout(new QHBoxLayout())
{
	setLayout(m_hbox_layout);

	QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
	sizePolicy2.setHorizontalStretch(0);
	sizePolicy2.setVerticalStretch(0);
	sizePolicy2.setHeightForWidth(m_widget->sizePolicy().hasHeightForWidth());

	m_widget->setSizePolicy(sizePolicy2);

	m_scroll->setWidget(m_widget);
	m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_scroll->setWidgetResizable(true);
	m_scroll->setFrameShape(QFrame::StyledPanel);
	m_scroll->setFrameShadow(QFrame::Raised);

	m_hbox_layout->addWidget(m_scroll);
}

void PropertiesWidget::update_state(int event_type)
{
	Persona *persona = nullptr;
	bool set_context = true;
	auto scene = m_context->scene;

	if (scene->currentObject() == nullptr) {
		return;
	}

	if (event_type == OBJECT_ADDED || event_type == OBJECT_SELECTED) {
		auto object = scene->currentObject();
		persona = object;
	}
	else if (event_type == NODE_SELECTED) {
		auto object = scene->currentObject();
		auto graph = object->graph();
		auto node = graph->active_node();

		if (!node) {
			return;
		}

		persona = node;

		/* Only update/evaluate the graph if the node is connected. */
		set_context = node->isLinked();
	}
	else if (event_type == OBJECT_REMOVED || event_type == NODE_REMOVED) {
		clear_layout(m_layout);
		return;
	}
	else {
		return;
	}

	drawProperties(persona, set_context);
}

void PropertiesWidget::evalObjectGraph()
{
	auto scene = m_context->scene;

	scene->evalObjectDag(m_context, scene->currentObject());
	scene->notify_listeners(-1);
}

void PropertiesWidget::tagObjectUpdate()
{
	m_context->scene->tagObjectUpdate();
}

void PropertiesWidget::updateProperties()
{
	auto scene = m_context->scene;
	auto object = scene->currentObject();

	if (!object) {
		return;
	}

	Persona *persona = nullptr;
	auto set_context = true;

	if (m_context->edit_mode) {
		auto graph = object->graph();
		auto node = graph->active_node();

		if (node == nullptr) {
			return;
		}

		persona = node;

		/* Only update/evaluate the graph if the node is connected. */
		set_context = node->isLinked();
	}
	else {
		persona = object;
	}

	if (persona->update_properties()) {
		drawProperties(persona, set_context);
	}
}

void PropertiesWidget::drawProperties(Persona *persona, bool set_context)
{
	clear_layout(m_layout);

	ParamCallback cb(m_layout);
	ParamCallback *callback = &cb;

	for (Property &prop : persona->props()) {
		if (!prop.visible) {
			continue;
		}

		assert(!prop.data.empty());

		switch (prop.type) {
			case property_type::prop_bool:
				bool_param(callback,
				           prop.name.c_str(),
				           any_cast<bool>(&prop.data),
				           any_cast<bool>(prop.data));
				break;
			case property_type::prop_float:
				float_param(callback,
				            prop.name.c_str(),
				            any_cast<float>(&prop.data),
				            prop.min, prop.max,
				            any_cast<float>(prop.data));
				break;
			case property_type::prop_int:
				int_param(callback,
				          prop.name.c_str(),
				          any_cast<int>(&prop.data),
				          prop.min, prop.max,
				          any_cast<int>(prop.data));
				break;
			case property_type::prop_enum:
				enum_param(callback,
				           prop.name.c_str(),
				           any_cast<int>(&prop.data),
				           prop.enum_items,
				           any_cast<int>(prop.data));
				break;
			case property_type::prop_vec3:
				xyz_param(callback,
				          prop.name.c_str(),
				          &(any_cast<glm::vec3>(&prop.data)->x));
				break;
			case property_type::prop_input_file:
				input_file_param(callback,
				                 prop.name.c_str(),
				                 any_cast<std::string>(&prop.data));
				break;
			case property_type::prop_output_file:
				output_file_param(callback,
				                  prop.name.c_str(),
				                  any_cast<std::string>(&prop.data));
				break;
			case property_type::prop_string:
				string_param(callback,
				             prop.name.c_str(),
				             any_cast<std::string>(&prop.data),
				             any_cast<std::string>(prop.data).c_str());
				break;
		}

		if (!prop.tooltip.empty()) {
			param_tooltip(callback, prop.tooltip.c_str());
		}
	}

	if (set_context) {
		if (m_context->edit_mode) {
			cb.setContext(this, SLOT(evalObjectGraph()));
		}
		else {
			cb.setContext(this, SLOT(tagObjectUpdate()));
		}
	}

	cb.setContext(this, SLOT(updateProperties()));
}
