#version 430 core

layout(location = 0) out vec4 fragColor;

uniform sampler2D tex;
uniform sampler2D shadow_tex;
uniform int mode;
uniform int enable_cel_shading;
uniform int isLeaves;

in VS_OUT
{
	vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 V; // eye space view vector
	vec3 L2; // eye space point light vector
	vec2 texcoord;
	vec4 shadow_coord;
	vec3 WV;	// world space vertex
	vec3 WN;	// world space normal
} fs_in;

const vec3 ambient_light = vec3(0.1);
const vec3 diffuse_light = vec3(0.8);
const vec3 specular_light = vec3(0.1);
const vec3 ka = vec3(1.0);
const vec3 kd = vec3(0.8);
const vec3 ks[2] = {vec3(0.5), vec3(0.0)};
const float shininess[2] = {2.0, 1.0};
// point light
const vec3 point_color = vec3(1.0, 1.0, 0.5);
const vec3 point_diffuse_light = point_color * vec3(0.85);
const vec3 point_specular_light = point_color * vec3(0.15);
// attenuation variables
const float c1 = 1.0;
const float c2 = 0.005;
const float c3 = 0.001;

void blinn_phong() {
	// blin-phong shading
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
	vec3 specular = ( pow(max(dot(N, H), 0.0), shininess[isLeaves]) * specular_light + pow(max(dot(N, H2), 0.0), shininess[isLeaves]) * attenuation_factor * point_specular_light );
	vec4 texColor = texture(tex, fs_in.texcoord);

	if (mode == 0) {
		vec3 phong_color = (ambient + diffuse + specular) * texColor.rgb;

		// shadow
		float bias = 0.0;
		float visibility = 1.0;
		if ( texture( shadow_tex, fs_in.shadow_coord.xy ).r  <  fs_in.shadow_coord.z - bias){
			visibility = 0.3;
		}
		fragColor = vec4(phong_color * visibility, texColor.a);

		// https://en.wikibooks.org/wiki/GLSL_Programming/Unity/Toon_Shading#Outlines
		// Cel Shading

		if(enable_cel_shading == 1){
			if(diff > 0.66){
				// fragColor = fragColor;
			}else if(diff > 0.33){
				fragColor = fragColor * 0.66f;
			}else{
				fragColor = fragColor * 0.33f;
			}
			// Outline
			if (dot(V, N) < mix(0.6, 0.1, diff))
			{
				fragColor = fragColor * 0; 
			}
			fragColor.a = texColor.a;
		}
	}
	else if(mode == 3) {	// ambient color map
		fragColor = vec4(ka, 1.0);
	}
	else if(mode == 4) {	// diffuse color map
		fragColor = vec4(texColor);
	}
	else if(mode == 5) {	// specular color map
		fragColor = vec4(ks[isLeaves], 1.0);
	}
}


void main()
{
	if (texture(tex, fs_in.texcoord).a < 0.5)
		discard;
	switch (mode) {
		case 0:
		case 3:
		case 4:
		case 5:
			blinn_phong();
			break;
		case 1:
			fragColor = vec4(fs_in.WV, 1.0);
			break;
		case 2:
			fragColor = vec4(fs_in.WN, 1.0);
			break;
		default:
			fragColor = vec4(1.0, 1.0, 1.0, 1.0);
			break;
	}
}