
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "scene.h"

#include "render/GPUShader.h"
#include "objects/levelset.h"
#include "objects/volume.h"

Scene::Scene()
    : m_volume(nullptr)
    , m_level_set(nullptr)
{}

Scene::~Scene()
{
	delete m_level_set;
	delete m_volume;
}

void Scene::keyboardEvent(unsigned char key)
{
	bool need_slicing = false;

	switch (key) {
		case '-':
			m_volume->changeNumSlicesBy(-1);
			need_slicing = true;
			break;
		case '+':
			m_volume->changeNumSlicesBy(1);
			need_slicing = true;
			break;
		case 'l':
			m_volume->toggleUseLUT();
			break;
		case 'b':
			m_volume->toggleBBoxDrawing();
			break;
		case 't':
			m_volume->toggleTopologyDrawing();
			break;
	}

	if (need_slicing) {
		//m_volume->slice(m_camera->viewDir());
	}
}

void Scene::add_volume(Volume *volume)
{
	m_volume = volume;
}

void Scene::add_level_set(LevelSet *level_set)
{
	m_level_set = level_set;
}

void Scene::render(const glm::vec3 &view_dir, const glm::mat4 &MV, const glm::mat4 &P)
{
	auto MVP = P * MV;

	if (m_volume != nullptr) {
		m_volume->render(view_dir, MVP);
	}

	if (m_level_set != nullptr) {
		m_level_set->render(MVP, glm::inverseTranspose(glm::mat3(MV)));
	}
}
