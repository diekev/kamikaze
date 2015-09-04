#include <fstream>
#include <iostream>
#include <sstream>

#include "GLSLShader.h"

GLSLShader::GLSLShader()
{
	m_total_shaders = 0;
	m_shaders[VERTEX_SHADER] = 0;
	m_shaders[FRAGMENT_SHADER] = 0;
	m_shaders[GEOMETRY_SHADER] = 0;
	m_attrib_list.clear();
	m_uniform_loc_list.clear();
}

GLSLShader::~GLSLShader()
{
	m_attrib_list.clear();
	m_uniform_loc_list.clear();
}

void GLSLShader::LoadFromString(GLenum whichShader, const std::string &source)
{
	GLuint shader = glCreateShader(whichShader);
	const char *ptmp = source.c_str();
	glShaderSource(shader, 1, &ptmp, nullptr);

	/* Check whether the shadr loads fine */
	GLint status;
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *infoLog = new GLchar[infoLogLength];
		glGetShaderInfoLog(shader, infoLogLength, nullptr, infoLog);
		std::cerr << "Compile log: " << infoLog << std::endl;
		delete [] infoLog;
	}

	m_shaders[m_total_shaders++] = shader;
}

void GLSLShader::LoadFromFile(GLenum whichShader, const std::string &source)
{
	std::ifstream fp(source.c_str());

	if (fp) {
		std::stringstream shader_data;
		shader_data << fp.rdbuf();

		fp.close();

		const std::string &shader = shader_data.str();
		LoadFromString(whichShader, shader);
	}
	else {
		std::cerr << "Error loading shader: " << source << std::endl;
	}
}

void GLSLShader::CreateAndLinkProgram()
{
	m_program = glCreateProgram();

	if (m_shaders[VERTEX_SHADER] != 0) {
		glAttachShader(m_program, m_shaders[VERTEX_SHADER]);
	}
	if (m_shaders[FRAGMENT_SHADER] != 0) {
		glAttachShader(m_program, m_shaders[FRAGMENT_SHADER]);
	}
	if (m_shaders[GEOMETRY_SHADER] != 0) {
		glAttachShader(m_program, m_shaders[GEOMETRY_SHADER]);
	}

	/* Link and check whether the program links fine */
	GLint status;
	glLinkProgram(m_program);
	glGetProgramiv(m_program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *infoLog = new GLchar[infoLogLength];
		glGetProgramInfoLog(m_program, infoLogLength, nullptr, infoLog);
		std::cerr << "Link log: " << infoLog << std::endl;
		delete [] infoLog;
	}

	glDeleteShader(m_shaders[VERTEX_SHADER]);
	glDeleteShader(m_shaders[FRAGMENT_SHADER]);
	glDeleteShader(m_shaders[GEOMETRY_SHADER]);
}

void GLSLShader::Use()
{
	glUseProgram(m_program);
}

void GLSLShader::UnUse()
{
	glUseProgram(0);
}

void GLSLShader::AddAttribute(const std::string &attribute)
{
	m_attrib_list[attribute] = glGetAttribLocation(m_program, attribute.c_str());
}

void GLSLShader::AddUniform(const std::string &uniform)
{
	m_uniform_loc_list[uniform] = glGetUniformLocation(m_program, uniform.c_str());
}

GLuint GLSLShader::operator[](const std::string &attribute)
{
	return m_attrib_list[attribute];
}

GLuint GLSLShader::operator()(const std::string &uniform)
{
	return m_uniform_loc_list[uniform];
}

void GLSLShader::DeleteShaderProgram()
{
	glDeleteProgram(m_program);
}

