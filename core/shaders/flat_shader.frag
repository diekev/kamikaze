#version 330 core

layout(location = 0) out vec4 fragment_color;

smooth in vec4 col;

void main()
{
	fragment_color = col;
}
