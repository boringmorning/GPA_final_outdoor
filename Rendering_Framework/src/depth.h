#include <glm\mat4x4.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <vector>
#include "basic\TextureMaterial.h"

using namespace glm;
using namespace std;

struct Depth {
	GLuint fbo;
	GLuint vao;			// vertex array object
	GLuint vbo;			// vertex buffer object
	GLuint ebo;			// element buffer object
	GLuint tex;
	GLuint tex2;
	vector<float> vertices;
	vector<unsigned int> indices;
	int terrianNum = 0;
	int indexCount = 0;
	TextureMaterial *elevationTex;
	mat4 chunkRotMat[4];
	mat4 vToHeightUVMat;
	int vertexCount = 0;
};