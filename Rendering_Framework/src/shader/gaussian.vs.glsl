#version 430 core                                                                                                                   

out VertexData                                                         
{                                                                  
    vec2 texcoord;                                                 
} vertexData;                                                          

const float position[] =
{
	1.0f,-1.0f,
	-1.0f,-1.0f,
	-1.0f,1.0f,
	1.0f,1.0f
};

const float texcoord[] = 
{
	1.0f,0.0f,
	0.0f,0.0f,
	0.0f,1.0f,
	1.0f,1.0f
};
                                                                   
                                                                   
void main(void)                                                    
{                                                                  
    gl_Position = vec4(position[gl_VertexID * 2], position[gl_VertexID * 2 + 1], 0.0, 1.0);							
    vertexData.texcoord = vec2(texcoord[gl_VertexID * 2], texcoord[gl_VertexID * 2 + 1]);
}	