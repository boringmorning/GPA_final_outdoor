#version 430 core

in vec3 v_vertex;
in vec3 v_normal ;
in vec3 v_uv ;

out vec3 f_viewVertex ;
out vec3 f_uv;

out VS_OUT
{
	vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 V; // eye space view vector
	vec3 L2; // eye space point light vector
	vec4 shadow_coord;
	vec3 WV;	// world space vertex
	vec3 WN;	// world space normal
} vs_out;

uniform mat4 projMat ;
uniform mat4 viewMat ;
uniform mat4 modelMat ;
uniform mat4 shadow_sbpv_matrix;
uniform vec3 view_point_light_pos;
uniform vec4 plane;

const vec4 sun_dir = vec4(0.2, 0.6, 0.5, 0.0);

// terrain
uniform mat4 vToHeightUVMat;
uniform sampler2D elevationMap;
uniform sampler2D normalMap;

vec3 getTerrainVertex(vec4 worldV){
	float mHeight = 300.0 ;
	vec4 uvForHeight = vToHeightUVMat * worldV;
	float h = texture(elevationMap, vec2(uvForHeight.x, uvForHeight.z)).r ;
	float y =  h * mHeight ;	
	return vec3(worldV.x, y, worldV.z) ;		
}

void renderTerrain(){	
	vec4 worldV = modelMat * vec4(v_vertex, 1.0) ;
	worldV.w = 1.0;
	vec3 cVertex = getTerrainVertex(worldV) ;
	gl_ClipDistance[0] = dot(vec4(cVertex, 1.0), plane);
	
	// get normal
	vec4 uvForNormal = vToHeightUVMat * worldV;
	vec4 normalTex = texture(normalMap, vec2(uvForNormal.x, uvForNormal.z)) ;
	// [0, 1] -> [-1, 1]
	normalTex = normalTex * 2.0 - 1.0 ;
		
	vec4 viewVertex = viewMat * vec4(cVertex, 1) ;
	vec4 viewNormal = viewMat * vec4(normalTex.rgb, 0) ;
	
	f_viewVertex = viewVertex.xyz ;
	
	vec4 uvForDiffuse = vToHeightUVMat * worldV;
	f_uv = vec3(uvForDiffuse.x, uvForDiffuse.z, 1.0) ;
	
	gl_Position = projMat * viewVertex ;

	vs_out.N = viewNormal.xyz;
	vec4 view_sun_dir = viewMat * sun_dir;
	vs_out.L = view_sun_dir.xyz;
	vs_out.V = -viewVertex.xyz;
	vs_out.L2 = view_point_light_pos - viewVertex.xyz;

	vs_out.shadow_coord = shadow_sbpv_matrix * vec4(cVertex, 1.0);
	vs_out.WV = normalize(worldV.xyz);
	vs_out.WN = normalTex.rgb;
}

void main(){
	renderTerrain() ;
}