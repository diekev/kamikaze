#version 330 core
  
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 vertex_color;

uniform mat4 MVP;
uniform mat4 matrix;
uniform bool has_vcolors;

uniform vec4 color;

smooth out vec4 col;

void main()
{  
	/* clipspace vertex position */
	gl_Position = MVP * matrix * vec4(vertex.xyz, 1.0);

	if (has_vcolors) {
		col = vec4(vertex_color, 1.0);
	}
	else {
		col = color;
	}
}
