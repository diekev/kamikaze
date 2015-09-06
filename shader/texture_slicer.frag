#version 330 core

layout(location = 0) out vec4 vFragColor;
smooth in vec3 vUV;
uniform sampler3D volume;
uniform sampler1D lut;
uniform bool use_lut;
uniform float scale;

void main()
{
	float density = texture(volume, vUV).r;
	vec3 color = vec3(density);

	if (use_lut) {
		color = texture(lut, density).rgb;
	}

	vFragColor = vec4(color, density * scale);
}
