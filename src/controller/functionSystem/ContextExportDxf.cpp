#include "controller/functionSystem/ContextExportDxf.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ErrorMessagesTexts.hpp"
#include "gui/GuiData/GuiDataIO.h"
#include "controller/messages/DataIdListMessage.h"
#include "controller/messages/FilesMessage.h"
#include "controller/messages/PrimitivesExportParametersMessage.h"
#include "utils/Utils.h"
#include "magic_enum/magic_enum.hpp"
#include "models/graph/AMeasureNode.h"
#include "models/graph/BeamBendingMeasureNode.h"
#include "models/graph/ColumnTiltMeasureNode.h"
#include "models/graph/CylinderNode.h"
#include "models/graph/SphereNode.h"
#include "models/graph/TorusNode.h"
#include "models/3d/GeometryGenerator.h"

#define TAG "Tag"
#define MEASURE "Measure"
#define CLIPPING "Clipping"
#define SCAN "Scanning_position"
#define POINT "Point"
#define PIPE "Pipe"
#define SPHERE "Sphere"
#define TORUS "Torus"
#define RADIUS_TEXT  0.125f
#define DECIMAL 4

ContextExportDxf::ContextExportDxf(const ContextId& id)
	: AContext(id)
{
	m_state = ContextState::waiting_for_input;
}

ContextExportDxf::~ContextExportDxf()
{}

ContextState ContextExportDxf::start(Controller& controller)
{
	return m_state;
}

ContextState ContextExportDxf::feedMessage(IMessage* message, Controller& controller)
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

ContextState ContextExportDxf::launch(Controller& controller)
{
	m_state = ContextState::running;
	ErrorType err(ErrorType::Success);

	if (m_output.empty())
		return processError(err, controller);
	if (m_output.filename().wstring().find_first_of(L".") == std::wstring::npos)
		m_output += ".dxf";
	if(begin(m_output))
		return processError(ErrorType::FailedToOpen, controller);

	for (const SafePtr<AGraphNode>& exportPtr : m_listToExport)
	{
		ElementType type;
		{
			ReadPtr<AGraphNode> readNode = exportPtr.cget();
			if (!readNode)
				continue;
			type = readNode->getType();
		}

		std::wstring text;
		glm::vec3 position;
		std::vector<Measure> measures;
		switch (type)
		{
			case ElementType::Scan:
				{
					ReadPtr<AObjectNode> data = static_pointer_cast<AObjectNode>(exportPtr).cget();
					if (!data)
						continue;
					text = data->getName();
					position = data->getCenter();
				}
				break;
			case ElementType::Tag: 
				{
					ReadPtr<AObjectNode> tag = static_pointer_cast<AObjectNode>(exportPtr).cget();
					if (!tag)
						continue;
					text = Utils::wCompleteWithZeros(tag->getUserIndex());
					if (!tag->getIdentifier().empty())
						text += L"-" + tag->getIdentifier();
					if (!tag->getName().empty())
						text += L"-" + tag->getName();
					position = tag->getCenter();
				}
				break;
			case ElementType::Point:
				{
					ReadPtr<AObjectNode> data = static_pointer_cast<AObjectNode>(exportPtr).cget();
					if (!data)
						continue;
					text = Utils::wCompleteWithZeros(data->getUserIndex());
					if (!data->getName().empty())
						text += L"-" + data->getName();
					position = data->getCenter();
				}
				break;
			case ElementType::BeamBendingMeasure:
				{
					ReadPtr<BeamBendingMeasureNode> bbm = static_pointer_cast<BeamBendingMeasureNode>(exportPtr).cget();
					if (!bbm)
						continue;
					text = Utils::wCompleteWithZeros(bbm->getUserIndex()) + L"-" + bbm->getName() + L"-bending-" + Utils::wRoundFloat(bbm->getBendingValue()) + L"-ratio-" + Utils::wRoundFloat(bbm->getRatio(),5) + L"-reliable-" + Utils::from_utf8(std::string(magic_enum::enum_name(bbm->getRatioSup())));
					if(bbm->getRatio() != 0.f)
						text += Utils::wRoundFloat(bbm->getRatio(),5) + L"-yes";
					else
						text += L"-no";
					position = bbm->getMaxBendingPos();
				}
				break;
			case ElementType::ColumnTiltMeasure: 
				{
					ReadPtr<ColumnTiltMeasureNode> ctm = static_pointer_cast<ColumnTiltMeasureNode>(exportPtr).cget();
					if (!ctm)
						continue;
					text = Utils::wCompleteWithZeros(ctm->getUserIndex()) + L"-" + ctm->getName() + L"-tilt-" + Utils::wRoundFloat(ctm->getTiltValue()) + L"-ratio-" +Utils::wRoundFloat(ctm->getRatio(), 5) + L"-reliable-" + Utils::from_utf8(std::string(magic_enum::enum_name(ctm->getRatioSup())));
					position = ctm->getTopPoint();
				}
				break;
			case ElementType::Cylinder:
				{
					ReadPtr<CylinderNode> cyl = static_pointer_cast<CylinderNode>(exportPtr).cget();
					if (!cyl)
						continue;
					text = Utils::wCompleteWithZeros(cyl->getUserIndex()) + L"-" + cyl->getIdentifier() + L"-" + cyl->getName() + L"-" + Utils::wRoundFloat(cyl->getRadius() * 2.0, DECIMAL);
				}
				break;
			case ElementType::Torus:
				{
					ReadPtr<TorusNode> torus = static_pointer_cast<TorusNode>(exportPtr).cget();
					if (!torus)
						continue;
					text = Utils::wCompleteWithZeros(torus->getUserIndex()) + L"-" + torus->getIdentifier() + L"-" + torus->getName() + L"-" + Utils::wRoundFloat(torus->getAdjustedTubeRadius() * 2.0, DECIMAL);
				}
				break;
			case ElementType::Sphere:
				{
					ReadPtr<SphereNode> sphere = static_pointer_cast<SphereNode>(exportPtr).cget();
					if (!sphere)
						continue;
					text = Utils::wCompleteWithZeros(sphere->getUserIndex()) + L"-" + sphere->getIdentifier() + L"-" + sphere->getName() + L"-" + Utils::wRoundFloat(sphere->getRadius() * 2.0, DECIMAL);
				}
				break;
			case ElementType::Box:
			case ElementType::Grid:
			case ElementType::PipeToPipeMeasure:
			case ElementType::PipeToPlaneMeasure:
			case ElementType::PointToPipeMeasure:
			case ElementType::PointToPlaneMeasure:
			case ElementType::SimpleMeasure:
			case ElementType::PolylineMeasure:
				{
					ReadPtr<AObjectNode> obj = static_pointer_cast<AObjectNode>(exportPtr).cget();
					if (!obj)
						continue;
					text = Utils::wCompleteWithZeros(obj->getUserIndex()) + L"-" + obj->getName();
				}
				break;
		}		

		std::vector<glm::vec3> vertices;
		std::vector<uint32_t> edgesIndices;
		glm::mat4 model;

		switch (type)
		{
			case ElementType::Box:
			case ElementType::Grid:
				{
					ReadPtr<TransformationModule> transfo = static_pointer_cast<TransformationModule>(exportPtr).cget();
					if (!transfo)
						continue;
					GeometryGenerator::generateBoxWire(vertices, edgesIndices);
					model = transfo->getTransformation();
				}
				break;
			case ElementType::Cylinder:
				{
					ReadPtr<CylinderNode> rCylinder = static_pointer_cast<CylinderNode>(exportPtr).cget();
					if (!rCylinder)
						continue;
					GeometryGenerator::generateCylinderWire(36u, vertices, edgesIndices);
					model = rCylinder->getTransformation();
				}
				break;
			case ElementType::Torus:
			{
				ReadPtr<TorusNode> torus = static_pointer_cast<TorusNode>(exportPtr).cget();
				if (!torus)
					continue;
				GeometryGenerator::generateTorusWire((float)(torus->getMainAngle()), (float)(torus->getMainRadius()), (float)(torus->getAdjustedTubeRadius()), 36u, vertices, edgesIndices);
				
				glm::dmat4 translation = { 1.0f, 0.f, 0.f, 0.f,
							  0.f, 1.0f, 0.f, 0.f,
							  0.f, 0.f, 1.0f, 0.f,
							  torus->getCenter().x, torus->getCenter().y, torus->getCenter().z, 1.f };

				// Rotation by quaternion
				glm::dmat4 rotMat = glm::mat4_cast(torus->getOrientation());
				
				model = (translation * rotMat);
			}
			break;
			case ElementType::Sphere:
				{
					ReadPtr<SphereNode> rSphere = static_pointer_cast<SphereNode>(exportPtr).cget();
					if (!rSphere)
						continue;
					GeometryGenerator::generateSphereWire(16u, 16u, vertices, edgesIndices);
					model = rSphere->getTransformation();
				}
				break;
			case ElementType::PipeToPipeMeasure:
			case ElementType::PipeToPlaneMeasure:
			case ElementType::PointToPipeMeasure:
			case ElementType::PointToPlaneMeasure:
			case ElementType::SimpleMeasure:
			case ElementType::PolylineMeasure:
				{
					ReadPtr<AMeasureNode> measuresNode = static_pointer_cast<AMeasureNode>(exportPtr).cget();
					if (!measuresNode)
						continue;
					measures = measuresNode->getMeasures();
				}
				break;
		}

		glm::dvec3 decalExport = m_primitiveExportParam.exportWithScanImportTranslation ? -controller.getContext().getProjectInfo().m_importScanTranslation : glm::dvec3(0);
		position += decalExport;
		glm::mat4 decalMat = { 1.0f, 0.f, 0.f, 0.f,
							  0.f, 1.0f, 0.f, 0.f,
							  0.f, 0.f, 1.0f, 0.f,
							  decalExport.x, decalExport.y, decalExport.z, 1.f };

		model = decalMat * model;

		switch (type)
		{
			case ElementType::Scan:
			case ElementType::BeamBendingMeasure:
			case ElementType::ColumnTiltMeasure:
			case ElementType::Point:
			case ElementType::Tag:
				if ((err = exportMarker(m_output, position, text)) != ErrorType::Success)
					return processError(err, controller);
				break;
			case ElementType::PipeToPipeMeasure:
			case ElementType::PipeToPlaneMeasure:
			case ElementType::PointToPipeMeasure:
			case ElementType::PointToPlaneMeasure:
			case ElementType::SimpleMeasure:
			case ElementType::PolylineMeasure:
				if ((err = exportLines(m_output, measures, decalExport, text)) != ErrorType::Success)
					return processError(err, controller);
				break;
			case ElementType::Box:
			case ElementType::Grid:
			case ElementType::Cylinder:
			case ElementType::Sphere:
			case ElementType::Torus:
				if ((err = exportWireframeObject(m_output, model, vertices, edgesIndices, text)) != ErrorType::Success)
					return processError(err, controller);
				break;
			break;
		}
	}
	if (!end())
		return processError(ErrorType::FailedToWrite, controller);

	if (m_primitiveExportParam.openFolderWindowsAfterExport)
		controller.updateInfo(new GuiDataOpenInExplorer(m_output));
	return (m_state = ContextState::done);
}

bool ContextExportDxf::canAutoRelaunch() const
{
	return (false);
}

ContextType ContextExportDxf::getType() const
{
	return (ContextType::exportDxf);
}

ContextState ContextExportDxf::processError(const ContextExportDxf::ErrorType& error, Controller& controller)
{
	controller.updateInfo(new GuiDataWarning(TEXT_WRITE_FAILED_PERMISSION));
	return (m_state = ContextState::abort);
}

ContextExportDxf::ErrorType ContextExportDxf::getFilename(const ElementType& type, std::filesystem::path& output) const
{
	std::string name;
	switch (type)
	{
		case ElementType::Scan:
			name += SCAN;
			break;
		case ElementType::BeamBendingMeasure:
		case ElementType::ColumnTiltMeasure:
		case ElementType::PipeToPipeMeasure:
		case ElementType::PipeToPlaneMeasure:
		case ElementType::PointToPipeMeasure:
		case ElementType::PointToPlaneMeasure:
		case ElementType::SimpleMeasure:
		case ElementType::PolylineMeasure:
			name += MEASURE;
			break;
		case ElementType::Point:
			name += POINT;
			break;
		case ElementType::Tag:
			name += TAG;
			break;
		case ElementType::Box:
		case ElementType::Grid:
			name += CLIPPING;
			break;
		case ElementType::Cylinder:
			name += PIPE;
			break;
		case ElementType::Sphere:
			name += SPHERE;
			break;
		case ElementType::Torus:
			name += TORUS;
			break;
		case ElementType::None:
			name = "All";
			break;
		default:
			return ErrorType::WrongType;
	}
	std::time_t result = std::time(nullptr);
	std::tm* time = localtime(&result);
	name += "-" + std::to_string(1900 + time->tm_year) + "-" + Utils::completeWithZeros(1 + time->tm_mon, 2) + "-" + Utils::completeWithZeros(time->tm_mday, 2) + "_" + std::to_string(time->tm_hour) + "-" + Utils::completeWithZeros(time->tm_min, 2) + "-" + Utils::completeWithZeros(time->tm_sec, 2) + ".dxf";
	output /= name;
	return ErrorType::Success;
}

ContextExportDxf::ErrorType ContextExportDxf::exportMarker(const std::filesystem::path& output, const glm::vec3& center, const std::wstring& text)
{
	if(drawTarget(center, 0))
		return ErrorType::FailedToWrite;
	if (this->text(text, 0, center, RADIUS_TEXT))
		return ErrorType::FailedToWrite;
	return ErrorType::Success;
}

ContextExportDxf::ErrorType ContextExportDxf::exportLines(const std::filesystem::path& output, const std::vector<Measure>& lines, const glm::dvec3& decalExport, const std::wstring& text)
{
	for (const Measure& measure : lines)
	{
		if (drawLine(measure.origin - decalExport, measure.final - decalExport, 0))
			return ErrorType::FailedToWrite;
	}
	if (this->text(text, 0, lines.begin()->origin - decalExport, RADIUS_TEXT))
		return ErrorType::FailedToWrite;
	return ErrorType::Success;
}

ContextExportDxf::ErrorType ContextExportDxf::exportWireframeObject(const std::filesystem::path& output, const glm::mat4& model, const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& edgesIndices, const std::wstring& text)
{
	for (uint64_t iterator(0); iterator < edgesIndices.size(); iterator += 2)
		if (drawLine(model * glm::vec4(vertices[edgesIndices[iterator]], 1.0f), model * glm::vec4(vertices[edgesIndices[iterator + 1]], 1.0f), 0))
			return ErrorType::FailedToWrite;
	
	if (this->text(text, 0, model * glm::vec4(vertices[0], 1.0f), RADIUS_TEXT))
		return ErrorType::FailedToWrite;
	return ErrorType::Success;
}