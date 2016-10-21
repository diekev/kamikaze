#version 330 core
  
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 vertex_color;

uniform mat4 matrix;
uniform mat4 MVP;
uniform mat3 N;
uniform bool has_vcolors;
smooth out vec3 nor;
smooth out vec3 col;

void main()
{  
	/* clipspace vertex position */
	gl_Position = MVP * matrix * vec4(vertex.xyz, 1.0);
	nor = normalize(N * normal);

	if (has_vcolors) {
		col = vertex_color;
	}
	else {
		col = vec3(1.0);
	}
}
