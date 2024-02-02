#ifndef UNIFORM_CLIPPING_DATA
#define UNIFORM_CLIPPING_DATA

#include <glm/glm.hpp>
#include <vector>

class ClippingAssembly;

struct UniformClippingData
{
    glm::mat4 transfo; // the inverse model transformation of the clipping
    glm::vec4 limits; // depends on the geometry : radius, bounds, acos, etc
    glm::vec3 color; // (r, g, b)
    int rampSteps;
};

void generateUniformData(ClippingAssembly& _ca, std::vector<UniformClippingData>& _shaderData);

#endif