#include <glm/glm.hpp>

#include "GLSLShader.h"

const int MAX_SLICES = 512;

class VolumeShader {
	GLuint m_vao;
	GLuint m_vbo;
	GLuint m_texture_id, m_transfer_func_id;
	GLSLShader m_shader;
	glm::vec3 m_texture_slices[MAX_SLICES * 12];

	glm::vec3 m_min, m_max;
	glm::vec3 m_size, m_inv_size;
	int m_num_slices;
	int m_axis;
	bool m_use_lut;

public:
	VolumeShader();
	~VolumeShader();

	bool init(const std::string &filename);
	bool loadVolumeFile(const std::string &filename);
	void slice(const glm::vec3 &dir);
	void sliceAxisAligned(const glm::vec3 &view_dir);
	void setupRender();
	void render(const glm::vec3 &dir, const glm::mat4 &MVP, const bool is_rotated);
	void changeNumSlicesBy(int x);

	/* Function to generate interpolated colours from the set of colours values
	 * (jet_values). This function first calculates the amount of increments for
	 * each component and the index difference. Then it linearly interpolates the
	 * adjacent values to get the interpolated result.
	 */
	void loadTransferFunction();
	void toggleUseLUT();
};
