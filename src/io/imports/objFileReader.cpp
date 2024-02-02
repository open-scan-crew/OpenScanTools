#include "io/imports/objFileReader.h"
#include "gui/texts/SplashScreenTexts.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "utils/Utils.h"
#include "utils/Logger.h"

ObjFileReader::ObjFileReader(Controller* pController, const MeshObjInputData& inputInfo)
	: IMeshReader(pController, inputInfo)
{
	m_scale = 1.f;
	m_reader = new tinyobj::ObjReader();
}

ObjFileReader::~ObjFileReader()
{
	if (m_reader)
		delete m_reader;
}

bool ObjFileReader::read()
{
	if (!m_reader->ParseFromFile(m_inputInfo.path.string()))
		return false;

	m_maxCount = (int)m_reader->GetShapes().size();
	return true;
}

ObjectAllocation::ReturnCode ObjFileReader::generateGeometries()
{
	int count = 0;

	// TODO - Write functions for obj with only vertexes
	// TODO - Write functions for obj with vertexes, normals and texture coord

	if (m_inputInfo.isMerge)
	{
		MeshShape merge_shape;
		std::unordered_map<uint64_t, uint32_t> correspIndexes;

		for (const auto& shape : m_reader->GetShapes())
		{
			count++;

			mapShapeToGeometry_vn(shape.mesh, m_reader->GetAttrib(), correspIndexes, merge_shape.geometry);

			m_loadCount = count;
			if (!updateImportProcessUI(TEXT_SPLASH_SCREEN_IMPORT_GEOMETRIES.arg(count).arg(m_reader->GetShapes().size()), true))
				return ObjectAllocation::ReturnCode::Aborted;
		}
		merge_shape.name = m_inputInfo.path.stem().wstring();
		m_meshesShapes.push_back(merge_shape);
	}
	else
	{
		for (const tinyobj::shape_t& shape : m_reader->GetShapes())
		{
			count++;

			MeshShape meshShape;
			
			std::unordered_map<uint64_t, uint32_t> correspIndexes;
			mapShapeToGeometry_vn(shape.mesh, m_reader->GetAttrib(), correspIndexes, meshShape.geometry);
			meshShape.name = shape.name.empty() ? m_inputInfo.path.stem().wstring() + L"_" + Utils::wCompleteWithZeros(count) : Utils::from_utf8(shape.name);
			m_meshesShapes.push_back(meshShape);

			m_loadCount = count;
			if (!updateImportProcessUI(TEXT_SPLASH_SCREEN_IMPORT_GEOMETRIES.arg(count).arg(m_reader->GetShapes().size()), true))
				return ObjectAllocation::ReturnCode::Aborted;
		}
	}

	delete m_reader;
	m_reader = nullptr;

	return ObjectAllocation::ReturnCode::Success;
}

void ObjFileReader::getVertices(const tinyobj::attrib_t& attrib, MeshGeometries& geom) const
{
	for (uint64_t iterator(0); iterator < attrib.vertices.size();)
	{
		geom.vertices.push_back(attrib.vertices[iterator++]);
		geom.vertices.push_back(attrib.vertices[iterator++]);
		geom.vertices.push_back(attrib.vertices[iterator++]);
	}
}

uint64_t getHash(const tinyobj::index_t& index)
{
	return ((uint64_t)index.vertex_index << 32) + (uint64_t)index.normal_index;
}

void ObjFileReader::mapShapeToGeometry_vn(const tinyobj::mesh_t& mesh, const tinyobj::attrib_t& attrib, std::unordered_map<uint64_t, uint32_t>& correspIndexes, MeshGeometries& geom)
{
	std::unordered_set<uint32_t> unique_vertex_index;
	std::unordered_set<uint32_t> unique_normal_index;
	for (const tinyobj::index_t& index : mesh.indices)
	{
		unique_vertex_index.insert(index.vertex_index);
		unique_normal_index.insert(index.normal_index);
		uint64_t hashedIndex = getHash(index);
		if (correspIndexes.find(hashedIndex) == correspIndexes.end())
		{
			geom.indices.push_back((uint32_t)(geom.vertices.size() / 3));
			correspIndexes.insert({ hashedIndex, (uint32_t)(geom.vertices.size() / 3) });

			geom.vertices.push_back(attrib.vertices[index.vertex_index * 3u + 0]);
			geom.vertices.push_back(attrib.vertices[index.vertex_index * 3u + 1]);
			geom.vertices.push_back(attrib.vertices[index.vertex_index * 3u + 2]);

			if (index.normal_index >= 0)
			{
				geom.normals.push_back(attrib.normals[index.normal_index * 3 + 0]);
				geom.normals.push_back(attrib.normals[index.normal_index * 3 + 1]);
				geom.normals.push_back(attrib.normals[index.normal_index * 3 + 2]);
			}
			if (index.texcoord_index >= 0)
			{
				geom.texcoords.push_back(attrib.texcoords[index.texcoord_index * 2 + 0]);
				geom.texcoords.push_back(attrib.texcoords[index.texcoord_index * 2 + 1]);
			}
		}
		else
			geom.indices.push_back(correspIndexes.at(hashedIndex));
	}
	SubLogger& log = Logger::log(LoggerMode::IOLog);
	log << "--- Mesh Indexes convertion ---\n";
	log << "Initial : " << unique_vertex_index.size() << " vertices, " << unique_normal_index.size() << " normals, " << mesh.indices.size() << " double indexes\n";
	log << "Conversion : " << geom.vertices.size() / 3 << " vertices & normals, " << geom.indices.size() << " simple indexes\n";
	size_t initial_size = (unique_vertex_index.size() + unique_normal_index.size()) * 12 + mesh.indices.size() * 8;
	size_t final_size = geom.vertices.size() * 2 * 4 + geom.indices.size() * 4;
	float increase = ((float)final_size / (float)initial_size) * 100.f;
	log << "Size difference : initial " << initial_size / 1024 << " kio | final " << final_size / 1024 << " kio (" << increase << "%)" << Logger::endl;
}


void ObjFileReader::getVertexIndices(const std::vector<tinyobj::index_t>& indices, std::vector<uint32_t>& buffer) const
{
	for (const auto& index : indices)
		buffer.push_back(index.vertex_index);
}
