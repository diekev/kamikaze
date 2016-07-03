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

#include "primitive.h"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "ui/paramfactory.h"  /* XXX - bad level call */
#include "util/util_render.h"  /* XXX - bad level call */

bool Primitive::intersect(const Ray &ray, float &min) const
{
	const auto inv_dir = 1.0f / ray.dir;
	const auto t_min = (m_min - ray.pos) * inv_dir;
	const auto t_max = (m_max - ray.pos) * inv_dir;
	const auto t1 = glm::min(t_min, t_max);
	const auto t2 = glm::max(t_min, t_max);
	const auto t_near = glm::max(t1.x, glm::max(t1.y, t1.z));
	const auto t_far = glm::min(t2.x, glm::min(t2.y, t2.z));

	if (t_near < t_far && t_near < min) {
		min = t_near;
		return true;
	}

	return false;
}

void Primitive::drawBBox(const bool b)
{
	m_draw_bbox = b;
}

bool Primitive::drawBBox() const
{
	return m_draw_bbox;
}

Cube *Primitive::bbox() const
{
	return m_bbox.get();
}

glm::vec3 Primitive::pos() const
{
	return m_pos;
}

glm::vec3 &Primitive::pos()
{
	m_need_update = true;
	return m_pos;
}

glm::vec3 Primitive::scale() const
{
	return m_scale;
}

glm::vec3 &Primitive::scale()
{
	m_need_update = true;
	return m_scale;
}

glm::vec3 Primitive::rotation() const
{
	return m_rotation;
}

glm::vec3 &Primitive::rotation()
{
	m_need_update = true;
	return m_rotation;
}

const glm::mat4 &Primitive::matrix() const
{
	return m_matrix;
}

glm::mat4 &Primitive::matrix()
{
	return m_matrix;
}

void Primitive::matrix(const glm::mat4 &m)
{
	m_matrix = m;
	m_inv_matrix = glm::inverse(m);
}

void Primitive::update()
{
	if (m_need_update) {
		m_bbox.reset(new Cube(m_min, m_max));
		m_need_update = false;
	}
}

void Primitive::tagUpdate()
{
	m_need_update = true;
	m_need_data_update = true;
}

QString Primitive::name() const
{
	return QString::fromStdString(m_name);
}

void Primitive::name(const QString &name)
{
	m_name = name.toStdString();
}

void Primitive::setUIParams(ParamCallback *cb)
{
	string_param(cb, "Name", &m_name, "");

	bool_param(cb, "Draw BoundingBox", &m_draw_bbox, false);

	xyz_param(cb, "Position", &m_pos[0]);
	xyz_param(cb, "Scale", &m_scale[0]);
	xyz_param(cb, "Rotation", &m_rotation[0]);
}

/* ********************************************** */

void PrimitiveCache::add(PrimitiveCollection *collection)
{
	if (collection->refcount() > 0) {
		return;
	}

	collection->incref();

	m_collections.push_back(collection);
}

void PrimitiveCache::clear()
{
	for (auto &primitive : m_collections) {
		delete primitive;
	}

	m_collections.clear();
}

/* ********************************************** */

size_t PrimitiveFactory::registerType(const std::string &name, PrimitiveFactory::factory_func func)
{
	const auto iter = m_map.find(name);
	assert(iter == m_map.end());

	m_map[name] = func;
	return m_map.size();
}

Primitive *PrimitiveFactory::operator()(const std::string &name)
{
	const auto iter = m_map.find(name);
	assert(iter != m_map.end());

	return iter->second();
}

size_t PrimitiveFactory::numEntries() const
{
	return m_map.size();
}

std::vector<std::string> PrimitiveFactory::keys() const
{
	std::vector<std::string> v;

	for (const auto &entry : m_map) {
		v.push_back(entry.first);
	}

	return v;
}

bool PrimitiveFactory::registered(const std::string &key) const
{
	return (m_map.find(key) != m_map.end());
}

/* ********************************************** */

template <typename OpType>
bool ensure_unique_name(QString &name, const OpType &op)
{
	if (op(name)) {
		return false;
	}

	QString temp = name + ".0000";
	const auto temp_size = temp.size();
	int number = 0;

	do {
		++number;

		if (number < 10) {
			temp[temp_size - 1] = number;
		}
		else if (number < 100) {
			temp[temp_size - 1] = number % 10;
			temp[temp_size - 2] = number / 10;
		}
		else if (number < 1000) {
			temp[temp_size - 1] = number % 10;
			temp[temp_size - 2] = (number % 100) / 10;
			temp[temp_size - 3] = number / 100;
		}
		else {
			temp[temp_size - 1] = number % 10;
			temp[temp_size - 2] = (number % 100) / 10;
			temp[temp_size - 3] = (number % 1000) / 100;
			temp[temp_size - 4] = number / 1000;
		}
	} while (!op(temp));

	name = temp;
	return true;
}

/* ********************************************** */

PrimitiveCollection::PrimitiveCollection(PrimitiveFactory *factory)
    : PrimitiveCollection()
{
	m_factory = factory;
}

PrimitiveCollection::~PrimitiveCollection()
{
	clear();
}

Primitive *PrimitiveCollection::build(const std::string &key)
{
	assert(m_factory->registered(key));
	auto prim = (*m_factory)(key);

	this->add(prim);

	return prim;
}

void PrimitiveCollection::add(Primitive *prim)
{
	m_collection.push_back(prim);

	auto name = prim->name();

	bool changed = ensure_unique_name(name, [&](const QString &str)
	{
		for (const auto &prim : m_collection) {
			if (prim->name() == str) {
				return false;
			}
		}

		return true;
	});

	if (changed) {
		prim->name(name);
	}
}

void PrimitiveCollection::clear()
{
	for (auto &prim : m_collection) {
		delete prim;
	}

	m_collection.clear();
}

PrimitiveCollection *PrimitiveCollection::copy() const
{
	auto collection = new PrimitiveCollection(this->m_factory);

	for (const auto &prim : this->m_collection) {
		collection->add(prim->copy());
	}

	return collection;
}

const std::vector<Primitive *> &PrimitiveCollection::primitives() const
{
	return m_collection;
}

int PrimitiveCollection::refcount() const
{
	return m_ref;
}

void PrimitiveCollection::incref()
{
	++m_ref;
}

void PrimitiveCollection::decref()
{
	--m_ref;
}

/* ********************************************** */

primitive_iterator::primitive_iterator()
{
	collection.add(nullptr);
	m_iter = collection.primitives().begin();
	m_end = collection.primitives().end();
}

primitive_iterator::primitive_iterator(const PrimitiveCollection *collection, int type)
    : m_type(type)
{
	m_iter = collection->primitives().begin();
	m_end = collection->primitives().end();
}

primitive_iterator::primitive_iterator(const primitive_iterator &rhs)
    : m_iter(rhs.m_iter)
    , m_end(rhs.m_end)
    , m_type(rhs.m_type)
{}

primitive_iterator &primitive_iterator::operator++()
{
	do {
		++m_iter;
	}
	while (m_iter != m_end && (*m_iter)->typeID() != m_type);

	return *this;
}

const primitive_iterator::reference primitive_iterator::operator*() const
{
	return *m_iter;
}

const primitive_iterator::pointer primitive_iterator::operator->() const
{
	return &(*m_iter);
}

primitive_iterator::value_type primitive_iterator::get() const
{
	return (m_iter != m_end) ? *m_iter : nullptr;
}

bool operator==(const primitive_iterator &ita, const primitive_iterator &itb) noexcept
{
	Primitive *a = ita.get();
	Primitive *b = itb.get();
	return a == b;
}

bool operator!=(const primitive_iterator &ita, const primitive_iterator &itb) noexcept
{
	return !(ita == itb);
}

primitive_iterator begin(const primitive_iterator &iter)
{
	return iter;
}

primitive_iterator end(const primitive_iterator &)
{
	return primitive_iterator();
}
