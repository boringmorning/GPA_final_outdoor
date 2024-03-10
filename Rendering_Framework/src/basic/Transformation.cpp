#include "Transformation.h"


Transformation::Transformation()
{
	m_position[0] = 0;
	m_position[1] = 0;
	m_position[2] = 0;

	m_rotation[0] = 0;
	m_rotation[1] = 0;
	m_rotation[2] = 0;

	m_magnitude[0] = 0;
	m_magnitude[1] = 0;
	m_magnitude[2] = 0;

	m_translateMat = glm::mat4(1.0);
	m_rotateMat = glm::mat4(1.0);
	m_scaleMat = glm::mat4(1.0);
	m_modelMat = glm::mat4(1.0);

	m_parent = nullptr;
	m_children.clear();
}


Transformation::~Transformation()
{
}

void Transformation::translate(const glm::vec3 &magnitude){
	glm::vec4 ans = m_rotateMat * glm::vec4(magnitude, 1);
	
	m_position[0] = m_position[0] + ans[0];
	m_position[1] = m_position[1] + ans[1];
	m_position[2] = m_position[2] + ans[2];
}
void Transformation::setPosition(const glm::vec3 &pos) {
	this->m_position = pos;
}
void Transformation::scale(const glm::vec3 &scale){
	m_scaleMat = glm::scale(scale);
}
void Transformation::rotate(const glm::vec3 &rot) {
	m_rotation = rot;
	glm::quat quaternion = glm::quat(glm::radians(m_rotation));
	m_rotateMat = glm::toMat4(quaternion);
}
void Transformation::setRotatationMat(const glm::mat4 &rotMat) {
	m_rotateMat = rotMat;
}
glm::mat4 Transformation::rotationMat() const {
	return this->m_rotateMat;
}
void Transformation::setUpTransformation(){
	// Calculate Translation Matrix
	m_translateMat = glm::translate(m_position);
	// Calculate Rotation Matrix, Remember change degree to radians	
	//m_rotateMat = glm::rotate(glm::radians(m_rotation[1]), glm::vec3(0, 1, 0));	
	
	// Transform Matrix = parent * translate * rotate * scale
	if (m_parent != nullptr){
		m_modelMat = m_parent->m_modelMat * m_translateMat * m_rotateMat * m_scaleMat;
	}
	else{
		m_modelMat = m_translateMat * m_rotateMat * m_scaleMat;
	}

	// Update children
	int childCount = m_children.size();
	for (int i = 0; i < childCount; i++){
		m_children[i]->setUpTransformation();
	}
}
void Transformation::addChild(Transformation *child){
	m_children.push_back(child);
	child->setParent(this);
}
void Transformation::setParent(Transformation *parent){
	m_parent = parent;
}

