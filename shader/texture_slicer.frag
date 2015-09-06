#version 330 core

layout(location = 0) out vec4 vFragColor;
smooth in vec3 vUV;
uniform sampler3D volume;
uniform sampler1D lut;
uniform bool use_lut;
uniform float scale;

void main()
{
	if (use_lut) {
		vFragColor = texture(lut, texture(volume, vUV).r);
	}
	else {
		float density = texture(volume, vUV).r;
		vFragColor = vec4(density, density, density, density * scale);
	}
}
