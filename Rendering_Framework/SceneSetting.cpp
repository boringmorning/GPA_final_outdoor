#include "SceneSetting.h"

PlantManager::PlantManager() {
	auto tree0 = appendPlants(
		"assets\\baum_hd_pine_pos.pos");
	this->m_numPlantInstance[0] = std::get<0>(tree0);
	this->m_plantInstancePositions[0] = std::get<1>(tree0);

	auto tree1 = appendPlants(
		"assets\\baum_hd_pos.pos");
	this->m_numPlantInstance[1] = std::get<0>(tree1);
	this->m_plantInstancePositions[1] = std::get<1>(tree1);

	auto grass0 = appendPlants(
		"assets\\grass_pos.pos");	
	this->m_numPlantInstance[2] = std::get<0>(grass0);
	this->m_plantInstancePositions[2] = std::get<1>(grass0);

	auto grass2 = appendPlants(
		"assets\\grass2_pos.pos");
	this->m_numPlantInstance[3] = std::get<0>(grass2);
	this->m_plantInstancePositions[3] = std::get<1>(grass2);
}
PlantManager::~PlantManager(){}
std::tuple<int, float*> PlantManager::appendPlants(const std::string &posFile) {
	// load positions
	int numPos = -1;
	std::ifstream input(posFile, std::ios::binary);
	if (!input.is_open()) {
		return{ -1, nullptr };
	}
	input.read((char*)(&numPos), sizeof(int));
	float *positions = new float[numPos * 3];
	input.read((char*)positions, numPos * 3 * sizeof(float));
	input.close();

	return{ numPos, positions };
}