#include "models/3d/GeometryGenerator.h"
#include <glm/gtc/constants.hpp>

#include "utils/Logger.h"

GeometryGenerator::GeometryGenerator()
{}

GeometryGenerator::~GeometryGenerator()
{}

void GeometryGenerator::generateBoxWire(std::vector<glm::vec3>& vertices, std::vector<uint32_t>& indices)
{
	vertices = std::vector<glm::vec3>({
		{ -1.f, -1.f, -1.f },
		{ -1.f, -1.f,  1.f },
		{ -1.f,  1.f, -1.f },
		{ -1.f,  1.f,  1.f },
		{  1.f, -1.f, -1.f },
		{  1.f, -1.f,  1.f },
		{  1.f,  1.f, -1.f },
		{  1.f,  1.f,  1.f }
	});

	indices = std::vector<uint32_t>({
		0, 1, 
		0, 2,
		0, 4,
		7, 5,
		7, 6,
		7, 3,
		3, 2,
		3, 1,
		4, 5,
		4, 6,
		1, 5,
		2, 6
	});
}

void GeometryGenerator::generateCylinderWire(uint32_t sectorCount, std::vector<glm::vec3>& vertices, std::vector<uint32_t>& indices)
{
	vertices.clear();
	indices.clear();
	const float angle(glm::pi<float>() * 2.0f / (float)sectorCount);

	// Main axis
	vertices.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
	vertices.push_back(glm::vec3(0.0f, 0.0f, -1.0f));
	indices.push_back(0);
	indices.push_back(1);

	for (uint32_t i = 0; i < sectorCount; ++i)
	{
		float x = sinf(angle * i);
		float y = cosf(angle * i);
		vertices.push_back(glm::vec3(x, y, 1.f));
		uint32_t vtop = (uint32_t)vertices.size() - 1;
		vertices.push_back(glm::vec3(x, y, -1.f));
		uint32_t vbot = (uint32_t)vertices.size() - 1;

		// top edge
		indices.push_back(vtop);
		indices.push_back((vtop + 2));

		// sector edge
		//indices.push_back(vtop);
		//indices.push_back(vbot);

		// bottom edge
		indices.push_back(vbot);
		indices.push_back((vbot + 2));
	}
	vertices.push_back(glm::vec3(0.0f, 1.0f, 1.0f));
	vertices.push_back(glm::vec3(0.0f, 1.0f, -1.0f));
}

void GeometryGenerator::generateSphereWire(uint32_t sectorCount, uint32_t stackCount, std::vector<glm::vec3>& vertices, std::vector<uint32_t>& indices)
{
	constexpr float m_pi2(glm::pi<float>() / 2.0f);
	constexpr float m_2pi(glm::pi<float>() * 2.0f);
	const float sectorStep = m_2pi / sectorCount;
	const float stackStep = glm::pi<float>() / stackCount;

	for (uint32_t j = 0; j < sectorCount; ++j)
	{
		const float sectorAngle = j * sectorStep;
		const float X = cosf(sectorAngle);
		const float Y = sinf(sectorAngle);
		for (uint32_t i = 0; i < stackCount + 1; ++i)
		{
			const float stackAngle = m_pi2 - i * stackStep;
			const float x = X * cosf(stackAngle);
			const float y = Y * cosf(stackAngle);
			const float z = sinf(stackAngle);
			vertices.push_back({ x, y, z });
			if (i > 0)
			{
				uint32_t v1 = (uint32_t)vertices.size() - 1;
				indices.push_back(v1 - 1);
				indices.push_back(v1);
			}
			if (i > 0 && i < stackCount)
			{
				uint32_t v1 = (uint32_t)vertices.size() - 1;
				uint32_t v2 = (v1 + stackCount + 1) % (sectorCount * (stackCount + 1));
				indices.push_back(v1);
				indices.push_back(v2);
			}
		}
	}
}

void GeometryGenerator::generateTorusWire(float angleRad, float mainRadius, float tubeRadius, uint32_t sectorCount, std::vector<glm::vec3>& vertices, std::vector<uint32_t>& indices)
{
	constexpr float m_pi2(glm::pi<float>() / 2.0f);
	constexpr float m_2pi(glm::pi<float>() * 2.0f);
	// Calculate and cache counts of vertices and indices
	const uint32_t mainSectorCount = (uint32_t)((float)sectorCount * angleRad / m_2pi);

	float mainAngleStep(angleRad / (float)mainSectorCount);
	float tubeAngleStep(m_2pi / (float)sectorCount);


	//first circle
	glm::vec3 stackDir(1.f, 0.f, 0.f);
	vertices.push_back(glm::vec3(mainRadius, 0.f, 0.f) + stackDir * tubeRadius);
	for (uint32_t i = 1; i <= sectorCount; i++)
	{
		float stackAngle = i * tubeAngleStep;
		const glm::vec3 stackDir(cosf(stackAngle), 0.f, sinf(stackAngle));
		vertices.push_back(glm::vec3(mainRadius, 0.f, 0.f) + stackDir * tubeRadius);

		indices.push_back((uint32_t)vertices.size() - 2);
		indices.push_back((uint32_t)vertices.size() - 1);
	}

	//center axe
	float sectorAngle = 0.f;
	glm::vec3 sectorCenter(cosf(sectorAngle) * mainRadius, sinf(sectorAngle) * mainRadius, 0.f);
	vertices.push_back(sectorCenter);
	for (uint32_t j = 1; j <= mainSectorCount; j++)
	{
		sectorAngle += mainAngleStep;
		sectorCenter = glm::vec3(cosf(sectorAngle) * mainRadius, sinf(sectorAngle) * mainRadius, 0.f);
		vertices.push_back(sectorCenter);
		indices.push_back((uint32_t)vertices.size() - 2);
		indices.push_back((uint32_t)vertices.size() - 1);
	}

	//second circle
	stackDir = glm::vec3(cosf(sectorAngle), sinf(sectorAngle), 0.f);
	vertices.push_back(sectorCenter + stackDir * tubeRadius);
	for (uint32_t i = 1; i <= sectorCount; i++)
	{
		float stackAngle = i * tubeAngleStep;
		const glm::vec3 stackDir(cosf(sectorAngle) * cosf(stackAngle), sinf(sectorAngle) * cosf(stackAngle), sinf(stackAngle));
		vertices.push_back(sectorCenter + stackDir * tubeRadius);

		indices.push_back((uint32_t)vertices.size() - 2);
		indices.push_back((uint32_t)vertices.size() - 1);
	}
}
