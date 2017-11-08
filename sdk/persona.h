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

#include <algorithm>
#include <experimental/any>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>

enum class property_type {
	prop_bool,
	prop_int,
	prop_enum,
	prop_float,
	prop_vec3,
	prop_string,
	prop_input_file,
	prop_output_file,
	prop_list,
};

struct EnumPair {
	std::string name;
	int value;
};

struct EnumProperty {
	std::vector<EnumPair> m_props;

	void insert(std::string name, int value)
	{
		EnumPair pair;
		pair.name = std::move(name);
		pair.value = value;

		m_props.push_back(std::move(pair));
	}
};

struct Property {
	std::string name;
	std::string ui_name;
	std::string tooltip;
	property_type type;

	std::experimental::any data;
	std::experimental::any default_val;

	EnumProperty enum_items;

	float min, max;
	bool visible;
};

/**
 * @brief Persona is the base class which defines the characteristics of the
 * objects that can have properties exposed in the UI.
 */
class Persona {
	std::vector<Property> m_props;

public:
	virtual ~Persona() = default;

	void add_prop(std::string name, std::string ui_name, property_type type);

	void set_prop_visible(const std::string &prop_name, bool visible);

	virtual bool update_properties();

	int eval_int(const std::string &prop_name);

	float eval_float(const std::string &prop_name);

	int eval_enum(const std::string &prop_name);

	int eval_bool(const std::string &prop_name);

	glm::vec3 eval_vec3(const std::string &prop_name);

	std::string eval_string(const std::string &prop_name);

	void set_prop_min_max(const float min, const float max);

	void set_prop_enum_values(const EnumProperty &enum_prop);
	void set_prop_enum_values(const std::string &prop_name, const EnumProperty &enum_prop);

	void set_prop_default_value_int(int value);

	void set_prop_default_value_float(float value);

	void set_prop_default_value_bool(bool value);

	void set_prop_default_value_string(const std::string &value);

	void set_prop_default_value_vec3(const glm::vec3 &value);

	void set_prop_tooltip(std::string tooltip);

	std::vector<Property> &props();

	void valeur_propriete_bool(const std::string &prop_name, bool valeur);
	void valeur_propriete_int(const std::string &prop_name, int valeur);
	void valeur_propriete_float(const std::string &prop_name, float valeur);
	void valeur_propriete_vec3(const std::string &prop_name, const glm::vec3 &valeur);
	void valeur_propriete_string(const std::string &prop_name, const std::string &valeur);

private:
	inline Property *find_property(const std::string &prop_name)
	{

		const auto &iter = std::find_if(m_props.begin(), m_props.end(),
		                                [&](const Property &prop)
		{
			return prop.name == prop_name;
		});

		if (iter == m_props.end()) {
			std::cerr << "Cannot find prop: " << prop_name << '\n';
			return nullptr;
		}

		const auto index = iter - m_props.begin();
		return &m_props[index];
	}
};
