#ifndef FILE_INPUT_DATA_H
#define FILE_INPUT_DATA_H

#include "models/3d/ManipulationTypes.h"
#include "io/FileUtils.h"
#include "io/imports/ImportTypes.h"

#include <filesystem>

struct FileInputData {
	std::filesystem::path file;
	float scale;
	Selection up;
	Selection forward;
	bool isMerge;
	bool truncateCoordinatesAsTheScans = false;
	PositionOptions posOption;
	glm::vec3 position;
	FileType extension;
	int lod; //value between 1 and 100
};


#endif