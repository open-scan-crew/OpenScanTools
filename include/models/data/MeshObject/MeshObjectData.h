#ifndef MESH_OBJECT_DATA_H
#define MESH_OBJECT_DATA_H

#include <filesystem>
#include <glm/glm.hpp>
#include "models/3d/MeshId.h"

class MeshObjectData
{
public:
	MeshObjectData();
	MeshObjectData(const MeshObjectData& data);
	~MeshObjectData();

	void copyWavefrontData(const MeshObjectData& data);

	void setFilePath(const std::filesystem::path& filePath);
	void setObjectName(const std::wstring& name);
	void setMeshId(const MeshId& id);
	void setDimension(const glm::vec3& dimension);

	std::filesystem::path getFilePath() const;
	std::wstring getObjectName() const;
	MeshId getMeshId() const;
	glm::vec3 getDimension() const;

protected:
	std::filesystem::path	m_filePath;
	std::wstring			m_objectName;
	MeshId					m_meshId;
	glm::vec3				m_dimension;
};

#endif
