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

#include "any.h"

#include <algorithm>
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
	std::string tooltip;
	property_type type;

	any data;
	any default_val;

	EnumProperty enum_items;

	float min, max;
	bool visible;
};

template <typename StateType>
class Watcher {
public:
	virtual ~Watcher() = default;
	virtual void update(const std::vector<Property> &props, const StateType &state) = 0;
};

template <typename WatcherType>
class Watched {
	std::vector<WatcherType *> m_watchers;
	std::vector<Property> m_props;

public:
	virtual ~Watched() = default;

	/* Watchers. */

	void install_watcher(WatcherType *watcher)
	{
		m_watchers.push_back(watcher);
	}

	void remove_watcher(WatcherType *watcher)
	{
		auto iter = std::find(m_watchers.begin(), m_watchers.end(), watcher);
		m_watchers.erase(iter);
	}

	void notify_watchers() const
	{
		for (auto &watcher : m_watchers) {
			watcher->update(this->props());
		}
	}

	/* Properties. */

	void add_prop(std::string name, property_type type)
	{
		Property prop;
		prop.name = std::move(name);
		prop.type = type;
		prop.visible = true;

		switch (type) {
			case property_type::prop_bool:
				prop.data = any(false);
				break;
			case property_type::prop_float:
				prop.data = any(0.0f);
				break;
			case property_type::prop_vec3:
				prop.data = any(glm::vec3(0.0f));
				break;
			case property_type::prop_enum:
			case property_type::prop_int:
				prop.data = any(int(0));
				break;
			case property_type::prop_input_file:
			case property_type::prop_output_file:
			case property_type::prop_string:
				prop.data = any(std::string(""));
				break;
		}

		assert(!prop.data.empty());

		m_props.push_back(std::move(prop));
	}

	void set_prop_visible(const std::string &prop_name, bool visible)
	{
		const Property *prop = find_property(prop_name);

		if (prop) {
			return prop->visible = visible;
		}
	}

	virtual void update_properties() {}

	int eval_int(const std::string &prop_name)
	{
		const Property *prop = find_property(prop_name);

		if (prop) {
			return any_cast<int>(prop->data);
		}

		return 0;
	}

	float eval_float(const std::string &prop_name)
	{
		const Property *prop = find_property(prop_name);

		if (prop) {
			return any_cast<float>(prop->data);
		}

		return 0.0f;
	}

	int eval_enum(const std::string &prop_name)
	{
		return eval_int(prop_name);
	}

	int eval_bool(const std::string &prop_name)
	{
		const Property *prop = find_property(prop_name);

		if (prop) {
			return any_cast<bool>(prop->data);
		}

		return false;
	}

	glm::vec3 eval_vec3(const std::string &prop_name)
	{
		const Property *prop = find_property(prop_name);

		if (prop) {
			return any_cast<glm::vec3>(prop->data);
		}

		return glm::vec3(0.0f, 0.0f, 0.0f);
	}

	std::string eval_string(const std::string &prop_name)
	{
		const Property *prop = find_property(prop_name);

		if (prop) {
			return any_cast<std::string>(prop->data);
		}

		return {};
	}

	void set_prop_min_max(const float min, const float max)
	{
		Property &prop = this->m_props.back();
		prop.min = min;
		prop.max = max;
	}

	void set_prop_enum_values(const EnumProperty &enum_prop)
	{
		Property &prop = this->m_props.back();

		assert(prop.type == property_type::prop_enum);

		prop.enum_items = enum_prop;
	}

	void set_prop_default_value_int(int value)
	{
		Property &prop = this->m_props.back();

		assert(prop.type == property_type::prop_int || prop.type == property_type::prop_enum);

		*(prop.data.as_ptr<int>()) = value;
	}

	void set_prop_default_value_float(float value)
	{
		Property &prop = this->m_props.back();

		assert(prop.type == property_type::prop_float);

		*(prop.data.as_ptr<float>()) = value;
	}

	void set_prop_default_value_bool(bool value)
	{
		Property &prop = this->m_props.back();

		assert(prop.type == property_type::prop_bool);

		*(prop.data.as_ptr<bool>()) = value;
	}

	void set_prop_default_value_string(const std::string &value)
	{
		Property &prop = this->m_props.back();

		assert(prop.type == property_type::prop_string ||
		       prop.type == property_type::prop_input_file ||
		       prop.type == property_type::prop_output_file);

		*(prop.data.as_ptr<std::string>()) = value;
	}

	void set_prop_default_value_vec3(const glm::vec3 &value)
	{
		Property &prop = this->m_props.back();

		assert(prop.type == property_type::prop_vec3);

		*(prop.data.as_ptr<glm::vec3>()) = value;
	}

	void set_prop_tooltip(std::string tooltip)
	{
		Property &prop = this->m_props.back();
		prop.tooltip = std::move(tooltip);
	}

	std::vector<Property> &props()
	{
		return m_props;
	}

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
