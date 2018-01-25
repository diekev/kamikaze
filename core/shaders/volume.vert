#version 330 core

layout(location = 0) in vec3 vertex;

uniform mat4 MVP;
uniform mat4 matrix;
uniform vec3 offset;
uniform vec3 dimension;

smooth out vec3 UV;

void main()
{
	gl_Position = MVP * matrix * vec4(vertex.xyz, 1.0);
	UV = (vertex - offset) / dimension;
}
