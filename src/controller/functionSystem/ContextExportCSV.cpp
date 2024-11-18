#include "controller/functionSystem/ContextExportCSV.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/messages/DataIdListMessage.h"
#include "controller/messages/FilesMessage.h"
#include "controller/messages/PrimitivesExportParametersMessage.h"
#include "utils/Utils.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataIO.h"
#include "io/exports/CSVWriter.hxx"
#include "services/MarkerDefinitions.hpp"
#include "utils/math/trigo.h"
#include "magic_enum/magic_enum.hpp"

#include "models/graph/SimpleMeasureNode.h"
#include "models/graph/PolylineMeasureNode.h"
#include "models/graph/PipeToPipeMeasureNode.h"
#include "models/graph/PipeToPlaneMeasureNode.h"
#include "models/graph/PointToPipeMeasureNode.h"
#include "models/graph/PointToPlaneMeasureNode.h"
#include "models/graph/BeamBendingMeasureNode.h"
#include "models/graph/ColumnTiltMeasureNode.h"
#include "models/graph/ScanNode.h"
#include "models/graph/CylinderNode.h"
#include "models/graph/SphereNode.h"
#include "models/graph/TorusNode.h"
#include "models/graph/TagNode.h"
#include "models/3d/NodeFunctions.h"

#include "models/application/Author.h"

#include <glm/gtx/quaternion.hpp>

#include "gui/texts/ErrorMessagesTexts.hpp"

#define TAG L"Tag"
#define SIMPLEMEASURE L"SimpleMeasure"
#define POLYLINEMEASURE L"PolyLineMeasure"
#define PIPETOPIPEMEASURE L"PipeToPipeMeasure"
#define PIPETOPLANEMEASURE L"PipeToPlaneMeasure"
#define POINTTOPIPEMEASURE L"PointToPipeMeasure"
#define POINTTOPLANEMEASURE L"PointToPlaneMeasure"
#define BEAMBENDINGMEASURE L"BeamBendingMeasure"
#define COLUMNTILTMEASURE L"ColumnTiltMeasure"
#define BOX L"Box"
#define SCAN L"Scanning position"
#define POINT L"Point"
#define PIPE L"Pipe"
#define SPHERE L"Sphere"
#define TORUS L"Torus"
#define DECIMAL 4

#define IN L"In"
#define OUT L"Out"

ContextExportCSV::ContextExportCSV(const ContextId& id)
	: AContext(id)
	, m_primitiveExportParam()
{
	m_state = ContextState::waiting_for_input;
}

ContextExportCSV::~ContextExportCSV()
{}

ContextState ContextExportCSV::start(Controller& controller)
{
	return m_state;
}

ContextState ContextExportCSV::feedMessage(IMessage* message, Controller& controller)
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

ContextState ContextExportCSV::launch(Controller& controller)
{
	m_state = ContextState::running;
	ErrorType err(ErrorType::Success);

	std::unordered_map<ElementType, std::vector<SafePtr<AGraphNode>>> typeObjects;

	ControllerContext& context = controller.getContext();

	std::map<sma::tFieldId, std::wstring> allCustomTagFields;
	for (const SafePtr<AGraphNode>& exportData : m_listToExport) {

		ElementType currentType;
		{
			ReadPtr<AGraphNode> readData = exportData.cget();
			if (!readData)
				continue;
			currentType = readData->getType();
		}

		switch (currentType)
		{
			
			case ElementType::Tag:
			{
				SafePtr<sma::TagTemplate> tagTemplate;
				{
					ReadPtr<TagNode> rTag = static_pointer_cast<TagNode>(exportData).cget();
					if (rTag)
						tagTemplate = rTag->getTemplate();
				}
				
				{
					ReadPtr<sma::TagTemplate> rTagTemp = tagTemplate.cget();
					if (rTagTemp)
						for (auto field : rTagTemp->getFields())
							allCustomTagFields.insert({ field.first, field.second.m_name });
				}
			}
			break;
			case ElementType::Grid:
			{
				currentType = ElementType::Box;
			}
			break;
		}

		if (typeObjects.find(currentType) == typeObjects.end())
			typeObjects[currentType] = { exportData };
		else
			typeObjects[currentType].push_back(exportData);
	}

	std::wstring filename = m_output.filename().stem().wstring();

	for (auto typeObject : typeObjects) {

		if (m_output.empty())
		{
			m_output = context.cgetProjectInternalInfo().getObjectsProjectPath();
			if ((err = getFilename(typeObject.first, m_output)) != ErrorType::Success)
				return processError(err, controller);
		}
		else if (typeObjects.size() > 1) {
			std::wstring name = filename + L"_";
			addTypeExtFilename(typeObject.first, name);
			m_output.replace_filename(name);
		}
		if (m_output.filename().wstring().find_first_of(L".") == std::wstring::npos)
			m_output += ".csv";
		CSVWriter writer(m_output, Utils::to_utf8(m_primitiveExportParam.csvSeparator));
		if (!writer.isGood())
			return processError(ErrorType::FailedToOpen, controller);

		switch (typeObject.first)
		{
			case ElementType::Tag:
			case ElementType::Point:
			case ElementType::BeamBendingMeasure:
			case ElementType::ColumnTiltMeasure:
			case ElementType::Cylinder:
			case ElementType::Torus:
			case ElementType::Sphere:
			case ElementType::PipeToPipeMeasure:
			case ElementType::PipeToPlaneMeasure:
			case ElementType::PointToPipeMeasure:
			case ElementType::PointToPlaneMeasure:
			case ElementType::SimpleMeasure:
			case ElementType::PolylineMeasure:
			case ElementType::Box:
			case ElementType::Grid:
			{
				writer << L"Author" << L"Color R" << L"Color G" << L"Color B" << L"Index" << L"Identifier" << L"Name" << L"Description" << L"Discipline" << L"Phase" << L"UUID" << L"Hyperlinks" << L"Creation Date" << L"Modification Date";
				break;
			}
		}


		switch (typeObject.first)
		{
			case ElementType::Scan:
				writer << L"Name" << L"X (m)" << L"Y (m)" << L"Z (m)" << L"Rot X" << L"Rot Y" << L"Rot Z" << L"Point Count";
				break;
			case ElementType::Tag:
				{
					writer << "X (m)" << L"Y (m)" << L"Z (m)" << L"Marker Icon";
					for (auto field : allCustomTagFields)
						writer << field.second;
				}
				break;
			case ElementType::Point:
				writer << L"X (m)" << L"Y (m)" << L"Z (m)";
				break;
			case ElementType::BeamBendingMeasure:
				writer << L"X (m)" << L"Y (m)" << L"Z (m)" << L"Bending (m)" << L"Length (m)" << L"Ratio b/L = 1/" << L"Max ratio = 1/" << L"Tolerance (m)" << L"Reliability" << L"Ratio b/L > Max Ratio ?";
				break;
			case ElementType::ColumnTiltMeasure:
				writer << L"X (m)" << L"Y (m)" << L"Z (m)" << L"Tilt (m)" << L"Height (m)" << L"Ratio t/h = 1/" << L"Max ratio = 1/" << L"Tolerance (m)" << L"Reliability" << "Ratio t/h > Max Ratio ?";
				break;
			case ElementType::Cylinder:
				writer << L"Start X (m)" << L"Start Y (m)" << L"Start Z (m)" << L"End X (m)" << L"End Y (m)" << L"End Z (m)" << L"Detected diam (m)" << L"Retained diam (m)" << L"Length (m)" << L"Volume (m3)";;
				break;
			case ElementType::Torus:
				writer << L"X (m)" << L"Y (m)" << L"Z (m)" << L"Detected diam (m)" << L"Retained diam (m)" << L"Angle" << L"Volume (m3)";;
				break;
			case ElementType::Sphere:
				writer << L"X (m)" << L"Y (m)" << L"Z (m)" << L"Diameter (m)" << L"Volume (m3)";;
				break;
			case ElementType::Box:
			case ElementType::Grid:
				writer << L"X (m)" << L"Y (m)" << L"Z (m)" << L"Rot X" << L"Rot Y" << L"Rot Z" << L"Size X (m)" << L"Size Y (m)" << L"Size Z (m)" << L"Volume (m3)";
				break;
			case ElementType::PipeToPipeMeasure:
				writer << L"Axis Distance (m)" << L"Axis Distance Horizontal (m)" << L"Axis Distance Vertical (m)" << L"Total Footprint (m)" 
					<< L"Free Distance (m)" << L"Free Distance Horizontal (m)" << L"Free Distance Vertical (m)" << L"Pipe 1 Diameter (m)" << L"Pipe 2 Diameter (m)" 
					<< L"Pipe 2 Center to Projected (m)" << L"Pipe 1 Center X (m)" << L"Pipe 1 Center Y (m)" <<  L"Pipe 1 Center Z (m)" 
					<< L"Pipe 2 Center X (m)" << L"Pipe 2 Center Y (m)" << L"Pipe 2 Center Z (m)" << L"Projected Point X (m)" << L"Projected Point Y (m)" << L"Projected Point Z (m)";
				break;
			case ElementType::PipeToPlaneMeasure:
				writer << L"Pipe to Plane Distance (m)" << L"Pipe to Plane Distance Horizontal (m)" << L"Pipe to Plane Distance Vertical (m)" 
					<< L"Total Footprint (m)" << L"Free Distance (m)" << L"Free Distance Horizontal (m)" << L"Free Distance Vertical (m)" << L"Pipe Diameter (m)" 
					<< L"Point on plane to Projected (m)" << L"Pipe Center X (m)" << L"Pipe Center Y (m)" << L"Pipe Center Z (m)" 
					<< L"Point on Plane X (m)" << L"Point on Plane Y (m)" << L"Point on Plane Z (m)" << L"Normal to Plane X" << L"Normal to Plane Y" << L"Normal to Plane Z" 
					<< L"Projected Point Coordinates X (m)" << L"Projected Point Coordinates Y (m)" << L"Projected Point Coordinates Z (m)";
				break;
			case ElementType::PointToPipeMeasure:
				writer << L"Point to Axis Distance (m)" << L"Point to Axis Distance Horizontal (m)" << L"Point to Axis Distance Vertical (m)" 
					<< L"Total Footprint (m)" << L"Free Distance (m)" << L"Free Distance Horizontal (m)" << L"Free Distance Vertical (m)" << L"Pipe Diameter (m)" 
					<< L"Pipe Center to Projected (m)" << L"Pipe Center X (m)" << L"Pipe Center Y (m)" << L"Pipe Center Z (m)" 
					<< L"Point Coordinates X (m)" << L"Point Coordinates Y (m)" << L"Point Coordinates Z (m)" 
					<< L"Projected Point Coordinates X (m)" << L"Projected Point Coordinates Y (m)" << L"Projected Point Coordinates Z (m)";
				break;
			case ElementType::PointToPlaneMeasure:
				writer << L"Point to Plane distance (m)" << L"Point to Plane Distance Horizontal (m)" << L"Point to Plane Distance Vertical (m)" 
					<< L"Projected Point to Plane Distance (m)" << L"Point Coordinates X (m)" << L"Point Coordinates Y (m)" << L"Point Coordinates Z (m)" 
					<< L"Point on Plane Coordinates X (m)" << L"Point on Plane Coordinates Y (m)" << L"Point on Plane Coordinates Z (m)" 
					<< L"Normal to Plane X" << L"Normal to Plane Y" << L"Normal to Plane Z" 
					<< L"Projected Point Coordinates X (m)" << L"Projected Point Coordinates Y (m)" << L"Projected Point Coordinates Z (m)";
				break;
			case ElementType::SimpleMeasure:
				writer << L"Total (m)" << L"Horizontal (m)" << L"Vertical (m)" << L"Along X (m)" << L"Along Y (m)" << L"Angle with horizontal (arc degree)" 
					<< L"Point 1 Coordinates X (m)" << L"Point 1 Coordinates Y (m)" << L"Point 1 Coordinates Z (m)" 
					<< L"Point 2 Coordinates X (m)" << L"Point 2 Coordinates Y (m)" << L"Point 2 Coordinates Z (m)";
				break;
			case ElementType::PolylineMeasure:
			{
				uint32_t nbMeasureMax = 0;
				for (const SafePtr<AGraphNode>& polyPtr : typeObject.second)
				{
					ReadPtr<PolylineMeasureNode> readPoly = static_pointer_cast<PolylineMeasureNode>(polyPtr).cget();
					if (!readPoly)
						continue;
					if (nbMeasureMax < readPoly->getMeasures().size())
						nbMeasureMax = (uint32_t)readPoly->getMeasures().size();
				}
				writer << L"Total (m)" << L"Total Horizontal (m)" << L"Horizontal (m)" << L"Vertical (m)" << L"XY Area (m2)" << L"YZ Area (m2)" << L"XZ Area (m2)" 
					<< L"Point 1 Coordinates X (m)" << L"Point 1 Coordinates Y (m)" << L"Point 1 Coordinates Z (m)";
				for (uint32_t i = 0; i < nbMeasureMax; i++)
				{
					writer << L"Point " + std::to_wstring(i+2) + L" Coordinates X (m)" << L"Point " + std::to_wstring(i + 2) + L" Coordinates Y (m)" << L"Point " + std::to_wstring(i + 2) + L" Coordinates Z (m)";
					writer << L"Measure " + std::to_wstring(i + 1) + L" Total Length (m)" << L"Measure " + std::to_wstring(i + 1) + L" Horizontal Length (m)" << L"Measure " + std::to_wstring(i + 1) + L" Vertical Length (m)";
				}
				break;
			}
		}

		writer << CSVWriter::endl;

		for (const SafePtr<AGraphNode>& dataPtr : typeObject.second)
		{
			double objectVolume = nodeFunctions::calculateVolume(dataPtr);
			std::wstring authName = L"NO_AUTHOR";

			ElementType type;
			TransformationModule transfoModule;
			{
				ReadPtr<AGraphNode> readNode = dataPtr.cget();
				if (!readNode)
					continue;
				type = readNode->getType();
				transfoModule = readNode->getTransformationModule();

				{
					ReadPtr<Author> rAuth = readNode->getAuthor().cget();
					if (rAuth)
						authName = rAuth->getName();
				}

				switch (readNode->getType())
				{
					case ElementType::Tag:
					case ElementType::Point:
					case ElementType::BeamBendingMeasure:
					case ElementType::ColumnTiltMeasure:
					case ElementType::Cylinder:
					case ElementType::Torus:
					case ElementType::Sphere:
					case ElementType::PipeToPipeMeasure:
					case ElementType::PipeToPlaneMeasure:
					case ElementType::PointToPipeMeasure:
					case ElementType::PointToPlaneMeasure:
					case ElementType::SimpleMeasure:
					case ElementType::PolylineMeasure:
					case ElementType::Box:
					case ElementType::Grid:
					{
						Color32 color = readNode->getColor();
						writer << authName << Utils::wCompleteWithZeros(color.r) << Utils::wCompleteWithZeros(color.g) << Utils::wCompleteWithZeros(color.b)
							<< Utils::wCompleteWithZeros(readNode->getUserIndex()) << readNode->getIdentifier() << readNode->getName()
							<< readNode->getDescription() << readNode->getDiscipline() << readNode->getPhase() << Utils::from_utf8(readNode->getId());
						std::wstring links = L"\"";
						for (auto link : readNode->getHyperlinks())
							links += link.second.hyperlink + L'\n';
						links += L'"';
						writer << links;
						writer << readNode->getStringTimeCreated();
						writer << readNode->getStringTimeModified();
						break;
					}
				}
			}

			glm::dvec3 decalExport = (m_primitiveExportParam.exportWithScanImportTranslation ? -controller.getContext().getProjectInfo().m_importScanTranslation : glm::dvec3(0.));
			glm::vec3 position = transfoModule.getCenter() + decalExport;

			switch (type)
			{
			case ElementType::Scan:
			{
				ReadPtr<ScanNode> readScan = static_pointer_cast<ScanNode>(dataPtr).cget();
				if (!readScan)
					continue;
				glm::vec3 rotation = tls::math::quat_to_euler_zyx_deg(transfoModule.getOrientation());
				writer << readScan->getName() << position.x << position.y << position.z << rotation.x << rotation.y << rotation.z << readScan->getNbPoint();
			}
			break;
			case ElementType::Tag:
			{
				ReadPtr<TagNode> readTag = static_pointer_cast<TagNode>(dataPtr).cget();
				if (!readTag)
					continue;
				Color32 color = readTag->getColor();
				writer << position.x << position.y << position.z << scs::markerStyleDefs.at(readTag->getMarkerIcon()).traduction.toStdWString();
				for (auto field : allCustomTagFields)
					writer << readTag->getValue(field.first);
			}
			break;
			case ElementType::Point:
			{
				ReadPtr<AObjectNode> readObj = static_pointer_cast<AObjectNode>(dataPtr).cget();
				if (!readObj)
					continue;
				Color32 color = readObj->getColor();
				writer << position.x << position.y << position.z;
			}
			break;
			case ElementType::BeamBendingMeasure:
			{
				ReadPtr<BeamBendingMeasureNode> readBbm = static_pointer_cast<BeamBendingMeasureNode>(dataPtr).cget();
				if (!readBbm)
					continue;
				Color32 color = readBbm->getColor();
				glm::vec3 position = readBbm->getMaxBendingPos() + decalExport;
				double tolerance(context.cgetProjectInfo().m_beamBendingTolerance);
				if (tolerance)
					tolerance = 1.0 / tolerance;
				writer << position.x << position.y << position.z << readBbm->getBendingValue() << readBbm->getLength()
					<< readBbm->getRatio() << readBbm->getMaxRatio() << tolerance
					<< ((readBbm->getBendingValue() > tolerance) ? OUT : IN) << Utils::from_utf8(std::string(magic_enum::enum_name(readBbm->getRatioSup())));
			}
			break;
			case ElementType::ColumnTiltMeasure:
			{
				ReadPtr<ColumnTiltMeasureNode> readCtm = static_pointer_cast<ColumnTiltMeasureNode>(dataPtr).cget();
				if (!readCtm)
					continue;
				Color32 color = readCtm->getColor();
				glm::vec3 position = readCtm->getTopPoint() + decalExport;
				double tolerance(context.cgetProjectInfo().m_columnTiltTolerance);
				if (tolerance)
					tolerance = 1.0 / tolerance;
				writer << position.x << position.y << position.z << readCtm->getTiltValue() << readCtm->getHeight()
					<< readCtm->getRatio() << readCtm->getMaxRatio() << tolerance
					<< ((readCtm->getTiltValue() > tolerance) ? OUT : IN) << Utils::from_utf8(std::string(magic_enum::enum_name(readCtm->getRatioSup())));
			}
			break;
			case ElementType::Cylinder:
			{
				ReadPtr<CylinderNode> readCyl = static_pointer_cast<CylinderNode>(dataPtr).cget();
				if (!readCyl)
					continue;
				Color32 color = readCyl->getColor();
				glm::dvec4 posStart, posEnd;
				glm::dmat4 rotation(glm::toMat4(readCyl->getOrientation()));
				posStart = glm::dvec4(0.0, 0.0, -readCyl->getScale().z, 1.0);
				posEnd = glm::dvec4(0.0, 0.0, readCyl->getScale().z, 1.0);
				posStart = rotation * posStart;
				posEnd = rotation * posEnd;
				posStart += glm::dvec4(position, 0.0);
				posEnd += glm::dvec4(position, 0.0);
				writer << posStart.x << posStart.y << posStart.z << posEnd.x << posEnd.y << posEnd.z << Utils::wRoundFloat(readCyl->getDetectedRadius() * 2.0, DECIMAL) <<
					Utils::wRoundFloat(readCyl->getRadius() * 2.0, DECIMAL) << Utils::wRoundFloat(readCyl->getLength(), DECIMAL) << Utils::wRoundFloat(objectVolume, DECIMAL);
			}
			break;
			case ElementType::Torus:
			{
				ReadPtr<TorusNode> readTor = static_pointer_cast<TorusNode>(dataPtr).cget();
				if (!readTor)
					continue;
				Color32 color = readTor->getColor();
				writer << position.x << position.y << position.z << Utils::wRoundFloat(readTor->getAdjustedTubeRadius() * 2.0, DECIMAL) <<
					Utils::wRoundFloat(readTor->getMainRadius() * 2.0, DECIMAL) << Utils::wRoundFloat(readTor->getMainAngle(), DECIMAL) << Utils::wRoundFloat(objectVolume, DECIMAL);
			}
			break;
			case ElementType::Sphere:
			{
				ReadPtr<SphereNode> readSph = static_pointer_cast<SphereNode>(dataPtr).cget();
				if (!readSph)
					continue;
				Color32 color = readSph->getColor();
				writer << position.x << position.y << position.z <<
					Utils::wRoundFloat(readSph->getRadius() * 2.0, DECIMAL) << Utils::wRoundFloat(objectVolume, DECIMAL);
			}
			break;
			case ElementType::Box:
			case ElementType::Grid:
			{
				ReadPtr<AObjectNode> readBox = static_pointer_cast<AObjectNode>(dataPtr).cget();
				if (!readBox)
					continue;
				Color32 color = readBox->getColor();
				glm::vec3 size = readBox->getSize();
				glm::dvec3 eulers(tls::math::quat_to_euler_zyx_deg(readBox->getOrientation()));
				writer << position.x << position.y << position.z << eulers.x << eulers.y << eulers.z << size.x << size.y << size.z << Utils::wRoundFloat(objectVolume, DECIMAL);
			}
			break;
			case ElementType::PipeToPipeMeasure:
			{
				ReadPtr<PipeToPipeMeasureNode> readPtp = static_pointer_cast<PipeToPipeMeasureNode>(dataPtr).cget();
				if (!readPtp)
					continue;

				glm::vec3 pipe1Pos = readPtp->getPipe1Center() + decalExport;
				glm::vec3 pipe2Pos = readPtp->getPipe2Center() + decalExport;
				glm::vec3 projPos = readPtp->getProjPoint() + decalExport;

				writer << readPtp->getCenterP1ToAxeP2() << readPtp->getP1ToP2Horizontal() << readPtp->getP1ToP2Vertical() << readPtp->getTotalFootprint() <<
					readPtp->getFreeDist() << readPtp->getFreeDistHorizontal() << readPtp->getFreeDistVertical() << readPtp->getPipe1Diameter() << readPtp->getPipe2Diameter() <<
					readPtp->getPipe2CenterToProj() << pipe1Pos.x << pipe1Pos.y << pipe1Pos.z <<
					pipe2Pos.x << pipe2Pos.y << pipe2Pos.z << projPos.x << projPos.y << projPos.z;
			}
			break;
			case ElementType::PipeToPlaneMeasure:
			{
				ReadPtr<PipeToPlaneMeasureNode> readPtpl = static_pointer_cast<PipeToPlaneMeasureNode>(dataPtr).cget();
				if (!readPtpl)
					continue;

				glm::vec3 pipePos = readPtpl->getPipeCenter() + decalExport;
				glm::vec3 planePos = readPtpl->getPointOnPlane() + decalExport;
				glm::vec3 normVec = readPtpl->getNormalOnPlane();
				glm::vec3 projPos = readPtpl->getProjPoint() + decalExport;

				writer << readPtpl->getCenterToPlaneDist() << readPtpl->getPlaneCenterHorizontal() << readPtpl->getPlaneCenterVertical() << readPtpl->getTotalFootprint() <<
					readPtpl->getFreeDist() << readPtpl->getFreeDistHorizontal() << readPtpl->getFreeDistVertical() << readPtpl->getPipeDiameter() << readPtpl->getPointOnPlaneToProj() <<
					pipePos.x << pipePos.y << pipePos.z << planePos.x << planePos.y << planePos.z <<
					normVec.x << normVec.y << normVec.z << projPos.x << projPos.y << projPos.z;
			}
			break;
			case ElementType::PointToPipeMeasure:
			{
				ReadPtr<PointToPipeMeasureNode> potp = static_pointer_cast<PointToPipeMeasureNode>(dataPtr).cget();
				if (!potp)
					continue;

				glm::vec3 pipePos = potp->getPipeCenter() + decalExport;
				glm::vec3 pointPos = potp->getPointCoord() + decalExport;
				glm::vec3 projPos = potp->getProjPoint() + decalExport;

				writer << potp->getPointToAxeDist() << potp->getPointToAxeHorizontal() << potp->getPointToAxeVertical() << potp->getTotalFootprint() <<
					potp->getFreeDist() << potp->getFreeDistHorizontal() << potp->getFreeDistVertical() << potp->getPipeDiameter() << potp->getPipeCenterToProj() <<
					pipePos.x << pipePos.y << pipePos.z << pointPos.x << pointPos.y << pointPos.z <<
					projPos.x << projPos.y << projPos.z;
			}
			break;
			case ElementType::PointToPlaneMeasure:
			{
				ReadPtr<PointToPlaneMeasureNode> potpl = static_pointer_cast<PointToPlaneMeasureNode>(dataPtr).cget();
				if (!potpl)
					continue;

				glm::vec3 pointPos = potpl->getPointCoord() + decalExport;
				glm::vec3 planePos = potpl->getPointOnPlane() + decalExport;
				glm::vec3 normVec = potpl->getNormalToPlane();
				glm::vec3 projPos = potpl->getProjPoint() + decalExport;

				writer << potpl->getPointToPlaneD() << potpl->getHorizontal() << potpl->getVertical() << potpl->getPointProjToPlaneD() <<
					planePos.x << planePos.y << planePos.z << pointPos.x << pointPos.y << pointPos.z <<
					normVec.x << normVec.y << normVec.z << projPos.x << projPos.y << projPos.z;
			}
			break;
			case ElementType::SimpleMeasure:
			{
				ReadPtr<SimpleMeasureNode> sm = static_pointer_cast<SimpleMeasureNode>(dataPtr).cget();
				if (!sm)
					continue;

				glm::vec3 originPos = sm->getMeasure().origin + decalExport;
				glm::vec3 finalPos = sm->getMeasure().final + decalExport;

				writer << sm->getMeasure().getDistanceTotal() << sm->getMeasure().getDistanceHorizontal() << sm->getMeasure().getDistanceAlongZ()
					<< sm->getMeasure().getDistanceAlongX() << sm->getMeasure().getDistanceAlongY() << sm->getMeasure().getAngleHorizontal()
					<< originPos.x << originPos.y << originPos.z << finalPos.x << finalPos.y << finalPos.z;
			}
			break;
			case ElementType::PolylineMeasure:
			{
				ReadPtr<PolylineMeasureNode> pm = static_pointer_cast<PolylineMeasureNode>(dataPtr).cget();
				if (!pm)
					continue;
				double total = 0;
				double totalH = 0;

				glm::dvec3 area = pm->computeAreaOfPolyline();
				Measure hV = { pm->getFirstPos(), pm->getLastPos() };
				for (const Measure& m : pm->getMeasures())
				{
					total += m.getDistanceTotal();
					totalH += m.getDistanceHorizontal();
				}

				glm::vec3 originPos = pm->getFirstPos() + decalExport;

				writer << total << totalH << hV.getDistanceHorizontal() << hV.getDistanceAlongZ() << area.z << area.x << area.y
					<< originPos.x << originPos.y << originPos.z;
				for (const Measure& m : pm->getMeasures())
				{
					glm::vec3 nextPos = m.final + decalExport;
					writer << nextPos.x << nextPos.y << nextPos.z << m.getDistanceTotal() << m.getDistanceHorizontal() << m.getDistanceAlongZ();
				}

			}
			break;
			}

			writer << CSVWriter::endl;
		}

		writer << CSVWriter::endl;
	}

	controller.updateInfo(new GuiDataTmpMessage(TEXT_EXPORT_FILES_EXPORTED_TO.arg(QString::fromStdWString(m_output.parent_path().wstring())), 2000));
	if (m_primitiveExportParam.openFolderWindowsAfterExport)
		controller.updateInfo(new GuiDataOpenInExplorer(m_output));

	return (m_state = ContextState::done);
}

bool ContextExportCSV::canAutoRelaunch() const
{
	return (false);
}

ContextType ContextExportCSV::getType() const
{
	return (ContextType::exportCSV);
}

ContextState ContextExportCSV::processError(const ContextExportCSV::ErrorType& error, Controller& controller)
{
	controller.updateInfo(new GuiDataWarning(TEXT_WRITE_FAILED_PERMISSION));
	return (m_state = ContextState::abort);
}

ContextExportCSV::ErrorType ContextExportCSV::getFilename(const ElementType& type, std::filesystem::path& output) const
{
	std::wstring name;
	if (addTypeExtFilename(type, name) == ErrorType::WrongType)
		return ErrorType::WrongType;
	std::time_t result = std::time(nullptr);
	std::tm* time = localtime(&result);
	name += L"-" + std::to_wstring(1900 + time->tm_year) + L"-" + Utils::wCompleteWithZeros(1 + time->tm_mon, 2) + L"-" + Utils::wCompleteWithZeros(time->tm_mday, 2) + L"_" + std::to_wstring(time->tm_hour) + L"-" + Utils::wCompleteWithZeros(time->tm_min, 2) + L"-" + Utils::wCompleteWithZeros(time->tm_sec, 2) + L".dxf";
	output /= name;
	return ErrorType::Success;
}

ContextExportCSV::ErrorType ContextExportCSV::addTypeExtFilename(const ElementType & type, std::wstring& fileName) const
{
	switch (type)
	{
	case ElementType::Scan:
		fileName += SCAN;
		break;
	case ElementType::BeamBendingMeasure:
		fileName += BEAMBENDINGMEASURE;
		break;
	case ElementType::ColumnTiltMeasure:
		fileName += COLUMNTILTMEASURE;
		break;
	case ElementType::PipeToPipeMeasure:
		fileName += PIPETOPIPEMEASURE;
		break;
	case ElementType::PipeToPlaneMeasure:
		fileName += PIPETOPLANEMEASURE;
		break;
	case ElementType::PointToPipeMeasure:
		fileName += POINTTOPIPEMEASURE;
		break;
	case ElementType::PointToPlaneMeasure:
		fileName += POINTTOPLANEMEASURE;
		break;
	case ElementType::SimpleMeasure:
		fileName += SIMPLEMEASURE;
		break;
	case ElementType::PolylineMeasure:
		fileName += POLYLINEMEASURE;
		break;
	case ElementType::Point:
		fileName += POINT;
		break;
	case ElementType::Tag:
		fileName += TAG;
		break;
	case ElementType::Box:
	case ElementType::Grid:
		fileName += BOX;
		break;
	case ElementType::Cylinder:
		fileName += PIPE;
		break;
	case ElementType::Sphere:
		fileName += SPHERE;
		break;
	case ElementType::Torus:
		fileName += TORUS;
		break;
	case ElementType::None:
		fileName = L"All";
		break;
	default:
		return ErrorType::WrongType;
	}
	return ErrorType::Success;
}
