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
class PrimitiveFactory;
class Ray;
class ViewerContext;

extern "C" {

/**
 * @brief new_kamikaze_prims API for plugins to register new primitive types.
 *                           There is no limit to the number of primitives to
 *                           register from a single call to this function.
 * @param factory The factory used to register the new primitives in.
 */
void new_kamikaze_prims(PrimitiveFactory *factory);

}

enum {
	DRAW_WIRE = 0,
	DRAW_SOLID = 1,
};

enum object_flags {
	object_flags_none      = 0,
	object_supports_sculpt = (1 << 0),
};

class Primitive {
protected:
	std::unique_ptr<Cube> m_bbox{};
	unsigned int m_draw_type = 0x0004;  /* GL_TRIANGLES */

	glm::vec3 m_dimensions = glm::vec3(0.0f);
	glm::vec3 m_scale = glm::vec3(1.0f);
	glm::vec3 m_inv_size = glm::vec3(0.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f);

	glm::vec3 m_min = glm::vec3(0.0f);
	glm::vec3 m_max = glm::vec3(0.0f);
	glm::vec3 m_pos = glm::vec3(0.0f);

	glm::mat4 m_matrix = glm::mat4(0.0f);
	glm::mat4 m_inv_matrix = glm::mat4(0.0f);

	QString m_name{};

	bool m_draw_bbox = false;
	bool m_need_update = true;

	int m_refcount = 0;

	object_flags m_flags = object_flags::object_flags_none;

	void updateMatrix();

public:
	Primitive() = default;
	virtual ~Primitive() = default;

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

	/* perform a deep copy of this primitive */
	virtual Primitive *copy() const = 0;

	/* UI parameters */
	void setUIParams(ParamCallback *cb);
	virtual void setCustomUIParams(ParamCallback *cb) = 0;

	/* Reference counting */
	int refcount() const;
	void incref();
	void decref();
};

class PrimitiveFactory final {
public:
	typedef Primitive *(*factory_func)(void);

	/**
	 * @brief registerType Register a new element in this factory.
	 *
	 * @param key The key associate @ func to.
	 * @param func A function pointer with signature 'Primitive *(void)'.
	 */
	void registerType(const std::string &key, factory_func func);

	/**
	 * @brief operator() Create a Primitive based on the given key.
	 *
	 * @param key The key to lookup.
	 * @return A new Primitive object corresponding to the given key.
	 */
	Primitive *operator()(const std::string &key);

	/**
	 * @brief numEntries The number of entries registered in this factory.
	 *
	 * @return The number of entries registered in this factory, 0 if empty.
	 */
	size_t numEntries() const;

	/**
	 * @brief keys Keys registered in this factory.
	 *
	 * @return A vector containing the keys registered in this factory.
	 */
	std::vector<std::string> keys() const;

	/**
	 * @brief registered Check whether or not a key has been registered in this
	 *                   factory.
	 *
	 * @param key The key to lookup.
	 * @return True if the key is found, false otherwise.
	 */
	bool registered(const std::string &key) const;

private:
	std::unordered_map<std::string, factory_func> m_map;
};
