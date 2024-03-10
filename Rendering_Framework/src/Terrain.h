#pragma once

#include <fstream>
#include "basic\TextureMaterial.h"
#include "basic\Transformation.h"
#include "basic\SceneManager.h"
#include "depth.h"

class Terrain
{
public:
	Terrain(Depth *depth);
	virtual ~Terrain();
	float getHeight(const float x, const float z) const;

private:
	GLuint m_vao;
	GLuint m_dataBuffer;
	int m_dataByteOffset;
	Transformation *m_transform;

private:
	glm::vec3 worldVToHeightMapUV(float x, float z) const;

	void fromMYTD(const std::string &filename);

	int m_width;
	int m_height;
	float m_heightScale;

private:
	GLuint m_indexBuffer;
	glm::mat4 m_worldToHeightUVMat;
	glm::mat4 m_worldToDiffuseUVMat;

	TextureMaterial *m_elevationTex;
	TextureMaterial *m_normalTex;
	TextureMaterial *m_colorTex;
	

	float* m_elevationMap;
	float* m_normalMap;
	float* m_colorMap;

	float m_heightFactor = 50.0;

	// 4 rotation matrix for chunk
	glm::mat4 m_chunkRotMat[4];


public:
	void update(GLuint reflection_fbo, GLuint refraction_fbo, vec3 view_point_light_pos2, mat4 view_matrix2, bool activateWater);

public:
	void setCameraPosition(const glm::vec3 &pos);

};

