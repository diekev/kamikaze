#version 330 core

layout(location = 0) out vec4 fragment_color;
smooth in vec3 UV;
uniform sampler3D volume;
uniform sampler3D index_volume;
uniform sampler1D lut;
uniform bool use_lut;
uniform float scale;

void main()
{
	vec3 co = texture(index_volume, UV).rgb;
	co = co + (UV - fract(UV));

	float density = texture(volume, co).r;
	vec3 color = vec3(density);

	if (use_lut) {
		color = texture(lut, density).rgb;
	}

	fragment_color = vec4(color, density * scale);
}
