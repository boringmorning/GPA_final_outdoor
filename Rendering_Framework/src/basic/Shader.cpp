#include "Shader.h"

// version: 160907

Shader::Shader(std::string vetShFile, std::string fraShFile)
{
	/////////////
	// Prepare code buffer
	char *shaderCode = new char[11111];
	// Set up vertex shader
	this->readProgram((char*)(vetShFile.c_str()), shaderCode);
	GLuint m_vertexShaderId = this->loadShader(shaderCode, GL_VERTEX_SHADER);
	// Set up fragment shader
	this->readProgram((char*)(fraShFile.c_str()), shaderCode);
	GLuint m_fragmentShaderId = this->loadShader(shaderCode, GL_FRAGMENT_SHADER);

	// Create Program
	m_programID = glCreateProgram();

	// Attach shader
	glAttachShader(m_programID, m_vertexShaderId);
	glAttachShader(m_programID, m_fragmentShaderId);

	// Link program
	glLinkProgram(m_programID);

	// It is not required to hande the shader in memory
	glDeleteShader(m_vertexShaderId);
	glDeleteShader(m_fragmentShaderId);

	// Delete shader code
	delete[] shaderCode;
}
Shader::Shader(char *vetShFile, char *geoShFile, char *fraShFile){
	/////////////
	// Prepare code buffer
	char *shaderCode = new char[11111];
	// Set up vertex shader
	this->readProgram(vetShFile, shaderCode);
	GLuint m_vertexShaderId = this->loadShader(shaderCode, GL_VERTEX_SHADER);
	// Set up geometry shader
	this->readProgram(geoShFile, shaderCode);
	GLuint m_geometryShaderId = this->loadShader(shaderCode, GL_GEOMETRY_SHADER);
	// Set up fragment shader
	this->readProgram(fraShFile, shaderCode);
	GLuint m_fragmentShaderId = this->loadShader(shaderCode, GL_FRAGMENT_SHADER);

	// Create Program
	m_programID = glCreateProgram();

	// Attach shader
	glAttachShader(m_programID, m_vertexShaderId);
	glAttachShader(m_programID, m_geometryShaderId);
	glAttachShader(m_programID, m_fragmentShaderId);

	// Link program
	glLinkProgram(m_programID);

	// It is not required to hande the shader in memory
	glDeleteShader(m_vertexShaderId);
	glDeleteShader(m_fragmentShaderId);

	// Delete shader code
	delete[] shaderCode;
}

Shader::~Shader(void)
{
	glDeleteProgram(m_programID);
}

void Shader::readProgram(char *shaderFile, char *shaderCode){
	char data[5000];
	shaderCode[0] = '\0';

	ifstream inputFile(shaderFile);
	if (!inputFile.is_open()) {
		cout << "Shader file failed" << endl;
		system("pause");
		exit(0);
	}

	std::string line;
	while (std::getline(inputFile, line)) {
		strcat_s(shaderCode, 11000, line.c_str());
		strcat_s(shaderCode, 11000, "\r\n");
	}

	inputFile.close();
}
int Shader::loadShader(char *shaderCode, GLenum shaderType){
	int shaderID ;
	shaderID = glCreateShader(shaderType) ;
	glShaderSource(shaderID, 1, &shaderCode, NULL) ;
	glCompileShader(shaderID) ;
	char compileError[1000] = "";
	glGetShaderInfoLog(shaderID, 1000, NULL, compileError) ;
	if(strcmp(compileError, "")){
		cout << compileError << endl ;
		system("pause") ;
		exit(0) ;
	}
	cout << "shader compiling complete !!" << endl ;
	return shaderID ;	
}
void Shader::useShader(){	
	glUseProgram(m_programID);
}
void Shader::disableShader(){	
	glUseProgram(0);
}
GLuint Shader::getProgramID(){
	return m_programID ;
}
