#version 330 core

layout(location = 0) out vec4 fragment_color;
smooth in vec3 fcol;

uniform bool for_outline;

void main()
{
	if (for_outline) {
		fragment_color = vec4(0.8, 0.4, 0.2, 1.0);
		return;
	}

	fragment_color = vec4(fcol, 1.0);
}
