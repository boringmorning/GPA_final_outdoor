#pragma once

#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

class Transformation
{
private:
	glm::mat4 m_translateMat;
	glm::mat4 m_rotateMat;
	glm::mat4 m_scaleMat;

	Transformation *m_parent;
	std::vector<Transformation*> m_children;

public:
	Transformation();
	virtual ~Transformation();

	glm::vec3 m_position;
	glm::vec3 m_rotation;
	glm::vec3 m_magnitude;
	glm::vec3 m_scale;

	glm::mat4 m_modelMat;

	void translate(const glm::vec3 &magnitude);
	void setPosition(const glm::vec3 &pos);
	void setUpTransformation();

	void addChild(Transformation *child);
	void setParent(Transformation *parent);
	void scale(const glm::vec3 &scale); 
	void rotate(const glm::vec3 &rot);
	void setRotatationMat(const glm::mat4 &rotMat);
	glm::mat4 rotationMat() const;
};

	

