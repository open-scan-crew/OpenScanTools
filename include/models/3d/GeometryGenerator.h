#ifndef GEOMETRY_GENERATOR_H_
#define GEOMETRY_GENERATOR_H_

#include <vector>
#include <glm/glm.hpp>

class GeometryGenerator
{
public:
	GeometryGenerator();
	~GeometryGenerator();

	static void generateBoxWire(std::vector<glm::vec3>& vertices, std::vector<uint32_t>& edgesIndices);
	static void generateCylinderWire(uint32_t sectorCount, std::vector<glm::vec3>& vertices, std::vector<uint32_t>& edgesIndices);
	static void generateSphereWire(uint32_t sectorCount, uint32_t stackCount, std::vector<glm::vec3>& vertices, std::vector<uint32_t>& edgesIndices);
	static void generateTorusWire(float angleRad, float mainRadius, float tubeRadius, uint32_t sectionCount, std::vector<glm::vec3>& vertices, std::vector<uint32_t>& edgesIndices);
};

#endif //!GEOMETRY_GENERATOR_H_
