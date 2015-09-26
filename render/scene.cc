
#include <glm/glm.hpp>

#include "scene.h"

#include "render/GLSLShader.h"
#include "objects/volume.h"

Scene::Scene()
    : m_volume(nullptr)
{}

Scene::~Scene()
{
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

void Scene::render(const glm::vec3 &view_dir, const glm::mat4 &MVP)
{
	if (m_volume != nullptr) {
		m_volume->render(view_dir, MVP);
	}
}
