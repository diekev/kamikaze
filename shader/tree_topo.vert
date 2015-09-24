#version 330 core
  
layout(location = 0) in vec3 vertex;
in vec3 color;

uniform mat4 MVP;

smooth out vec3 fcol;

void main()
{  
	/* clipspace vertex position */
	gl_Position = MVP * vec4(vertex.xyz, 1.0);
	fcol = color;
}
