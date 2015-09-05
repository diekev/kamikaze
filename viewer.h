#pragma once

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

const int WIDTH = 1280;
const int HEIGHT = 960;

class VolumeShader;

class Viewer {
	/* camera transform variables */
	int m_state, m_old_x, m_old_y;
	float m_rX, m_rY, m_dist;

	glm::mat4 m_model_view, m_projection;
	glm::vec3 m_view_dir;
	glm::vec4 m_bg;
	bool m_view_rotated;

	VolumeShader *m_volume_shader;

public:
	Viewer();
	~Viewer();

	void init(const char *filename);
	void shutDown();
	void resize(int w, int h);
	void mouseDownEvent(int button, int s, int x, int y);
	void mouseMoveEvent(int x, int y);
	void keyboardEvent(unsigned char key, int x, int y);
	void render();
	void setViewDir();
};
