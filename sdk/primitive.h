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

#include <memory>
#include <unordered_map>

#include "cube.h"
#include "factory.h"

class Modifier;
class ParamCallback;
class Ray;
class ViewerContext;

/* ********************************************** */

class Primitive {
protected:
	std::unique_ptr<Cube> m_bbox{};

	glm::vec3 m_dimensions = glm::vec3(1.0f);
	glm::vec3 m_scale = glm::vec3(1.0f);
	glm::vec3 m_inv_size = glm::vec3(0.0f);
	glm::vec3 m_rotation = glm::vec3(0.0f);

	glm::vec3 m_min = glm::vec3(0.0f);
	glm::vec3 m_max = glm::vec3(0.0f);
	glm::vec3 m_pos = glm::vec3(0.0f);

	glm::mat4 m_matrix = glm::mat4(1.0f);
	glm::mat4 m_inv_matrix = glm::mat4(1.0f);

	std::string m_name{};

	bool m_draw_bbox = false;
	bool m_need_update = true;
	bool m_need_data_update = true;

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
	virtual void render(const ViewerContext &context) = 0;

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

	std::string name() const;
	void name(const std::string &name);

	/**
	 * @brief copy Perform a deep copy of this primitive.
	 * @return     A new primitive with the same data as this primitive.
	 */
	virtual Primitive *copy() const = 0;

	/**
	 * @brief typeID The unique ID that is shared between primitives instanced
	 *               from a type derived from this class.
	 *
	 * @return The unique ID that is assigned to the primitive class derived
	 *         from this base class upon registration inside of a primitive
	 *         factory.
	 *
	 * A typical implementation could be:
	 * @code
	 * class MyPrimitive : public Primitive {
	 * public:
	 *     static size_t id;
	 *
	 *     size_t typeID() const override
	 *     {
	 *         return MyPrimitive::id;
	 *     }
	 * };
	 *
	 * // Make sure static is initialized
	 * size_t MyPrimitive::id = -1;
	 *
	 * void new_kamikaze_prims(PrimitiveFactory *factory)
	 * {
	 *     MyPrimitive::id = REGISTER_PRIMITIVE("MyPrimitive", MyPrimitive);
	 * }
	 * @endcode
	 */
	virtual size_t typeID() const = 0;
};

/* ********************************************** */

using PrimitiveFactory = Factory<Primitive>;

/**
 * Macro to help registering primitives.
 */
#define REGISTER_PRIMITIVE(name, type) \
	REGISTER_TYPE(factory, name, Primitive, type)

extern "C" {

/**
 * @brief new_kamikaze_prims API for plugins to register new primitive types.
 *                           There is no limit to the number of primitives to
 *                           register from a single call to this function.
 * @param factory The factory used to register the new primitives in.
 */
void new_kamikaze_prims(PrimitiveFactory *factory);

}

/* ********************************************** */

class PrimitiveCollection {
	std::vector<Primitive *> m_collection = {};
	PrimitiveFactory *m_factory = nullptr;
	int m_ref = 0;

	friend class primitive_iterator;
	PrimitiveCollection() = default;

public:
	explicit PrimitiveCollection(PrimitiveFactory *factory);

	~PrimitiveCollection();

	/**
	 * @brief build Build a primitive in this collection.
	 * @param key   The key of the primitive inside the PrimitiveFactory.
	 * @return      A pointer to the newly created primitive.
	 */
	Primitive *build(const std::string &key);

	/**
	 * @brief add  Add a primitve to this collection.
	 * @param prim The primitive to add.
	 */
	void add(Primitive *prim);

	/**
	 * @brief clear Remove all the primitives inside this collection, but does
	 * not delete them.
	 */
	void clear();

	/**
	 * @brief free_all Remove and delete all the primitives inside this collection.
	 */
	void free_all();

	/**
	 * @brief copy Copy this collection and the primitive that it holds.
	 * @return  A copy of this collection containing copies of the primitive
	 *          that this collection holds.
	 */
	PrimitiveCollection *copy() const;

	/**
	 * @brief primitives The primitives contained in this collection.
	 * @return A vector containing the primitive contained in this collection.
	 */
	const std::vector<Primitive *> &primitives() const;

	/**
	 * @brief destroy Destroy the given primitive from the collection. No-op if
	 *                the primitive is not found in the collection.
	 * @param prim The primitive to destroy.
	 */
	void destroy(Primitive *prim);

	/**
	 * @brief destroy Destroy all the primitives in the given vector from the
	 *                collection. No-op if (some of) the primitives are not
	 *                found in the collection.
	 * @param prims The list of primitives to destroy.
	 */
	void destroy(const std::vector<Primitive *> &prims);

	/**
	 * @brief factory
	 * @return Return a pointer to the factory used in this collection.
	 * @todo Not nice, need a way to create valid temporary collections.
	 */
	PrimitiveFactory *factory() const;

	/* Reference counting, NOT to be used from plugins. They are used to
	 * indicate that primitives are ready to be deleted.
	 *
	 * TODO: Replace with std::shared_ptr? */
	int refcount() const;

	void incref();

	void decref();
};

/* ********************************************** */

/**
 * @brief This class is used to gather and release the collections created
 *        inside of an object's node graph.
 */
class PrimitiveCache {
	std::vector<PrimitiveCollection *> m_collections;

public:
	void add(PrimitiveCollection *collection);
	void clear();
};

/* ********************************************** */

/**
 * @brief A C++ standard compliant iterator used to traverse the primitives
 *        contained in a PrimitiveCollection. Only the primitives whose type ID
 *        match the type given in the construtor are returned by the iterator.
 */
class primitive_iterator {
	std::vector<Primitive *>::const_iterator m_iter{nullptr}, m_end{nullptr};
	size_t m_type = 0;
	PrimitiveCollection collection{};

public:
	using value_type = Primitive*;
	using difference_type = std::ptrdiff_t;
	using pointer = const value_type *;
	using reference = const value_type &;
	using iterator_category = std::input_iterator_tag;

	/**
	 * @brief primitive_iterator Construct the end iterator.
	 */
	primitive_iterator();

	/**
	 * @brief primitive_iterator Construct the begin iterator.
	 * @param collection The PrimitiveCollection to traverse.
	 */
	primitive_iterator(const PrimitiveCollection *collection);

	/**
	 * @brief primitive_iterator Construct the begin iterator.
	 * @param collection The PrimitiveCollection to traverse.
	 * @param type       The type ID of the primitive to consider for iteration.
	 */
	primitive_iterator(const PrimitiveCollection *collection, int type);

	primitive_iterator(const primitive_iterator &other);
	primitive_iterator(primitive_iterator &&other);

	~primitive_iterator() noexcept = default;

	primitive_iterator &operator=(const primitive_iterator &other) = default;
	primitive_iterator &operator=(primitive_iterator &&other) = default;

	primitive_iterator &operator++();

	const reference operator*() const;
	pointer operator->() const;

	value_type get() const;
};

bool operator==(const primitive_iterator &ita, const primitive_iterator &itb) noexcept;
bool operator!=(const primitive_iterator &ita, const primitive_iterator &itb) noexcept;

primitive_iterator begin(const primitive_iterator &iter);
primitive_iterator end(const primitive_iterator &);
