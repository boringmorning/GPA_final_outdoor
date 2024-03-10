#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

uniform mat4 vp_matrix;
uniform mat4 shadow_matrix;
uniform vec3 camera_position;

const float tiling = 1.0;

out VS_OUT
{
	vec4 clip_pos;
	vec4 shadow_coord;
	vec3 WV;
	vec3 WN;
	vec2 texcoord;
	vec3 toCameraVector;
} vs_out;

void main()
{
	vs_out.clip_pos = vp_matrix * vec4(position, 1.0);
	vs_out.shadow_coord = shadow_matrix * vec4(position, 1.0);
	vs_out.WV = normalize(position);
	vs_out.WN = normal;
	vs_out.texcoord = texcoord * tiling;
	vs_out.toCameraVector = normalize(camera_position - position);
	gl_Position = vp_matrix * vec4(position, 1.0);
}