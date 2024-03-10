#version 430 core

layout(location = 0) out vec4 fragColor;

uniform sampler2D reflectionTex;
uniform sampler2D refractionTex;
uniform sampler2D shadow_tex;

in VS_OUT
{
	vec4 clip_pos;
	vec4 shadow_coord;
	vec3 WV;
	vec3 WN;
	vec2 texcoord;
	vec3 toCameraVector;
} fs_in;

uniform int mode;

const float waveStrength = 0.02;

void mixEffect() {
	vec2 ndc = (fs_in.clip_pos.xy / fs_in.clip_pos.w) / 2.0 + 0.5;
	vec2 reflectionTexcoord = vec2(ndc.x, 1.0 - ndc.y);
	vec2 refractionTexcoord = vec2(ndc.x, ndc.y);

	vec4 reflectionColor = texture(reflectionTex, reflectionTexcoord);
	vec4 refractionColor = texture(refractionTex, refractionTexcoord);


	// fresnel effect
	float refractiveFactor = dot(vec3(0, 1, 0), normalize(fs_in.toCameraVector));

	fragColor = mix(reflectionColor, refractionColor, refractiveFactor);
	fragColor = mix(fragColor, vec4(0.0, 0.3, 0.5, 1.0), 0.2);

	// shadow with acne prevention
	float bias = 0.00;
	float visibility = 1.0;
	if ( texture( shadow_tex, fs_in.shadow_coord.xy ).r  <  fs_in.shadow_coord.z - bias){
		visibility = 0.3;
	}

	fragColor *= visibility;
	//fragColor = vec4(texture(dudvmap, vec2(fs_in.texcoord.x, fs_in.texcoord.y)).rgb, 1.0);
}

void main()
{
	switch(mode) {
		case 0:
			mixEffect();
			break;
		case 1:
			fragColor = vec4(fs_in.WV, 1.0);
			break;
		case 2:
			fragColor = vec4(fs_in.WN, 1.0);
			break;
		case 3:
			fragColor = vec4(0.0, 0.0, 0.0, 1.0);
			break;
		case 4:
			fragColor = vec4(0.0, 0.0, 1.0, 1.0);
			break;
		case 5:
			fragColor = vec4(0.0, 0.0, 0.0, 1.0);
			break;
		default:
			fragColor = vec4(0.0, 0.0, 0.0, 1.0);
			break;
	}
	
}