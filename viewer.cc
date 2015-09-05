#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>

#include "viewer.h"
#include "volume.h"

Viewer::Viewer()
{
	m_state = 0;
	m_old_x = 0;
	m_old_y = 0;
	m_rX = 4.0f;
	m_rY = 50.0f;
	m_dist = -2.0f;

	m_bg = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);
	m_view_rotated = false;
	m_volume_shader = nullptr;
}

Viewer::~Viewer()
{
	delete m_volume_shader;
}

void Viewer::init(const char *filename)
{
	m_volume_shader = new VolumeShader();

	if (!m_volume_shader->init(filename)) {
		std::cerr << "Initialisation of the volume data failed!\n";
		return;
	}

	/* load the transfuer function data and generate the transfer look up table */
	loadTransferFunction();

	glClearColor(m_bg.r, m_bg.g, m_bg.b, m_bg.a);

	setViewDir();

	m_volume_shader->setupRender();
	m_volume_shader->slice(m_view_dir);
}

void Viewer::shutDown()
{
	glDeleteTextures(1, &m_tf_tex_ID);
}

void Viewer::resize(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	m_projection = glm::perspective(glm::radians(60.0f), (float)w/h, 0.1f, 1000.0f);
}

void Viewer::mouseDownEvent(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN) {
		m_old_x = x;
		m_old_y = y;
	}

	if (button == GLUT_MIDDLE_BUTTON) {
		m_state = 0;
	}
	else {
		m_state = 1;
	}

	if (s == GLUT_UP) {
		m_view_rotated = false;
	}
}

void Viewer::mouseMoveEvent(int x, int y)
{
	if (m_state == 0) {
		m_dist += (y - m_old_y) / 50.0f;
	}
	else {
		m_rX += (y - m_old_y) / 5.0f;
		m_rY += (x - m_old_x) / 5.0f;
		m_view_rotated = true;
	}

	m_old_x = x;
	m_old_y = y;

	glutPostRedisplay();
}

void Viewer::keyboardEvent(unsigned char key, int /*x*/, int /*y*/)
{
	switch (key) {
		case '-':
			m_volume_shader->changeNumSlicesBy(-1);
			break;
		case '+':
			m_volume_shader->changeNumSlicesBy(1);
			break;
	}

	m_volume_shader->slice(m_view_dir);

	glutPostRedisplay();
}

void Viewer::render()
{
	setViewDir();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 MVP = m_projection * m_model_view;

	m_volume_shader->render(m_view_dir, MVP, m_view_rotated);

	glutSwapBuffers();
}

void Viewer::setViewDir()
{
	glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, m_dist));
	glm::mat4 R = glm::rotate(T, glm::radians(m_rX), glm::vec3(1.0f, 0.0f, 0.0f));
	m_model_view = glm::rotate(R, glm::radians(m_rY), glm::vec3(0.0f, 1.0f, 0.0f));
	m_view_dir = -glm::vec3(m_model_view[0][2], m_model_view[1][2], m_model_view[2][2]);
}

void Viewer::loadTransferFunction()
{
	float pData[256][4];
	int indices[9];

	/* fill the colour values at the place where the colour shuld be after
		 * interpolation */
	for (int i = 0; i < 9; ++i) {
		auto index = i * 28;
		pData[index][0] = jet_values[i].x;
		pData[index][1] = jet_values[i].y;
		pData[index][2] = jet_values[i].z;
		pData[index][3] = jet_values[i].w;
		indices[i] = index;
	}

	/* for each adjacent pair of colours, find the difference in the rgba values
		 * and then interpolate */
	for (int j = 0; j < 9 - 1; ++j) {
		auto dDataR = (pData[indices[j + 1]][0] - pData[indices[j]][0]);
		auto dDataG = (pData[indices[j + 1]][1] - pData[indices[j]][1]);
		auto dDataB = (pData[indices[j + 1]][2] - pData[indices[j]][2]);
		auto dDataA = (pData[indices[j + 1]][3] - pData[indices[j]][3]);

		auto dIndex = indices[j + 1] - indices[j];

		auto dDatatIncR = dDataR / static_cast<float>(dIndex);
		auto dDatatIncG = dDataG / static_cast<float>(dIndex);
		auto dDatatIncB = dDataB / static_cast<float>(dIndex);
		auto dDatatIncA = dDataA / static_cast<float>(dIndex);

		for (int i = indices[j] + 1; i < indices[j + 1]; ++i) {
			pData[i][0] = (pData[i - 1][0] + dDatatIncR);
			pData[i][1] = (pData[i - 1][1] + dDatatIncG);
			pData[i][2] = (pData[i - 1][2] + dDatatIncB);
			pData[i][3] = (pData[i - 1][3] + dDatatIncA);
		}
	}

	glGenTextures(1, &m_tf_tex_ID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, m_tf_tex_ID);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, pData);
}
