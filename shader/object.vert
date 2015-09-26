#version 330 core
  
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;

uniform mat4 MVP;
uniform mat3 N;
smooth out vec3 nor;

void main()
{  
	/* clipspace vertex position */
	gl_Position = MVP * vec4(vertex.xyz, 1.0);
	nor = normalize(N * normal);
}
