#pragma once

#include <glm\mat4x4.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\type_ptr.hpp>


#include <glad\glad.h>

// Singleton

class SceneManager
{
private:
	SceneManager(){}
	

public:	
	
	virtual ~SceneManager(){}

	static SceneManager *Instance(){
		static SceneManager *m_instance = nullptr;
		if (m_instance == nullptr){
			m_instance = new SceneManager();
		}
		return m_instance;
	}

	GLint m_vertexHandle;
	GLint m_normalHandle;
	GLint m_uvHandle;

	GLuint m_projMatHandle;
	GLuint m_viewMatHandle;
	GLuint m_modelMatHandle;
	GLuint m_shadowMatHandle;
	GLuint m_pointlightHandle;
	GLuint modeHandle;
	GLuint planeHandle;

	GLuint m_texture0Handle;

	GLuint m_vToHeightUVMatHandle;
	GLuint m_elevationMapHandle;
	GLuint m_normalMapHandle;	
	
	GLenum m_elevationMapTexUnit;
	GLenum m_normalMapTexUnit;
	GLenum m_diffuseTexUnit;

	GLuint enable_cel_shading;
};

