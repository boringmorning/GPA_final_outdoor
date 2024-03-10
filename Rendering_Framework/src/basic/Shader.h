#pragma once
#include <iostream>
#include <fstream>
#include <string>

#include <glad\glad.h>


// version: 160907

using namespace std ;
class Shader
{
	GLuint m_programID ;	
	void readProgram(char *shaderFile, char *shaderCode) ;
	int loadShader(char *shaderCode, GLenum shaderType) ;	
public:
	Shader(std::string vetShFile, std::string fraShFile);
	// The program has Geometry Shader
	Shader(char *vetShFile, char *geoShFile, char *fraShFile);
	~Shader(void);
	void useShader() ;
	void disableShader() ;
    GLuint getProgramID() ;
};

