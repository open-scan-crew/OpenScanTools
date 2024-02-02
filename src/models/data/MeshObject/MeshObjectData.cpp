#include "models/data/MeshObject/MeshObjectData.h"

MeshObjectData::MeshObjectData()
{}

MeshObjectData::MeshObjectData(const MeshObjectData& data)
{
	copyWavefrontData(data);
}

MeshObjectData::~MeshObjectData()
{
}

void MeshObjectData::copyWavefrontData(const MeshObjectData& data)
{
	m_filePath = data.getFilePath();
	m_objectName = data.getObjectName();
	m_dimension = data.getDimension();
	m_meshId = data.getMeshId();
}

void MeshObjectData::setFilePath(const std::filesystem::path& filePath)
{
	m_filePath = filePath;
}

void MeshObjectData::setObjectName(const std::wstring& name)
{
	m_objectName = name;
}

void MeshObjectData::setMeshId(const MeshId& id)
{
	m_meshId = id;
}

void MeshObjectData::setDimension(const glm::vec3& dimension)
{
	m_dimension = dimension;
}

std::filesystem::path MeshObjectData::getFilePath() const
{
	return m_filePath;
}

std::wstring MeshObjectData::getObjectName() const
{
	return m_objectName;
}

MeshId MeshObjectData::getMeshId() const
{
	return m_meshId;
}

glm::vec3 MeshObjectData::getDimension() const
{
	return m_dimension;
}