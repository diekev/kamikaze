#include <glm/glm.hpp>
#include <vector>

const int MAX_SLICES = 512;

class VolumeShader {
	GLuint m_vao, m_bbox_vao;
	GLuint m_vbo, m_bbox_verts_vbo, m_bbox_index_vbo;
	GLuint m_texture_id, m_transfer_func_id;
	GLSLShader m_shader, m_bbox_shader;
	std::vector<glm::vec3> m_texture_slices;

	glm::vec3 m_min, m_max;
	glm::vec3 m_size, m_inv_size;
	int m_num_slices;
	int m_axis;
	float m_scale; // scale of the values contained in the grid (1 / (max - min))
	bool m_use_lut, m_draw_bbox;

	bool loadVolumeFile(const std::string &filename, std::ostream &os);
	void loadBBoxShader();
	void loadTransferFunction();
	void loadVolumeShader();

public:
	VolumeShader();
	~VolumeShader();

	bool init(const std::string &filename, std::ostream &os);

	void slice(const glm::vec3 &view_dir);
	void render(const glm::vec3 &dir, const glm::mat4 &MVP, const bool is_rotated);
	void changeNumSlicesBy(int x);

	void toggleUseLUT();
	void toggleBBoxDrawing();
};
