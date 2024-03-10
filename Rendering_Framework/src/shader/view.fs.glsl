#version 430 core                                                              
                                                                              
out vec4 fragColor;

uniform sampler2D tex;
uniform sampler2D blurTex;

uniform bool activateBloom;
                                                                               
in VertexData                                                                      
{                                                                              
    vec2 texcoord;                                                             
} vertexData;                                                                       
                                                                               
void main(void)                                                                
{                         
	if (activateBloom) {
		const float gamma = 2.2;
		const float exposure = 2.0;
		vec3 hdrColor = texture(tex, vertexData.texcoord).rgb;      
		vec3 bloomColor = texture(blurTex, vertexData.texcoord).rgb;
		hdrColor += bloomColor; // additive blending
		fragColor = vec4(hdrColor, 1.0);

		// tone mapping
		vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
		// also gamma correct while we're at it       
		result = pow(result, vec3(1.0 / gamma));
		fragColor = vec4(result, 1.0);
		
	}
	else {
		fragColor = vec4(texture(tex, vertexData.texcoord).rgb, 1.0);
	}
}                                                                              