#version 330 core
  
layout(location = 0) in vec3 vertex;

uniform mat4 MVP;
uniform mat4 matrix;

void main()
{  
	/* clipspace vertex position */
	gl_Position = MVP * matrix * vec4(vertex.xyz, 1.0);
}
