#version 330 core

layout(location = 0) out vec4 vFragColor;
smooth in vec3 vUV;
uniform sampler3D volume;
uniform sampler1D lut;
uniform bool use_lut;

void main()
{
	if (use_lut) {
		vFragColor = texture(lut, texture(volume, vUV).r);
	}
	else {
		vFragColor = texture(volume, vUV).rrrr;
	}
}
