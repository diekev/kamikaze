#pragma once

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

const int WIDTH = 1280;
const int HEIGHT = 960;

class VolumeShader;

/* transfer function (lookup table) colour values */
const glm::vec4 jet_values[9] = {
	glm::vec4(0.0f, 0.0f, 0.5f, 0.0f),
	glm::vec4(0.0f, 0.0f, 1.0f, 0.1f),
	glm::vec4(0.0f, 0.5f, 1.0f, 0.3f),
	glm::vec4(0.0f, 1.0f, 1.0f, 0.3f),
	glm::vec4(0.5f, 1.0f, 0.5f, 0.75f),
	glm::vec4(1.0f, 1.0f, 0.0f, 0.8f),
	glm::vec4(1.0f, 0.5f, 0.0f, 0.6f),
	glm::vec4(1.0f, 0.0f, 0.0f, 0.5f),
	glm::vec4(0.5f, 0.0f, 0.0f, 0.0f),
};

class Viewer {
	/* camera transform variables */
	int m_state, m_old_x, m_old_y;
	float m_rX, m_rY, m_dist;

	glm::mat4 m_model_view, m_projection;
	glm::vec3 m_view_dir;
	glm::vec4 m_bg;
	bool m_view_rotated;

	GLuint m_tf_tex_ID;

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

	/* Function to generate interpolated colours from the set of colours values
	 * (jet_values). This function first calculates the amount of increments for
	 * each component and the index difference. Then it linearly interpolates the
	 * adjacent values to get the interpolated result.
	 */
	void loadTransferFunction();
	void setViewDir();
};
