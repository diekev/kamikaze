#include <glm/glm.hpp>
#include "GLSLShader.h"

const int MAX_SLICES = 512;

struct GPUVolumeShader {
	GLuint volumeVAO;
	GLuint volumeVBO;
	GLuint textureID;
	GLSLShader shader;
	glm::vec3 vTextureSlices[MAX_SLICES * 12];
};

bool loadVolumeFile(const std::string &volume_file, GLuint textureID);
void sliceVolume(GPUVolumeShader &volume, glm::vec3 viewDir);
bool init_volume_shader(GPUVolumeShader &volume);
void setup_volume_render(GPUVolumeShader &volume, glm::vec3 viewDir);
void delete_volume_shader(GPUVolumeShader &volume);
void render_volume(GPUVolumeShader &volume, glm::vec3 viewDir, glm::mat4 MVP, bool view_rotated);

