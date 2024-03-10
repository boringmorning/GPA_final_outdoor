#version 430 core

in vec3 f_viewVertex ;
in vec3 f_uv ;

in VS_OUT
{
	vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 V; // eye space view vector
	vec3 L2; // eye space point light vector
	vec4 shadow_coord;
	vec3 WV;	// world space vertex
	vec3 WN;	// world space normal
} fs_in;

layout (location = 0) out vec4 fragColor0 ;

uniform sampler2D texture0 ;
uniform sampler2D shadow_tex;
uniform int mode;
uniform int enable_cel_shading;

const vec3 ambient_light = vec3(0.1);
const vec3 diffuse_light = vec3(0.8);
const vec3 specular_light = vec3(0.1);
const float shininess = 1.0;
// point light
const vec3 point_color = vec3(1.0, 1.0, 0.5);
const vec3 point_diffuse_light = point_color * vec3(0.85);
const vec3 point_specular_light = point_color * vec3(0.15);
// attenuation variables
const float c1 = 1.0;
const float c2 = 0.005;
const float c3 = 0.001;

vec4 withFog(vec4 color){
	const vec4 FOG_COLOR = vec4(1.0, 1.0, 1.0, 1) ;
	const float MAX_DIST = 1000.0 ;
	const float MIN_DIST = 600.0 ;
	
	float dis = length(f_viewVertex) ;
	float fogFactor = (MAX_DIST - dis) / (MAX_DIST - MIN_DIST) ;
	fogFactor = clamp(fogFactor, 0.0, 1.0) ;
	fogFactor = fogFactor * fogFactor ;
	
	vec4 colorWithFog = mix(FOG_COLOR, color, fogFactor) ;
	return colorWithFog ;
}

void renderTerrain(){
	// get terrain color
	vec4 terrainColor = texture(texture0, f_uv.rg) ;
	
	// blin-phong
	vec3 N = normalize(fs_in.N);
	vec3 L = normalize(fs_in.L);
	vec3 V = normalize(fs_in.V);
	vec3 H = normalize(L + V);
	vec3 L2 = normalize(fs_in.L2);
	vec3 H2 = normalize(L2 + V);

	float dist = length(fs_in.L2);
	float attenuation_factor = 1 / (c1 + c2 * dist + c3 * pow(dist, 2));

	float diff = max(dot(N, L), 0.0);
	vec3 ambient = ambient_light;
	vec3 diffuse = ( diff * diffuse_light + max(dot(N, L2), 0.0) * attenuation_factor * point_diffuse_light);
	//vec3 specular = ( pow(max(dot(N, H), 0.0), shininess) * specular_light + pow(max(dot(N, H2), 0.0), shininess) * attenuation_factor * point_specular_light );
	vec3 specular = vec3(0.0, 0.0, 0.0);

	if (mode == 0) {
		vec3 phong_color = (ambient + diffuse + specular) * terrainColor.rgb;

		// shadow with acne prevention
		float bias = 0.005;
		float visibility = 1.0;
		if ( texture( shadow_tex, fs_in.shadow_coord.xy ).r  <  fs_in.shadow_coord.z - bias){
			visibility = 0.3;
		}
		vec4 shadow_color = vec4(phong_color * visibility, terrainColor.a);

		// apply fog
		vec4 fColor = withFog(shadow_color) ;
		fColor.a = 1.0 ;
		fragColor0 = fColor ;

		if(enable_cel_shading == 1){
			// cel shading
			if(diff > 0.66){
				// fragColor0 = fragColor0;
			}else if(diff > 0.33){
				fragColor0 = fragColor0 * 0.66f;
			}else{
				fragColor0 = fragColor0 * 0.33f ;
			}
			// outline
			if (dot(L, N) < mix(0.7, 0.1, max(0.0, dot(N, L)))){
				if(diff > 0.1){
					//fragColor0 = fragColor0;
				}else{
					fragColor0 = fragColor0 * 0; 
				}
				//fragColor0 = fragColor0 * 0; 
			}
			fragColor0.a = 1.0;
		}
	}
	else if (mode == 3) {	// ambient color map
		fragColor0 = vec4(terrainColor.rgb, 1.0);
	}
	else if (mode == 4) {	// diffuse color map
		fragColor0 = vec4(terrainColor.rgb, 1.0);
	}
	else if (mode == 5) {	// specular color map
		fragColor0 = vec4(0.0, 0.0, 0.0, 1.0);
	}
}

void main(){
	switch (mode) {
		case 0:	// normal mode
		case 3: // ambient color map
		case 4: // diffuse color map
		case 5:	// specular color map
			renderTerrain() ;
			break;
		case 1: // world space vertex
			fragColor0 = vec4(fs_in.WV, 1.0);
			break;
		case 2: // world space normal
			fragColor0 = vec4(fs_in.WN, 1.0);
			break;
		default: 
			fragColor0 = vec4(0.0, 0.0, 0.0, 1.0);
			break;
	}
	
}