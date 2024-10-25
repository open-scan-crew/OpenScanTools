#include "controller/functionSystem/ContextExportObj.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/Texts.hpp"
#include "gui/GuiData/GuiDataIO.h"
#include "controller/messages/DataIdListMessage.h"
#include "controller/messages/FilesMessage.h"
#include "controller/messages/PrimitivesExportParametersMessage.h"
#include "utils/Utils.h"
#include "utils/OpenScanToolsVersion.h"
#include "models/graph/AMeasureNode.h"
#include "models/graph/MeshObjectNode.h"
#include "models/graph/TorusNode.h"


#include "vulkan/MeshManager.h"

ContextExportObj::ContextExportObj(const ContextId& id)
	: AContext(id)
{
	m_state = ContextState::waiting_for_input;
}

ContextExportObj::~ContextExportObj()
{}

ContextState ContextExportObj::start(Controller& controller)
{
	return m_state;
}

ContextState ContextExportObj::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
	case IMessage::MessageType::FILES:
		m_output = *(static_cast<FilesMessage*>(message)->m_inputFiles.begin());
		break;
	case IMessage::MessageType::DATAID_LIST:
		m_listToExport = static_cast<DataListMessage*>(message)->m_dataPtrs;
		if (m_listToExport.empty())
			return m_state = ContextState::abort;
		break;
	case IMessage::MessageType::PRIMITIVES_EXPORT_PARAMETERS:
		m_primitiveExportParam = static_cast<PrimitivesExportParametersMessage*>(message)->m_parameters;
		break;
	}

	m_state = (m_listToExport.empty() || m_output.empty()) ? ContextState::waiting_for_input : ContextState::ready_for_using;
	return m_state;
}

ContextState ContextExportObj::launch(Controller& controller)
{
	m_state = ContextState::running;

	MeshManager& meshManager = MeshManager::getInstance();

	if (m_output.empty())
		return processError(controller);

	if (m_output.filename().wstring().find_first_of(L".") == std::wstring::npos)
		m_output += ".obj";

	std::ofstream stream;
	stream.open(m_output);
	if(!stream.good())
		return processError(controller);

	stream << "# From OpenScanTools v" << OPENSCANTOOLS_VERSION << " - positions in meters" << std::endl;
	stream << std::endl;

	uint32_t indOffset = 1;

	for (const SafePtr<AGraphNode>& data : m_listToExport)
	{
		ElementType type;
		glm::dmat4 transfo = glm::dmat4();
		std::wstring name;
		{
			ReadPtr<AGraphNode> rData = data.cget();
			if (!rData)
				continue;
			type = rData->getType();
			transfo = rData->getTransformation();
			name = rData->getName();
		}

		RawMeshData meshData;

		std::vector<Measure> measures = {};

		switch (type)
		{
			case ElementType::Cylinder:
				{
					meshData = meshManager.generateCylinder(48);
				}
				break;
			case ElementType::Sphere:
				{
					meshData = meshManager.generateSphere(48, 48);
				}
				break;
			case ElementType::Box:
			case ElementType::Grid:
				{
					meshData = meshManager.generateBox();
				}
				break;
			case ElementType::Torus:
				{
					ReadPtr<TorusNode> rTorus = static_pointer_cast<TorusNode>(data).cget();
					if (!rTorus)
						continue;
					float outerRadius = rTorus->getMainRadius() + rTorus->getAdjustedTubeRadius();
					float innerRadius = rTorus->getMainRadius() - rTorus->getAdjustedTubeRadius();
					meshData = meshManager.generateTorusPart(48, rTorus->getMainAngle(), innerRadius/outerRadius);
				}
				break;
			case ElementType::MeshObject:
				{
					ReadPtr<MeshObjectNode> rMesh = static_pointer_cast<MeshObjectNode>(data).cget();
					if (!rMesh)
						continue;

					std::shared_ptr<MeshBuffer> buffer = meshManager.getMesh(rMesh->getMeshId()).m_mesh;
					meshBufferToRawMeshData(buffer, meshData);
				}
				break;
			case ElementType::PipeToPipeMeasure:
			case ElementType::PointToPipeMeasure:
			case ElementType::PointToPlaneMeasure:
			case ElementType::SimpleMeasure:
			case ElementType::PolylineMeasure:
			{
				ReadPtr<AMeasureNode> rMeasures = static_pointer_cast<AMeasureNode>(data).cget();
				if (!rMeasures)
					continue;
				measures = rMeasures->getMeasures();
			}
			break;
			default:
				continue;
		}

		glm::dvec3 decalExport = m_primitiveExportParam.exportWithScanImportTranslation ? -controller.getContext().getProjectInfo().m_importScanTranslation : glm::dvec3(0);
		glm::dmat4 decalMat = { 1.0f, 0.f, 0.f, 0.f,
							  0.f, 1.0f, 0.f, 0.f,
							  0.f, 0.f, 1.0f, 0.f,
							  decalExport.x, decalExport.y, decalExport.z, 1.f };

		transfo = decalMat * transfo;

		if (meshData.vertices.empty() && measures.empty())
			continue;

		stream << "o " << Utils::to_utf8(name) << std::endl;
		stream << std::endl;

		if (!meshData.vertices.empty())
		{
			for (const glm::vec3& point : meshData.vertices)
				addPoint(stream, transfo * glm::vec4(point, 1.0f));

			stream << std::endl;
			stream << "s 1" << std::endl;
			stream << std::endl;

			std::vector<uint32_t> indexes;
			getTrianglesList(meshData, indexes);
			assert((indexes.size() % 3) == 0);

			for (int i = 0; i + 2 < indexes.size(); i += 3)
				stream << "f " << indOffset + indexes[i] << " " << indOffset + indexes[i + 1] << " " << indOffset + indexes[i + 2] << std::endl;

			indOffset += (uint32_t)meshData.vertices.size();

			stream << std::endl;
		}
		else if (!measures.empty())
		{
			if (type != ElementType::PolylineMeasure)
			{
				for (const Measure& measure : measures)
				{
					addPoint(stream, measure.origin + decalExport);
					addPoint(stream, measure.final + decalExport);
				}
				stream << std::endl;

				for (int i = 0; i < measures.size(); i++)
				{
					stream << "l " << indOffset << " " << indOffset + 1 << std::endl;
					indOffset += 2;
				}

			}
			else
			{
				addPoint(stream, measures[0].origin);
				for (const Measure& measure : measures)
					addPoint(stream, measure.final);

				stream << std::endl;

				stream << "l " << indOffset << " ";
				for (int i = 0; i < measures.size(); i++)
					stream << indOffset + i + 1 << " ";
				stream << std::endl;

				indOffset += (uint32_t)measures.size() + 1;
			}
		}
	}

	stream.flush();
	stream.close();

	if (m_primitiveExportParam.openFolderWindowsAfterExport)
		controller.updateInfo(new GuiDataOpenInExplorer(m_output));
	return (m_state = ContextState::done);
}

bool ContextExportObj::canAutoRelaunch() const
{
	return (false);
}

ContextType ContextExportObj::getType() const
{
	return (ContextType::exportObj);
}

ContextState ContextExportObj::processError(Controller& controller)
{
	controller.updateInfo(new GuiDataWarning(TEXT_WRITE_FAILED_PERMISSION));
	return (m_state = ContextState::abort);
}

void ContextExportObj::meshBufferToRawMeshData(const std::shared_ptr<MeshBuffer>& meshBuffer, RawMeshData& meshData)
{
	if (meshBuffer == nullptr)
		return;

	size_t bufferSize = meshBuffer->getSimpleBuffer().size;
	void* bufferData = new char[bufferSize];
	memset(bufferData, 0, bufferSize);

	VulkanManager::getInstance().downloadSimpleBuffer(meshBuffer->getSimpleBuffer(), bufferData, bufferSize);

	glm::vec3* pointsData = static_cast<glm::vec3*>(bufferData);
	size_t vCount = meshBuffer->getVertexCount();
	meshData.vertices.reserve(vCount);
	for (size_t i = 0; i < vCount; ++i)
		meshData.vertices.emplace_back(glm::vec4(pointsData[i], 1.f));

	getRawIndicesData(bufferData, meshBuffer, meshData.indices);
	delete[] bufferData;

	meshData.indexedDraws = meshBuffer->getIndexedDrawList();
	meshData.standardDraws = meshBuffer->getDrawList();
}

void ContextExportObj::getRawIndicesData(void* bufferData, const std::shared_ptr<MeshBuffer>& meshBuffer, std::vector<uint32_t>& indices)
{
	uint32_t indicesCount = 0;
	for (auto topoIDraw : meshBuffer->getIndexedDrawList())
	{
		for (const IndexedDraw& idd : topoIDraw.second)
			indicesCount += idd.indexCount;
	}
	indices.reserve(indicesCount);

	uint32_t* inds = static_cast<uint32_t*>((void*)((char*)bufferData + meshBuffer->getIndexOffset()));

	indices.insert(indices.begin(), inds, inds + indicesCount);
}

void ContextExportObj::getTrianglesList(const RawMeshData& meshData, std::vector<uint32_t>& indexes)
{
	uint32_t triangleCount = 0;
	for (auto topoDraw : meshData.standardDraws)
	{
		for (const StandardDraw& stdDraw : topoDraw.second)
		{
			if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
				triangleCount += stdDraw.vertexCount / 3;
			if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP ||
				topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
				triangleCount += stdDraw.vertexCount - 2;
		}
	}

	for (auto topoIDraw : meshData.indexedDraws)
	{
		for (const IndexedDraw& idd : topoIDraw.second)
		{
			if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
				triangleCount += idd.indexCount / 3;
			if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP ||
				topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
				triangleCount += idd.indexCount - 2;
		}
	}
	indexes.reserve(triangleCount);


	for (auto topoDraw : meshData.standardDraws)
	{
		for (const StandardDraw& stdDraw : topoDraw.second)
		{
			if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			{
				for (uint32_t i = stdDraw.firstVertex; i < stdDraw.firstVertex + stdDraw.vertexCount - 2; i += 3)
				{
					indexes.push_back(i);
					indexes.push_back(i + 1);
					indexes.push_back(i + 2);
				}
			}
			else if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
			{
				for (uint32_t i = stdDraw.firstVertex; i < stdDraw.firstVertex + stdDraw.vertexCount - 2; ++i)
				{
					indexes.push_back(i);
					indexes.push_back(i + (1 + i % 2));
					indexes.push_back(i + (2 - i % 2));
				}
			}
			else if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
			{
				for (uint32_t i = stdDraw.firstVertex; i < stdDraw.firstVertex + stdDraw.vertexCount - 2; ++i)
				{
					indexes.push_back(i + 1);
					indexes.push_back(i + 2);
					indexes.push_back(stdDraw.firstVertex);
				}
			}
		}
	}

	const std::vector<uint32_t>& inds = meshData.indices;

	for (auto topoIDraw : meshData.indexedDraws)
	{
		for (const IndexedDraw& idd : topoIDraw.second)
		{
			if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			{
				for (uint32_t i = idd.firstIndex; i < idd.firstIndex + idd.indexCount - 2; i += 3)
				{
					indexes.push_back(idd.vertexOffset + inds[i]);
					indexes.push_back(idd.vertexOffset + inds[i + 1]);
					indexes.push_back(idd.vertexOffset + inds[i + 2]);
				}
			}
			else if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
			{
				for (uint32_t i = idd.firstIndex; i < idd.firstIndex + idd.indexCount - 2; ++i)
				{
					indexes.push_back(idd.vertexOffset + inds[i]);
					indexes.push_back(idd.vertexOffset + inds[i + (1 + i % 2)]);
					indexes.push_back(idd.vertexOffset + inds[i + (2 - i % 2)]);
				}
			}
			else if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
			{
				for (uint32_t i = idd.firstIndex; i < idd.firstIndex + idd.indexCount - 2; ++i)
				{
					indexes.push_back(idd.vertexOffset + inds[i + 1]);
					indexes.push_back(idd.vertexOffset + inds[i + 2]);
					indexes.push_back(idd.vertexOffset + inds[0]);
				}
			}
		}
	}
}

void ContextExportObj::addPoint(std::ofstream& wstream, const glm::dvec3& point)
{
	wstream << "v " << Utils::roundFloat(point.x) << " " << Utils::roundFloat(point.y) << " " << Utils::roundFloat(point.z) << std::endl;
}
