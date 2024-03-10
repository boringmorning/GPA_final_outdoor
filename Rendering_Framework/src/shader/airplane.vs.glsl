#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

uniform mat4 m_matrix;
uniform mat4 mvp_matrix;
uniform mat4 mv_matrix;
uniform mat4 view_matrix;
uniform mat4 shadow_matrix;
uniform vec3 view_point_light_pos;
uniform vec4 plane;

const vec4 sun_dir = vec4(0.2, 0.6, 0.5, 0.0);

out VS_OUT
{
	vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 V; // eye space view vector
	vec3 L2; // eye space point light vector
	vec2 texcoord;
	vec4 shadow_coord;
	vec3 WV;	// world space vertex
	vec3 WN;	// world space normal
} vs_out;

void main()
{
	vec4 P = mv_matrix * vec4(position, 1.0);
	vs_out.N = mat3(mv_matrix) * normal;
	vec4 view_sun_dir = view_matrix * sun_dir;
	vs_out.L = view_sun_dir.xyz;
	vs_out.V = -P.xyz;
	vs_out.L2 = view_point_light_pos - P.xyz;
	
	gl_Position = mvp_matrix * vec4(position, 1.0);
	vs_out.texcoord = texcoord;
	vs_out.shadow_coord = shadow_matrix * vec4(position, 1.0);

	vec4 wVertex = m_matrix * vec4(position, 1.0);
	gl_ClipDistance[0] = dot(wVertex, plane);
	vs_out.WV = normalize(wVertex.xyz);
	vs_out.WN = mat3(m_matrix) * normal;
}