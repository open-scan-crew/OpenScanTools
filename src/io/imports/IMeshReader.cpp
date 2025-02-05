#include "io\imports\IMeshReader.h"
#include "vulkan/VulkanManager.h"

#include "vulkan/Graph/MemoryReturnCode.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/Controller.h"

#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/SplashScreenTexts.hpp"

#include "utils/Logger.h"

IMeshReader::IMeshReader(Controller* pController, const MeshObjInputData& inputInfo)
	: m_pController(pController)
	, m_inputInfo(inputInfo)
	, m_loadCount(0)
	, m_maxCount(0)
	, m_scale(1.f)
{}

xg::Guid IMeshReader::getLoadedMeshId() const
{
	return xg::Guid();
}

int& IMeshReader::editLoadCountUI()
{
	return m_loadCount;
}

int& IMeshReader::editMaxCountUI()
{
	return m_maxCount;
}

void IMeshReader::createImportProcessUI(QString loadText, bool enableCancel)
{
	if (m_pController != nullptr)
	{
		m_pController->updateInfo(new GuiDataProcessingSplashScreenStart(m_maxCount,
			TEXT_SPLASH_SCREEN_IMPORT_EXTERNAL_DATA_TITLE_WITH_NAME.arg(QString::fromStdWString(m_inputInfo.path.filename().wstring())), loadText));
		m_pController->updateInfo(new GuiDataProcessingSplashScreenEnableCancelButton(enableCancel));
	}
}

bool IMeshReader::updateImportProcessUI(QString loadText, bool enableCancel)
{
	bool isContextRunning = true;

	if (m_pController != nullptr)
	{
		bool cancelState = enableCancel && (!getLoadedMeshId().isValid());

		if (
			(m_pController->getFunctionManager().getActiveContext() == nullptr
			|| m_pController->getFunctionManager().getActiveContext()->getState() != ContextState::running))
			isContextRunning = false;

		if(m_loadCount <= m_maxCount)
			m_pController->updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(loadText, m_loadCount));

		m_pController->updateInfo(new GuiDataProcessingSplashScreenEnableCancelButton(cancelState));
	}

	return isContextRunning;
}

ObjectAllocation::ReturnCode IMeshReader::allocateMeshBuffer(MeshBuffer& _mesh, const MeshShape& shape)
{
	ObjectAllocation::ReturnCode ret(virtualAllocateMeshBuffer(_mesh, shape));
	if (m_pController != nullptr && ret != ObjectAllocation::ReturnCode::Success)
		m_pController->updateInfo(new GuiDataProcessingSplashScreenLogUpdate(TEXT_SPLASH_SCREEN_IMPORT_PROCESSING_LOG_ERROR.arg(QString::fromStdWString(shape.name))));

	return ret;
}

std::vector<MeshShape>& IMeshReader::getMeshShapes()
{
	return m_meshesShapes;
}

void IMeshReader::fillMeshesBoundingBox(glm::vec3& merge_dim, glm::vec3& merge_center)
{
	if (m_meshesShapes.empty())
	{
		merge_dim = glm::vec3(0.f);
		merge_center = glm::vec3(0.f);
		return;
	}

	std::array<glm::vec3, 2> merge_bound;
	merge_bound[0] = glm::vec3(std::numeric_limits<float>::max());
	merge_bound[1] = glm::vec3(std::numeric_limits<float>::lowest());

	for (MeshShape& shape : m_meshesShapes)
		getBoundingBox(shape.geometry.vertices, shape.dim, shape.center, merge_bound);

	merge_dim.x = (merge_bound[1].x - merge_bound[0].x) / 2.0f;
	merge_dim.y = (merge_bound[1].y - merge_bound[0].y) / 2.0f;
	merge_dim.z = (merge_bound[1].z - merge_bound[0].z) / 2.0f;

	merge_center.x = (merge_bound[1].x + merge_bound[0].x) / 2.0f;
	merge_center.y = (merge_bound[1].y + merge_bound[0].y) / 2.0f;
	merge_center.z = (merge_bound[1].z + merge_bound[0].z) / 2.0f;
}

float IMeshReader::getScale() const
{
	return m_scale;
}

void IMeshReader::getCorrectedVertices(std::vector<float>& buffer, const glm::vec3& translation) const
{
	assert(buffer.size() % 3 == 0);
	for (uint64_t iterator(0); iterator < buffer.size();)
	{
		buffer[iterator] -= translation.x;
		buffer[iterator + 1] -= translation.y;
		buffer[iterator + 2] -= translation.z;
		iterator += 3;
	}
}

void IMeshReader::getBoundingBox(const std::vector<float>& vertices, glm::vec3& dim, glm::vec3& center, std::array<glm::vec3, 2>& merge_bound)
{
	if (vertices.empty())
	{
		dim = glm::vec3(0.f);
		center = glm::vec3(0.f);
		return;
	}

	std::array<glm::vec3, 2> bound;
	bound[0] = glm::vec3(std::numeric_limits<float>::max());
	bound[1] = glm::vec3(std::numeric_limits<float>::lowest());

	// FIXME(robin) - Pas top comme boucle for. Je comprends que le vector de float arrive avec une taille multiple de 3 mais ce n’est pas une garantie suffisante pour moi. Au minimum il faudrait `vertices.size() - 2` comme critère de fin.
	assert((vertices.size() % 3) == 0);
	for (uint64_t iterator(0); iterator < vertices.size() - 2; iterator += 3)
	{
		glm::vec3 vertex(vertices[iterator],
			vertices[iterator + 1],
			vertices[iterator + 2]);
		checkBoundingBox(vertex, bound);
		checkBoundingBox(vertex, merge_bound);
	}

	dim.x = (bound[1].x - bound[0].x) / 2.0f;
	dim.y = (bound[1].y - bound[0].y) / 2.0f;
	dim.z = (bound[1].z - bound[0].z) / 2.0f;

	center.x = (bound[1].x + bound[0].x) / 2.0f;
	center.y = (bound[1].y + bound[0].y) / 2.0f;
	center.z = (bound[1].z + bound[0].z) / 2.0f;
}


void IMeshReader::checkBoundingBox(const glm::vec3& vertex, std::array<glm::vec3, 2>& dim)
{
	if (vertex.x < dim[0].x)
		dim[0].x = vertex.x;
	if (vertex.y < dim[0].y)
		dim[0].y = vertex.y;
	if (vertex.z < dim[0].z)
		dim[0].z = vertex.z;

	if (vertex.x > dim[1].x)
		dim[1].x = vertex.x;
	if (vertex.y > dim[1].y)
		dim[1].y = vertex.y;
	if (vertex.z > dim[1].z)
		dim[1].z = vertex.z;
}

ObjectAllocation::ReturnCode IMeshReader::virtualAllocateMeshBuffer(MeshBuffer& _mesh, const MeshShape& shape)
{
	return allocateMesh(&_mesh, shape.geometry);
}

ObjectAllocation::ReturnCode IMeshReader::allocateMesh(MeshBuffer* _mesh, const MeshGeometries& geometrie)
{
	bool res(true);
	if (_mesh == nullptr)
		return ObjectAllocation::ReturnCode::Failed;
	VulkanManager& vkm = VulkanManager::getInstance();

	std::vector<IndexedDraw> polyligneDrawList;
	std::vector<uint32_t> polylignes;
	//uint32_t polyligneCumulatedSize = 0;
	for (auto polyligne : geometrie.polyligneIndices)
	{
		polyligneDrawList.push_back({ 0, (uint32_t)geometrie.indices.size() + (uint32_t)geometrie.edgesIndices.size() + (uint32_t)polylignes.size(), (uint32_t)polyligne.size() });
		polylignes.insert(polylignes.end(), polyligne.begin(), polyligne.end());
	}

	VkDeviceSize size = (geometrie.vertices.size() + geometrie.normals.size() + geometrie.texcoords.size()) * sizeof(float) + (geometrie.indices.size() + geometrie.edgesIndices.size() + polylignes.size()) * sizeof(uint32_t);

	IOLOG << "Allocating simple buffer: " << size << LOGENDL;

	SimpleBuffer& sbuf = _mesh->m_smpBuffer;

	VkResult err = vkm.allocSimpleBuffer(size, sbuf, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	if (err != VK_SUCCESS)
	{
		VulkanManager::getInstance().freeAllocation(sbuf);
		IOLOG << "Error failed to allocate simple buffer!" << LOGENDL;
		return ObjectAllocation::getVulkanReturnCode(err);
	}

	_mesh->m_vertexBufferOffset = 0; // Always 0, not saved
	res &= vkm.loadInSimpleBuffer(sbuf, geometrie.vertices.size() * sizeof(float), geometrie.vertices.data(), _mesh->m_vertexBufferOffset, 4);

	_mesh->m_normalBufferOffset = _mesh->m_vertexBufferOffset + geometrie.vertices.size() * sizeof(float);
	if (!geometrie.normals.empty())
		res &= vkm.loadInSimpleBuffer(sbuf, geometrie.normals.size() * sizeof(float), geometrie.normals.data(), _mesh->m_normalBufferOffset, 4);

	// NOTE(robin) - On n’utilise pas de texture pour le moment

	if (!geometrie.indices.empty() || !geometrie.edgesIndices.empty() || !geometrie.polyligneIndices.empty())
	{
		_mesh->m_indexBufferOffset = _mesh->m_normalBufferOffset + geometrie.normals.size() * sizeof(float);
		if (!geometrie.indices.empty())
		{
			_mesh->addIndexedDraw(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, { 0, 0, (uint32_t)geometrie.indices.size() });
			res &= vkm.loadInSimpleBuffer(sbuf, geometrie.indices.size() * sizeof(uint32_t), geometrie.indices.data(), _mesh->m_indexBufferOffset, 4);
		}

		VkDeviceSize edgeIndicesOffset =_mesh->m_indexBufferOffset + geometrie.indices.size() * sizeof(uint32_t);
		if (!geometrie.edgesIndices.empty())
		{
			_mesh->addIndexedDraw(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, { 0, (uint32_t)geometrie.indices.size(), (uint32_t)geometrie.edgesIndices.size() });
			res &= vkm.loadInSimpleBuffer(sbuf, geometrie.edgesIndices.size() * sizeof(uint32_t), geometrie.edgesIndices.data(), edgeIndicesOffset, 4);
		}

		VkDeviceSize polyligneIndicesOffset = edgeIndicesOffset + geometrie.edgesIndices.size() * sizeof(uint32_t);
		if (!geometrie.polyligneIndices.empty())
		{
			_mesh->addIndexedDraws(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, polyligneDrawList);
			res &= vkm.loadInSimpleBuffer(sbuf, polylignes.size() * sizeof(uint32_t), polylignes.data(), polyligneIndicesOffset, 4);
		}
	}
	else
		_mesh->m_indexBufferOffset = 0;

	// Manière choisi pour indiquer qu'il n'y a pas de normales.
	if (geometrie.normals.size() == 0)
		_mesh->m_normalBufferOffset = 0;

	if (!res)
	{
		VulkanManager::getInstance().freeAllocation(sbuf);
		return ObjectAllocation::ReturnCode::MeshAllocFail;
	}

	return ObjectAllocation::ReturnCode::Success;
}
