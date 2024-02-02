#ifndef STEP_SIMPLIFICATION_H
#define STEP_SIMPLIFICATION_H

#include <iostream>
#include <chrono>
#include <string>
#include <fstream>

#include <filesystem>

#include "io/imports/step-simplification/step-simplificationTypes.h"
#include "controller/Controller.h"

#include "vulkan/Graph/MemoryReturnCode.h"

class StepSimplification {
public:
	static ObjectAllocation::ReturnCode modelSimplification(const std::filesystem::path& inputFilePath, const std::filesystem::path& outputFilePath, const StepClassification& classification, const double& keepPercent, Controller& controller);
private:
	static void startTimer(std::string display, std::stringstream& ss);
	static void stopTimer(std::stringstream& ss);
	static void totalRunningTime();
	static std::ifstream::pos_type filesize(const char* filename);
};
#endif