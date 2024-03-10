#version 430 core

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 bright;

uniform int mode;

in VS_OUT
{
	vec3 WV;	// world space vertex
	vec3 WN;	// world space normal
} fs_in;

void main()
{
	if (mode == 1) {
		fragColor = vec4(fs_in.WV, 1.0);
	}
	else if (mode == 2) {
		fragColor = vec4(fs_in.WN, 1.0);
	}
	else {
		fragColor = vec4(2.0, 2.0, 0.5, 1.0);
		bright = vec4(1.0, 1.0, 0.5, 1.0);
	}
}