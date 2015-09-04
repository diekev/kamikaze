#include <glm/glm.hpp>
#include "GLSLShader.h"

const int MAX_SLICES = 512;

class VolumeShader {
	GLuint m_vao;
	GLuint m_vbo;
	GLuint m_texture_id;
	GLSLShader m_shader;
	glm::vec3 m_texture_slices[MAX_SLICES * 12];
	glm::vec3 m_vertices[8];

public:
	VolumeShader();
	~VolumeShader();

	bool init(const std::string &filename);
	bool loadVolumeFile(const std::string &filename);
	void slice(const glm::vec3 &dir);
	void setupRender(const glm::vec3 &dir);
	void render(const glm::vec3 &dir, const glm::mat4 &MVP, const bool is_rotated);
};

