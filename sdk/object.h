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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#pragma once

#include <QString>
#include <unordered_map>

#include "cube.h"

class Modifier;
class ParamCallback;
class Ray;
class ViewerContext;

enum {
	DRAW_WIRE = 0,
	DRAW_SOLID = 1,
};

enum object_flags {
	object_flags_none      = 0,
	object_supports_sculpt = (1 << 0),
};

class Object {
protected:
	std::unique_ptr<Cube> m_bbox;
	unsigned int m_draw_type;

	glm::vec3 m_dimensions, m_scale, m_inv_size, m_rotation;
	glm::vec3 m_min, m_max, m_pos;
	glm::mat4 m_matrix, m_inv_matrix;

	QString m_name;

	bool m_draw_bbox, m_need_update;

	object_flags m_flags;

	std::vector<Modifier *> m_modifiers;

	void updateMatrix();

public:
	Object();
	virtual ~Object();

	virtual bool intersect(const Ray &ray, float &min) const;
	virtual void render(ViewerContext *context, const bool for_outline) = 0;
	void setDrawType(int draw_type);

	void drawBBox(const bool b);
	bool drawBBox() const;

	Cube *bbox() const;

	glm::vec3 pos() const;
	glm::vec3 &pos();
	glm::vec3 scale() const;
	glm::vec3 &scale();
	glm::vec3 rotation() const;
	glm::vec3 &rotation();

	void flags(object_flags flags);
	object_flags flags() const;

	/* Return the object's matrix, mainly intended for rendering the active object */
	glm::mat4 matrix() const;
	glm::mat4 &matrix();

	virtual void update();

	void tagUpdate();

	QString name() const;
	void name(const QString &name);

	/* Modifiers */
	void addModifier(Modifier *modifier);
	std::vector<Modifier *> modifiers() const;

	/* UI parameters */
	void setUIParams(ParamCallback *cb);
	virtual void setCustomUIParams(ParamCallback *cb) = 0;
};

class ObjectFactory final {
public:
	typedef Object *(*factory_func)(void);

	void registerType(const std::string &name, factory_func func)
	{
		const auto iter = m_map.find(name);
		assert(iter == m_map.end());

		m_map[name] = func;
	}

	Object *operator()(const std::string &name)
	{
		const auto iter = m_map.find(name);
		assert(iter != m_map.end());

		return iter->second();
	}

	size_t numEntries() const
	{
		return m_map.size();
	}

	std::vector<std::string> keys() const
	{
		std::vector<std::string> v;

		for (const auto &entry : m_map) {
			v.push_back(entry.first);
		}

		return v;
	}

private:
	std::unordered_map<std::string, factory_func> m_map;
};

