#version 330 core

layout(location = 0) out vec4 fragment_color;
smooth in vec3 fcol;

void main()
{
	fragment_color = vec4(fcol, 1.0);
}
