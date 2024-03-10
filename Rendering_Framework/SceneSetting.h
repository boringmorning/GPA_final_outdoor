#pragma once

#include <fstream>
#include <tuple>

class PlantManager {
public:
	PlantManager();
	virtual ~PlantManager();

public:
	int m_numPlantInstance[4]; 
	float *m_plantInstancePositions[4];

public:
	static std::tuple<int, float*> appendPlants(const std::string &posFile);
};
