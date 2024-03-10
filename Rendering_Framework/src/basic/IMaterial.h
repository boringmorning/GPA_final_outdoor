#pragma once

#include "SceneManager.h"
#include <string>

class IMaterial
{
protected:
	// Preserving the Scene Manager Pointer
	const SceneManager *m_manager;

public:
	IMaterial(GLenum mode, int type) : 
		m_mode(mode), m_materialType(type){
		m_manager = SceneManager::Instance();

		m_vertexStart = 0;
		m_vertexCount = 0;

		// Default setting
		this->setAttribute(glm::vec4(0.0, 0.0, 0.0, 1.0), glm::vec4(0.0, 0.0, 0.0, 1.0), glm::vec4(0.0, 0.0, 0.0, 1.0));
		// Shininess
		m_material[12] = 1.0; m_material[13] = 0.0; m_material[14] = 0.0; m_material[15] = 0.0;
		// Emission
		m_material[16] = 0.0; m_material[17] = 0.0; m_material[18] = 0.0; m_material[19] = 1.0;
	}
	virtual ~IMaterial(){}

	void setAttribute(const glm::vec4 &ambient, const glm::vec4 &diffuse, const glm::vec4 &specular){
		for (int i = 0; i < 4; i++){
			m_material[i] = ambient[i];
			m_material[4 + i] = diffuse[i];
			m_material[8 + i] = specular[i];			
		}
	}

	void emission(const glm::vec4 &e){
		m_material[16] = e[0];
		m_material[17] = e[1];
		m_material[18] = e[2];
		m_material[19] = e[3];
	}

	const int m_materialType;
	const GLenum m_mode;
	std::string m_materialName;
	bool m_shadingFlag = true;

	float m_material[5 * 4];

	int m_vertexStart;
	int m_vertexCount;	

	void virtual bind() = 0;
	void virtual unbind(){}

	const static int DIFFUSE = 0;
	const static int TEXTURE = 1;
	const static int BUMP_MAPPING = 2;
};

