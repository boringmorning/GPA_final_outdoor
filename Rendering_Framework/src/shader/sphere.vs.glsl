#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 vp_matrix;
uniform vec4 plane;

out VS_OUT
{
	vec3 WV;	// world space vertex
	vec3 WN;	// world space normal
} vs_out;

void main()
{
	gl_Position = vp_matrix * vec4(position, 1.0);
	gl_ClipDistance[0] = dot(vec4(position, 1.0), plane);
	vs_out.WV = normalize(position);
	vs_out.WN = normalize(normal);
}