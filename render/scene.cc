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
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include "scene.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <QKeyEvent>
#include <QListWidget>

#include "objects/levelset.h"
#include "objects/volume.h"
#include "sculpt/brush.h"
#include "smoke/smokesimulation.h"

#include "util/util_string.h"

Scene::Scene()
    : m_active_object(nullptr)
    , m_brush(new Brush(5.0f, 0.5f))
    , m_smoke_simulation(new SmokeSimulation)
    , m_mode(SCENE_MODE_OBJECT)
{}

Scene::~Scene()
{
	for (auto &object : m_objects) {
		delete object;
	}

	delete m_brush;
	delete m_smoke_simulation;
}

void Scene::keyboardEvent(int key)
{
	if (m_objects.size() == 0) {
		return;
	}

	switch (key) {
		case Qt::Key_Delete:
			auto iter = std::find(m_objects.begin(), m_objects.end(), m_active_object);
			m_objects.erase(iter);
			delete m_active_object;
			m_active_object = nullptr;
			break;
	}
}

void Scene::removeObject(Object *ob)
{
	auto iter = std::find(m_objects.begin(), m_objects.end(), ob);

	if (iter != m_objects.end()) {
		m_objects.erase(iter);
	}

	if (ob == m_active_object) {
		m_active_object = nullptr;
	}

	Q_EMIT objectChanged();
}

void Scene::addObject(Object *object)
{
	QString name = object->name();
	if (ensureUniqueName(name)) {
		object->name(name);
	}

	m_objects.push_back(object);
	m_active_object = object;

	Q_EMIT objectChanged();
}

void Scene::render(const glm::mat4 &MV, const glm::mat4 &P, const glm::vec3 &view_dir)
{
	const auto &MVP = P * MV;
	const auto &N = glm::inverseTranspose(glm::mat3(MV));

	for (auto &object : m_objects) {
		const bool active_object = (object == m_active_object);

		/* update object before drawing */
		object->update();

		if (object->type() == VOLUME || object->type() == LEVEL_SET) {
			VolumeBase *vb = static_cast<VolumeBase *>(object);

			if (object->drawBBox()) {
				vb->bbox()->render(MVP, N, view_dir, false);
			}

			if (object->drawTreeTopology()) {
				vb->topology()->render(MVP);
			}
		}

		object->render(MVP, N, view_dir, false);

		if (active_object) {
			glStencilFunc(GL_NOTEQUAL, 1, 0xff);
			glStencilMask(0x00);
			glDisable(GL_DEPTH_TEST);

			/* scale up the object a bit */
			glm::mat4 obmat = object->matrix();
			object->matrix() = glm::scale(obmat, glm::vec3(1.01f));

			object->render(MVP, N, view_dir, true);

			object->matrix() = obmat;

			/* restore */
			glStencilFunc(GL_ALWAYS, 1, 0xff);
			glStencilMask(0xff);
			glEnable(GL_DEPTH_TEST);
		}
	}
}

void Scene::intersect(const Ray &ray)
{
	LevelSet *ls = static_cast<LevelSet *>(m_active_object);
	if (ls->intersectLS(ray, m_brush)) {
		// TODO: separate intersection from sculpting.
	}
}

/* Select the object which is closest to pos. */
void Scene::selectObject(const glm::vec3 &pos)
{
	float min = 1000.0f;
	int selected_object = -1, index = 0;

	for (auto &object : m_objects) {
		float dist = glm::distance(object->pos(), pos);
		if (/*dist < 1.0f &&*/ dist < min) {
			selected_object = index;
			min = dist;
		}

		++index;
	}

	if (selected_object != -1 && m_active_object != m_objects[selected_object]) {
		m_active_object = m_objects[selected_object];
		Q_EMIT objectChanged();
	}
}

int Scene::mode() const
{
	return m_mode;
}

void Scene::setMode(int mode)
{
	m_mode = mode;
	LevelSet *ls = static_cast<LevelSet *>(m_active_object);
	ls->swapGrids(mode == SCENE_MODE_SCULPT);
}

Object *Scene::currentObject()
{
	if (!m_objects.empty()) {
		return m_active_object;
	}

	return nullptr;
}

void Scene::moveObject(double value, int axis)
{
	m_active_object->pos()[axis] = value;
}

void Scene::scaleObject(double value, int axis)
{
	m_active_object->scale()[axis] = value;
}

void Scene::rotateObject(double value, int axis)
{
	m_active_object->rotation()[axis] = value;
}

void Scene::setVoxelSize(double value)
{
	if (m_active_object->type() == VOLUME || m_active_object->type() == LEVEL_SET) {
		VolumeBase *vb = static_cast<VolumeBase *>(m_active_object);
		vb->setVoxelSize(value);
	}
}

void Scene::setObjectName(const QString &name)
{
	/* Need to make a copy of the string, since the slot signature has to match
     * the signal signature (const QString &) */
	QString copy(name);

	bool name_changed = ensureUniqueName(copy);

	if (name_changed) {
		m_active_object->name(copy);
		Q_EMIT objectChanged(); // XXX - hack to update the tab and outliner
	}
	else {
		m_active_object->name(name);
	}
}

void Scene::setBrushMode(int mode)
{
	m_brush->mode(mode);
}

void Scene::setBrushRadius(double value)
{
	m_brush->radius(value);
}

void Scene::setBrushStrength(double value)
{
	m_brush->strength(value);
}

void Scene::setBrushTool(int tool)
{
	m_brush->tool(tool);
}

void Scene::objectNameList(QListWidget *widget) const
{
	widget->clear();

	for (auto &object : m_objects) {
		widget->addItem(object->name());
	}
}

bool Scene::ensureUniqueName(QString &name) const
{
	return ensure_unique_name(name, [&](const QString &str)
		{
			for (const auto &object : m_objects) {
				if (object->name() == str) {
					return false;
				}
			}

			return true;
		});
}

void Scene::setCurrentObject(QListWidgetItem *item)
{
	for (auto &object : m_objects) {
		if (object->name() == item->text()) {
			m_active_object = object;
			break;
		}
	}

	Q_EMIT objectChanged();
}

void Scene::setSimulationDt(double value)
{
	m_smoke_simulation->timeStep(value);
}

void Scene::setSimulationCache(const QString &path)
{
	m_smoke_simulation->cachePath(path.toStdString());
}

void Scene::setSimulationAdvection(int index)
{
	m_smoke_simulation->advectionScheme(index);
}

void Scene::setVolumeSlices(int slices)
{
	Volume *volume = static_cast<Volume *>(m_active_object);
	volume->numSlices(slices);
}

void Scene::setVolumeLUT(bool b)
{
	Volume *volume = static_cast<Volume *>(m_active_object);
	volume->useLUT(b);
}
