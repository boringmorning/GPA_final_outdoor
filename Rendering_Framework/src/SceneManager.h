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

	glm::mat4 m_viewMat;
	glm::mat4 m_projMat;
	glm::mat4 m_viewProjMat;

	GLint m_vertexHandle;
	GLint m_normalHandle;
	GLint m_uvHandle;
	//GLint m_boneIndexHandle;
	//GLint m_weightHandle;
	//GLint m_tangentHandle;

	GLuint m_projMatHandle;
	GLuint m_viewMatHandle;
	GLuint m_modelMatHandle;
	GLuint m_lightMVPHandle[3];
	GLuint m_lightViewHandle[3];
	GLuint m_lightProjHandle[3];

	GLuint m_materialHandle;
	GLuint m_texture0Handle;
	GLuint m_shadowMapHandle[3];

	GLuint m_grassTextureHandle;
	GLuint m_normalMapHandle;
	GLuint m_trackMaskHandle;
	

	GLuint m_currentFrameHandle;
	GLuint m_preFrameHandle;
	GLuint m_motionVectorMapHandle;
	GLuint m_firstFrameFlagHandle;
	GLuint m_noJitteredProjMatHandle;
	GLuint m_preVPHandle;
	GLuint m_preModelMatHandle;
	GLuint m_currentDepthHandle;


	GLuint m_vToHeightUVMatHandle;
	GLuint m_vToDiffuseUVMatHandle;
	GLuint m_elevationMapHandle;

	GLuint m_gaussianWeights;

	GLuint m_lightPositionInViewHandle;

	GLuint m_vs_recordDepth;
	GLuint m_vs_render;
	GLuint m_vs_postProcessing;
	GLuint m_vs_terrain;
	GLuint m_vs_motionVector;
	GLuint m_vs_terrainMotionVector;
	GLuint m_vs_modifyTerrainTexture;
	//GLuint m_vs_skeletonDeform;
	//GLuint m_vs_recordDepthOfSkeletonDeform;
	//GLuint m_vs_bumpMapping;

	GLuint m_fs_recordDepth;
	GLuint m_fs_phoneShading;
	GLuint m_fs_textureMapping;
	GLuint m_fs_gaussianXPass;
	GLuint m_fs_gaussianYPass;
	GLuint m_fs_renderTerrain;
	GLuint m_fs_accumulate;
	GLuint m_fs_framePostProcessing;
	GLuint m_fs_motionVector;
	GLuint m_fs_modifyTerrainTexture;

	GLuint m_fs_shadowFlag;
	GLuint m_fs_shadingFlag;
	
	
	GLuint m_fs_pureTextureMapping;
	//GLuint m_fs_bumpMappingWithShadow;

	//GLuint m_boneTransformMatrixSSBO;	
};

