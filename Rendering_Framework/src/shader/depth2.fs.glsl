#version 430 core                                                              
                                                                              
out vec4 color;

in VS_OUT
{
	vec2 texcoord;
} fs_in;

uniform sampler2D tex;
uniform bool isLeaves;
                                                                                                                                              
void main(void)                                                                
{                             
	if (isLeaves) {
		float transparency = texture(tex, fs_in.texcoord).a;
		if (transparency < 0.5)
			discard;
	}
	color = vec4(vec3(gl_FragCoord.z), 1.0);
}                                                                              