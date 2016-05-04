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

class Primitive {
protected:
	std::unique_ptr<Cube> m_bbox{};
	unsigned int m_draw_type = 0x0004;  /* GL_TRIANGLES */

	glm::vec3 m_dimensions = glm::vec3(1.0f);
	glm::vec3 m_scale = glm::vec3(1.0f);
	glm::vec3 m_inv_size = glm::vec3(0.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f);

	glm::vec3 m_min = glm::vec3(0.0f);
	glm::vec3 m_max = glm::vec3(0.0f);
	glm::vec3 m_pos = glm::vec3(0.0f);

	glm::mat4 m_matrix = glm::mat4(1.0f);
	glm::mat4 m_inv_matrix = glm::mat4(1.0f);

	QString m_name{};

	bool m_draw_bbox = false;
	bool m_need_update = true;
	bool m_need_data_update = true;

	int m_refcount = 0;

public:
	Primitive() = default;
	virtual ~Primitive() = default;

	/**
	 * @brief intersect Intersect a ray against this primitive AABB.
	 * @param ray       The ray to check intersection with.
	 * @param min       The minimum distance from the ray origin.
	 * @return          True if the ray intersects this primitive.
	 */
	virtual bool intersect(const Ray &ray, float &min) const;

	/**
	 * @brief prepareRenderData Prepare the data required for drawing this
	 *                          primitive inside an OpenGL context.
	 */
	virtual void prepareRenderData() = 0;

	/**
	 * @brief render      Draw this primitive inside of an OpenGL context.
	 * @param context     The OpenGL context in which the primitive is drawn.
	 * @param for_outline Flag to indicate that the primitive is selected. If so,
	 *                    its outline will be drawn highlighted.
	 */
	virtual void render(ViewerContext *context, const bool for_outline) = 0;

	/**
	 * @brief computeBBox Compute the bounding box of this primitive.
	 * @param min         The minimum position of this bounding box.
	 * @param max         The maximum position of this bounding box.
	 */
	virtual void computeBBox(glm::vec3 &min, glm::vec3 &max) = 0;

	/* todo remove these 3 */
	void drawBBox(const bool b);
	bool drawBBox() const;
	Cube *bbox() const;

	/* todo: remove these? */
	glm::vec3 pos() const;
	glm::vec3 &pos();
	glm::vec3 scale() const;
	glm::vec3 &scale();
	glm::vec3 rotation() const;
	glm::vec3 &rotation();

	/* Return the object's matrix, mainly intended for rendering the active object */
	const glm::mat4 &matrix() const;
	glm::mat4 &matrix();
	void matrix(const glm::mat4 &m);

	virtual void update();

	/**
	 * @brief tagUpdate Tag this primitive for updates.
	 */
	void tagUpdate();

	QString name() const;
	void name(const QString &name);

	/**
	 * @brief copy Perform a deep copy of this primitive.
	 * @return     A new primitive with the same data as this primitive.
	 */
	virtual Primitive *copy() const = 0;

	/* UI parameters */
	void setUIParams(ParamCallback *cb);
	virtual void setCustomUIParams(ParamCallback *cb) = 0;

	/* Reference counting, NOT to be used from plugins. They are used to
	 * indicate that primitives are ready to be deleted.
	 *
	 * TODO: Replace with std::shared_ptr? */
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
