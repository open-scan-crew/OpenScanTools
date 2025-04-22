#include "controller/functionSystem/ContextExportStep.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"

#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ErrorMessagesTexts.hpp"
#include "gui/texts/TreePanelTexts.hpp"
#include "gui/GuiData/GuiDataIO.h"

#include "controller/messages/DataIdListMessage.h"
#include "controller/messages/FilesMessage.h"
#include "controller/messages/PrimitivesExportParametersMessage.h"
#include "models/graph/AMeasureNode.h"
#include "models/graph/CylinderNode.h"
#include "models/graph/SphereNode.h"
#include "models/graph/TorusNode.h"

#include "models/application/Author.h"

#include "utils/Logger.h"

#include <glm/gtx/quaternion.hpp>

#define IOLOG Logger::log(LoggerMode::IOLog)

ContextExportStep::ContextExportStep(const ContextId& id)
	: AContext(id)
{
	m_state = ContextState::waiting_for_input;
}

ContextExportStep::~ContextExportStep()
{}

ContextState ContextExportStep::start(Controller& controller)
{
	return m_state;
}

ContextState ContextExportStep::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
		case IMessage::MessageType::PRIMITIVES_EXPORT_PARAMETERS:
			m_parameters = static_cast<PrimitivesExportParametersMessage*>(message)->m_parameters;
			m_state = m_listToExport.empty() || m_output.empty() ? ContextState::waiting_for_input : ContextState::ready_for_using;
			break;
		case IMessage::MessageType::FILES:
			m_output = *(static_cast<FilesMessage*>(message)->m_inputFiles.begin());
			m_state = m_listToExport.empty() ? ContextState::waiting_for_input : ContextState::ready_for_using;
			break;
		case IMessage::MessageType::DATAID_LIST:
			m_listToExport = static_cast<DataListMessage*>(message)->m_dataPtrs;
			if (m_listToExport.empty())
				return m_state = ContextState::abort;
			m_state = m_output.empty() ? ContextState::waiting_for_input : ContextState::ready_for_using;
			break;
	}
	return m_state;
}

ContextState ContextExportStep::launch(Controller& controller)
{
	IOLOG << "ContextExportStep launch " << LOGENDL;

	m_state = ContextState::running;
	ErrorType err(ErrorType::Success);


	if (m_output.empty())
		return processError(err, controller);
	if (m_output.filename().wstring().find_first_of(L".") == std::wstring::npos)
		m_output += ".step";

	IOLOG << "ContextExportStep launch 1" << LOGENDL;

	setRootName(m_output.filename().stem().string());

	IOLOG << "ContextExportStep launch 2" << LOGENDL;

	std::vector<SafePtr<AGraphNode>> listToExport;
	for (const SafePtr<AGraphNode>& exportObj : m_listToExport)
	{
		ReadPtr<AGraphNode> data = exportObj.cget();
		if (!data)
			continue;
		listToExport.push_back(exportObj);
	}

	if (listToExport.empty())
		return (m_state = ContextState::abort);

	std::sort(listToExport.begin(), listToExport.end(), [](SafePtr<AGraphNode> a, SafePtr<AGraphNode> b)
		{ 
			ReadPtr<AGraphNode> readA = a.cget();
			ReadPtr<AGraphNode> readB = b.cget();
			return ((readA->getUserIndex() < readB->getUserIndex()));
		});

	IOLOG << "ContextExportStep launch 3 -- exportSize" << listToExport.size() << LOGENDL;

	std::map<xg::Guid, primitiveId> objToPrim;

	for (const SafePtr<AGraphNode>& exportObj : listToExport)
	{
		IOLOG << "ContextExportStep launch 3.5 " << LOGENDL;
		primitiveId primId;

		ElementType type;
		glm::dvec3 position;
		{
			ReadPtr<AObjectNode> readNode = static_pointer_cast<AObjectNode>(exportObj).cget();
			if (!readNode)
				continue;
			type = readNode->getType();
			position = readNode->getCenter();
		}

		glm::dvec3 decalExport = m_parameters.exportWithScanImportTranslation ? -controller.getContext().getProjectInfo().m_importScanTranslation : glm::dvec3(0);
		position += decalExport;

		switch (type)
			{
			case ElementType::Scan:
			{
				ReadPtr<AObjectNode> readNode = static_pointer_cast<AObjectNode>(exportObj).cget();
				if (!readNode)
					continue;
				primId = makeScan(position);
			}
			break;
			case ElementType::Tag:
			case ElementType::Point: 
			{
				ReadPtr<AObjectNode> readNode = static_pointer_cast<AObjectNode>(exportObj).cget();
				if (!readNode)
					continue;
				primId = makePoint(position);
			}
			break;
			/*case ElementType::BeamBendingMeasure: 
			{
				BeamBendingMeasure* bending = static_cast<BeamBendingMeasure*>(data);
				primId = makePoint(bending->getMaxBendingPos());
			}
			break;
			case ElementType::ColumnTiltMeasure:
			{
				ColumnTiltMeasure* columntTilt = static_cast<ColumnTiltMeasure*>(data);
				shape = makePoint(columntTilt->get);
				assert(!shape.IsNull());
			}
			break;*/
			case ElementType::Cylinder:
			{
				ReadPtr<CylinderNode> readCyl = static_pointer_cast<CylinderNode>(exportObj).cget();
				if (!readCyl)
					continue;
				addTypeDirectory(readCyl->getType());
				glm::dmat4 rotation(glm::toMat4(readCyl->getOrientation()));
				primId = makeCylinder(readCyl->getRadius(), readCyl->getLength(), position, rotation * glm::dvec4(0., 0., 1., 1.), readCyl->getOrientation());
				setDiameter(primId, 2.0 * readCyl->getRadius());

				objToPrim[readCyl->getId()] = primId;
			}
			break;
			case ElementType::Torus:
			{
				ReadPtr<TorusNode> readTor = static_pointer_cast<TorusNode>(exportObj).cget();
				if (!readTor)
					continue;
				addTypeDirectory(readTor->getType());

				primId = makeTorus(readTor->getMainRadius(), readTor->getAdjustedTubeRadius(), readTor->getMainAngle(), position, readTor->getOrientation());
				setDiameter(primId, 2.0 * readTor->getAdjustedTubeRadius());

				objToPrim[readTor->getId()] = primId;
			}
			break;
			case ElementType::Piping:
			{
				ReadPtr<AObjectNode> readNode = static_pointer_cast<AObjectNode>(exportObj).cget();
				if (!readNode)
					continue;
				addPipingDirectory(readNode->getId(), controller);
				continue;
			}
			break;
			case ElementType::Sphere:
			{
				ReadPtr<SphereNode> readSphere = static_pointer_cast<SphereNode>(exportObj).cget();
				if (!readSphere)
					continue;
				primId = makeSphere(readSphere->getRadius(), position);
				setDiameter(primId, 2.0 * readSphere->getRadius());
			}
			break;
			case ElementType::Box:
			{
				ReadPtr<AObjectNode> readNode = static_pointer_cast<AObjectNode>(exportObj).cget();
				if (!readNode)
					continue;
				primId = makeBox(readNode->getSize(), position, readNode->getOrientation());
			}
			break;
			case ElementType::PipeToPipeMeasure:
			case ElementType::PipeToPlaneMeasure:
			case ElementType::PointToPipeMeasure:
			case ElementType::PointToPlaneMeasure:
			case ElementType::SimpleMeasure:
			case ElementType::PolylineMeasure:
			{
				std::vector<glm::dvec3> points;
				{
					ReadPtr<AMeasureNode> readMeasures = static_pointer_cast<AMeasureNode>(exportObj).cget();
					if (!readMeasures || readMeasures->getMeasures().empty())
						continue;
					
					points.push_back(readMeasures->getMeasures()[0].origin - decalExport);
					for (const Measure& mesure : readMeasures->getMeasures()) 
						points.push_back(mesure.final - decalExport);
				}
				ReadPtr<AObjectNode> readObject = static_pointer_cast<AObjectNode>(exportObj).cget();
				if (!readObject)
					continue;
				primId = makeWire(points, position);
				
			}
			break;
			default:
				continue;
			break;
		}

		{
			ReadPtr<AObjectNode> readNode = static_pointer_cast<AObjectNode>(exportObj).cget();
			if (!readNode)
				continue;
			type = readNode->getType();

			addTypeDirectory(readNode->getType());

			ElementType directory;

			std::wstring dump;
			getTypeNameDirectory(readNode->getType(), directory, dump);

			setName(primId, readNode->getComposedName());

			setColor(primId, readNode->getColor());

			addShape(primId, m_typesDirectory[directory]);
		}
	}

	//IOLOG << "ContextExportStep launch 3 -- pipingSize : " << m_pipingDirectory.size() << LOGENDL;

	/*
	for (auto pipingDirectory : m_pipingDirectory) {
		const Piping* piping = pipingDirectory.first;

		/*for (xg::Guid childrenId : piping->getPipingList()) {
			const Data* children = project->cgetDataOnId(childrenId);
			if (children == nullptr)
				continue;

			if (std::find(m_listToExport.begin(), m_listToExport.end(), childrenId) == m_listToExport.end())
				continue;

			primitiveId childId = objToPrim[childrenId];

			setName(childId, children->getComposedName());
			setColor(childId, children->getColor());
			addShape(childId, pipingDirectory.second);
		}
	}
	*/

	IOLOG << "ContextExportStep launch 4" << LOGENDL;

	std::wstring authName = L"NO_AUTHOR";
	ReadPtr<Author> rAuth = controller.getContext().getActiveAuthor().cget();
	if (rAuth)
		authName = rAuth->getName();

	if (!write(m_output, authName, controller.getContext().cgetProjectInfo().m_company))
		return processError(ErrorType::FailedToWrite, controller);

	if (m_parameters.openFolderWindowsAfterExport)
		controller.updateInfo(new GuiDataOpenInExplorer(m_output));

	IOLOG << "ContextExportStep launch 5" << LOGENDL;

	return (m_state = ContextState::done);
}

void ContextExportStep::addTypeDirectory(const ElementType& type)
{
	ElementType toAdd;
	std::wstring name;

	getTypeNameDirectory(type, toAdd, name);

	if (m_typesDirectory.find(toAdd) == m_typesDirectory.end())
		m_typesDirectory[toAdd] = addNewParent(name);

}

void ContextExportStep::getTypeNameDirectory(const ElementType& type, ElementType& directoryType, std::wstring& directoryName)
{
	directoryType = type;
	switch (type)
	{
	case ElementType::Tag:
		directoryName = TEXT_TAGS_TREE_NODE.toStdWString();
		break;
	case ElementType::Point:
		directoryName = TEXT_POINT_TREE_NODE.toStdWString();
		break;
	case ElementType::Scan:
		directoryName = TEXT_SCANS_TREE_NODE.toStdWString();
		break;
	/*case ElementType::BeamBendingMeasure:
		directoryName = TEXT_BEAMBENDING_MEASURE.toStdString();
		break;
	*/
	case ElementType::Cylinder:
		directoryName = TEXT_CYLINDER_SUB_NODE.toStdWString();
		break;
	case ElementType::Torus:
		directoryName = TEXT_TORUS_SUB_NODE.toStdWString();
		break;
	case ElementType::Sphere:
		directoryName = TEXT_SPHERES_TREE_NODE.toStdWString();
		break;
	case ElementType::Piping:
		directoryName = TEXT_PIPING_TREE_ROOT_NODE.toStdWString();
		break;
	case ElementType::Box:
	{
		directoryType = ElementType::Box;
		directoryName = TEXT_BOXES_TREE_NODE.toStdWString();
	}
	break;
	case ElementType::PipeToPipeMeasure:
	case ElementType::PipeToPlaneMeasure:
	case ElementType::PointToPipeMeasure:
	case ElementType::PointToPlaneMeasure:
	case ElementType::SimpleMeasure:
	case ElementType::PolylineMeasure:
	{
		directoryType = ElementType::SimpleMeasure;
		directoryName = TEXT_MEASURES_TREE_NODE.toStdWString();
	}
	break;
	default:
		break;
	}
}

bool ContextExportStep::addPipingDirectory(const xg::Guid& pipingId, Controller& controller)
{
	/*if (!pipingId.isValid())
		return false;
	const Data* pipingIdata = controller.getContext().getCurrentProject()->cgetDataOnId(pipingId);
	const Piping* piping = nullptr;
	if (pipingIdata->getType() == ElementType::Piping)
		piping = static_cast<const Piping*>(pipingIdata);
	else
		return false;

	if (m_pipingDirectory.find(piping) == m_pipingDirectory.end()) {
		addTypeDirectory(ElementType::Piping);
		TDF_Label pipingDirectory = m_typesDirectory[ElementType::Piping];
		m_pipingDirectory[piping] = addNewParent(piping->getComposedName(), pipingDirectory);
	}*/
	return true;
}

bool ContextExportStep::canAutoRelaunch() const
{
	return (false);
}

ContextType ContextExportStep::getType() const
{
	return (ContextType::exportStep);
}

ContextState ContextExportStep::processError(const ContextExportStep::ErrorType& error, Controller& controller)
{
	controller.updateInfo(new GuiDataWarning(TEXT_WRITE_FAILED_PERMISSION));
	return (m_state = ContextState::abort);
}

/*ContextExportStep::ErrorType ContextExportStep::getFilename(const ElementType& type, std::filesystem::path& output) const
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
	name += "-" + std::to_string(1900 + time->tm_year) + "-" + Utils::completeWithZeros(1 + time->tm_mon, 2) + "-" + Utils::completeWithZeros(time->tm_mday, 2) + "_" + std::to_string(time->tm_hour) + "-" + Utils::completeWithZeros(time->tm_min, 2) + "-" + Utils::completeWithZeros(time->tm_sec, 2) + ".step";
	output /= name;
	return ErrorType::Success;
}*/