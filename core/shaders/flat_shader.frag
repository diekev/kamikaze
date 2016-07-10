#version 330 core

layout(location = 0) out vec4 fragment_color;

uniform vec4 color;

void main()
{
	fragment_color = color;
}
