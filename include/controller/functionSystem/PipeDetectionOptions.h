#ifndef PIPE_DETECTION_OPTIONS_H
#define PIPE_DETECTION_OPTIONS_H

#include <glm/glm.hpp>

enum class PipeDetectionExtendMode
{
    Default,
    Manual,
    Auto
};

struct PipeDetectionOptions
{
	bool noisy = false;
	PipeDetectionExtendMode extendMode = PipeDetectionExtendMode::Default;
	bool optimized = false;
	double insulatedThickness = 0.0;
};
#endif