#include <GL/glew.h>
#include <map>
#include <string>

class GLSLShader {
	enum ShadeType {
		VERTEX_SHADER,
		FRAGMENT_SHADER,
		GEOMETRY_SHADER
	};

	GLuint m_program;
	int m_total_shaders;
	GLuint m_shaders[3];
	std::map<std::string, GLuint> m_attrib_list;
	std::map<std::string, GLuint> m_uniform_loc_list;

public:
	GLSLShader();
	~GLSLShader();

	void LoadFromString(GLenum whichShader, const std::string &source);
	void LoadFromFile(GLenum whichShader, const std::string &source);
	void CreateAndLinkProgram();
	void Use();
	void UnUse();
	void AddAttribute(const std::string &attribute);
	void AddUniform(const std::string &uniform);

	GLuint operator[](const std::string &attribute);
	GLuint operator()(const std::string &uniform);

	void DeleteShaderProgram();
};
