#version 330 core

layout(location = 0) in vec3 vertex;
uniform mat4 MVP;
uniform vec3 offset;
uniform vec3 inv_size;
smooth out vec3 UV;

void main()
{
	gl_Position = MVP * vec4(vertex.xyz, 1.0);
	UV = (vertex - offset) * inv_size;
}
