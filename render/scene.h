#pragma once

class LevelSet;
class Volume;

class Scene {
	Volume *m_volume;
	LevelSet *m_level_set;

public:
	Scene();
	~Scene();

	void keyboardEvent(unsigned char key);

	void add_volume(Volume *volume);
	void add_level_set(LevelSet *level_set);
	void render(const glm::vec3 &view_dir, const glm::mat4 &MV, const glm::mat4 &P);
};
