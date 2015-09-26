#pragma once

class Volume;

class Scene {
	Volume *m_volume;

public:
	Scene();
	~Scene();

	void keyboardEvent(unsigned char key);

	void add_volume(Volume *volume);
	void render(const glm::vec3 &view_dir, const glm::mat4 & MVP);
};
