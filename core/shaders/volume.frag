#version 330 core

layout(location = 0) out vec4 fragment_color;
smooth in vec3 UV;
uniform sampler3D volume;
uniform float scale;

void main()
{
	float density = texture(volume, UV).r;

	fragment_color = vec4(UV, density);
}
