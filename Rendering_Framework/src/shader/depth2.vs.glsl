#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 3) in vec3 offset;

out VS_OUT
{
	vec2 texcoord;
} vs_out;

uniform mat4 light_vp_matrix;
uniform bool isLeaves;

void main()
{
	if (isLeaves)
		vs_out.texcoord = texcoord;
	vec3 instanced_position = position + offset;
	gl_Position = light_vp_matrix * vec4(instanced_position, 1);
}