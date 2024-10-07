#include "controller/functionSystem/ContextMeshObjectCreation.h"
#include "controller/controls/ControlMeshObject.h"
#include "controller/controls/ControlFunction.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "models/3d/ManipulationTypes.h"
#include "vulkan/MeshManager.h"
#include "gui/texts/MeshObjectTexts.hpp"
#include "gui/texts/ContextTexts.hpp"
#include "magic_enum/magic_enum.hpp"
#include "utils/math/trigo.h"
#include "utils/Logger.h"
#include "utils/Utils.h"
#include "vulkan/Graph/MemoryReturnCode.h"

#include "models/graph/MeshObjectNode.h"
#include "models/graph/ClusterNode.h"
#include "models/graph/GraphManager.hxx"

#include "io/imports/stepFileReader.h"

ContextMeshObjectCreation::ContextMeshObjectCreation(const ContextId& id)
	: ARayTracingContext(id)
	, m_count(0)
{
}

ContextMeshObjectCreation::~ContextMeshObjectCreation()
{}

ContextState ContextMeshObjectCreation::start(Controller& controller)
{
	return (m_state = ContextState::waiting_for_input);
}

ContextState ContextMeshObjectCreation::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
	case  IMessage::MessageType::IMPORT_MESHOBJECT:
	{
		auto out = static_cast<ImportMeshObjectMessage*>(message);
		FUNCLOG << "ContextMeshObjectCreation files "<< LOGENDL;
		m_file = out->m_data.file;
		m_inputScale = out->m_data.scale;
		m_up = out->m_data.up;
		m_forward = out->m_data.forward;
		m_isMerge = out->m_data.isMerge;
		m_extension = out->m_data.extension;
		m_posOption = out->m_data.posOption;
		m_infoPosition = out->m_data.position;
		m_lod = out->m_data.lod;
		m_truncateCoor = out->m_data.truncateCoordinatesAsTheScans;
		if (m_posOption == PositionOptions::ClickPosition) 
		{
			m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_MESHOBJECT_START });
			return ARayTracingContext::start(controller);
		}
		else
			return (m_state = ContextState::ready_for_using);
	}
	break;
	case IMessage::MessageType::FULL_CLICK:
	{
		ARayTracingContext::feedMessage(message, controller);
	}
	break;
	default:
		FUNCLOG << "wrong message type (" << magic_enum::enum_name<IMessage::MessageType>(message->getType())<< ")" << LOGENDL;
		break;
	}
    return (m_state);
}

SafePtr<AGraphNode> recCreateArbo(SafePtr<ClusterNode> parentToCopy, SafePtr<AGraphNode> child, 
	std::unordered_set<SafePtr<AGraphNode>>& nodesToAdd, std::unordered_map<xg::Guid, SafePtr<ClusterNode>>& createdArbo,
	Controller& controller)
{
	Data parentData;
	{
		ReadPtr<ClusterNode> rParentToCopy = parentToCopy.cget();
		if (!rParentToCopy)
			return child;
		parentData = (Data)*&rParentToCopy;
	}

	SafePtr<AGraphNode> nextParent = AGraphNode::getOwningParent(parentToCopy, TreeType::MeshObjects);

	SafePtr<ClusterNode> parent = make_safe<ClusterNode>();
	{
		WritePtr<ClusterNode> wParent = parent.get();
		if (!wParent)
			return child;
		wParent->copyUIData(parentData, true);
		wParent->setTreeType(TreeType::MeshObjects);
	}

	AGraphNode::cleanOwningLinks(parent);
	AGraphNode::addOwningLink(parent, child);
	nodesToAdd.insert(parent);
	createdArbo[parentData.getId()] = parent;


	if (nextParent)
	{
		{
			ReadPtr<AGraphNode> rParent = nextParent.cget();
			if (rParent->getType() != ElementType::Cluster)
				return parent;
		}
		SafePtr<ClusterNode> nextParent = static_pointer_cast<ClusterNode>(nextParent);
		return recCreateArbo(nextParent, parent, nodesToAdd, createdArbo, controller);
	}
	return parent;
}

ContextState ContextMeshObjectCreation::launch(Controller& controller)
{
	// --- Ray Tracing ---
	if (m_posOption == PositionOptions::ClickPosition) {
		ARayTracingContext::getNextPosition(controller);
		if (pointMissing())
			return waitForNextPoint(controller);
	}
	// -!- Ray Tracing -!-

	m_state = ContextState::running;
	glm::dvec3 position;

	if ((m_posOption == PositionOptions::GivenCoordinates))
		position = m_infoPosition;

	FUNCLOG << "ContextMeshObjectCreation launch" << LOGENDL;
	const ClippingBoxSettings& settings = controller.getContext().getClippingSettings();

	MeshManager& manager = MeshManager::getInstance();
	MeshObjOutputData objects;

	MeshObjInputData input(m_file, true, m_isMerge);
	input.extension = m_extension;
	input.lod = m_lod;
	input.meshScale = m_inputScale;

	std::filesystem::path folder = controller.getContext().cgetProjectInternalInfo().getObjectsFilesFolderPath();
	/*if (!m_isMerge)
		folder /= m_file.stem();*/
	if (!std::filesystem::exists(folder) && !std::filesystem::create_directories(folder))
	{
		controller.updateInfo(new GuiDataWarning(TEXT_DIRECTORY_CREATION_FAILED.arg(QString::fromStdWString(folder.wstring()))));
		return ARayTracingContext::abort(controller);
	}

	ObjectAllocation::ReturnCode ret(manager.loadFile(objects, input, false, folder, &controller));

	if (ret != ObjectAllocation::ReturnCode::Success)
	{
		controller.updateInfo(new GuiDataWarning(ObjectAllocation::getText(ret)));
		return ARayTracingContext::abort(controller);
	}

	std::unordered_map<xg::Guid, SafePtr<ClusterNode>> createdArbo;
	std::unordered_set<SafePtr<AGraphNode>> nodesToAdd;

	if (!m_isMerge)
	{
		m_rootCluster = make_safe<ClusterNode>();
		WritePtr<ClusterNode> wCluster = m_rootCluster.get();
		if (!wCluster)
			return (m_state = ContextState::abort);

		wCluster->setTreeType(TreeType::MeshObjects);
		wCluster->setName(m_file.stem().wstring());

		nodesToAdd.insert(m_rootCluster);
	}

	switch (m_posOption)
	{
		case PositionOptions::ClickPosition:
			position = m_clickResults[0].position;
			break;
		case PositionOptions::GivenCoordinates:
			position = m_infoPosition;
			break;
		case PositionOptions::KeepModel:
			position = objects.scale * objects.mergeCenter;
			break;
	}

	if (m_truncateCoor)
		position += controller.getContext().cgetProjectInfo().m_importScanTranslation;

	uint32_t count(0);
	glm::dquat rot(getRotation());
	
	for (const std::pair<MeshId, MeshObjOutputData::MeshObjectInfo>& pair : objects.meshIdInfo)
	{
		MeshId mId = pair.first;
		MeshObjOutputData::MeshObjectInfo meshInfo = pair.second;

		if (!mId.isValid())
			continue;

		SMesh smesh = manager.getMesh(mId);
		assert(smesh.m_mesh);

		std::wstring name = smesh.m_name;
		if (name.empty())
			name = m_file.stem().wstring() + L"_" + Utils::wCompleteWithZeros(count);

		TransformationModule tr(glm::dvec3(0.0), rot, glm::dvec3(objects.scale));

		SafePtr<MeshObjectNode> object = make_safe<MeshObjectNode>();
		{
			WritePtr<MeshObjectNode> wObject = object.get();
			if (!wObject)
			{
				assert(false);
				return abort(controller);
			}

			wObject->setDefaultData(controller);

			wObject->setName(name);
			if (m_posOption == PositionOptions::ClickPosition) {

				double up;
				if (m_up == Selection::X || m_up == Selection::_X)
					up = (objects.mergeDim.x) * objects.scale;
				else if (m_up == Selection::Y || m_up == Selection::_Y)
					up = (objects.mergeDim.y) * objects.scale;
				else if (m_up == Selection::Z || m_up == Selection::_Z)
					up = (objects.mergeDim.z) * objects.scale;

				switch (settings.offset)
				{
				case ClippingBoxOffset::CenterOnPoint:
					wObject->setPosition(position);
					break;
				case ClippingBoxOffset::Topface:
					wObject->setPosition(position - glm::dvec3(0., 0., up));
					break;
				case ClippingBoxOffset::BottomFace:
					wObject->setPosition(position + glm::dvec3(0., 0., up));
					break;
				}
			}
			else
				wObject->setPosition(position);

			wObject->setFilePath(meshInfo.path);
			wObject->setObjectName(name);
			wObject->setMeshId(mId);
			manager.addMeshInstance(mId);
			wObject->setDimension(smesh.m_dimensions);
			wObject->setScale(glm::dvec3(objects.scale));
			wObject->setRotation(rot);

			if (!m_isMerge)
			{
				glm::dvec4 p(smesh.m_center, 1.0);
				wObject->addGlobalTranslation(p * tr.getTransformation());
			}

		}


		if (!m_isMerge)
		{
			xg::Guid parentId;
			{
				ReadPtr<ClusterNode> rParent = meshInfo.parentCluster.cget();
				if (rParent)
					parentId = rParent->getId();
			}

			if (createdArbo.find(parentId) == createdArbo.end())
			{
				SafePtr<AGraphNode> root = recCreateArbo(meshInfo.parentCluster, object, nodesToAdd, createdArbo, controller);
				AGraphNode::addOwningLink(m_rootCluster, root);
			}
			else
				AGraphNode::addOwningLink(createdArbo.at(parentId), object);
		}

		nodesToAdd.insert(object);
		count++;
	}
		

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(nodesToAdd));

	controller.updateInfo(new GuiDataProcessingSplashScreenForceClose());
	controller.updateInfo(new GuiDataSplashScreenEnd(GuiDataSplashScreenEnd::SplashScreenType::Message));
	
	if(!count || count != objects.meshIdInfo.size())
	{
		controller.updateInfo(new GuiDataWarning(QString(TEXT_MESHOBJECT_CREATION_FAILED)));
		return ARayTracingContext::abort(controller);
	}

	return ARayTracingContext::validate(controller);
}

ContextState ContextMeshObjectCreation::abort(Controller& controller)
{
	controller.updateInfo(new GuiDataProcessingSplashScreenForceClose());
	return ARayTracingContext::abort(controller);
}

bool ContextMeshObjectCreation::canAutoRelaunch() const
{
	return true;
}

ContextType ContextMeshObjectCreation::getType() const
{
	return ContextType::meshObjectCreation;
}

glm::dquat ContextMeshObjectCreation::getRotation() const
{
	if (m_up == Selection::Y && m_forward == Selection::_Z)
		return tls::math::euler_rad_to_quat(glm::vec3(M_PI2, 0.0, 0.0));
	if (m_up == Selection::Y && m_forward == Selection::Z)
		return tls::math::euler_rad_to_quat(glm::vec3(M_PI2, 0.0, M_PI));
	if (m_up == Selection::Y && m_forward == Selection::X)
		return tls::math::euler_rad_to_quat(glm::vec3(M_PI2, 0.0, M_PI2));
	if (m_up == Selection::Y && m_forward == Selection::_X)
		return tls::math::euler_rad_to_quat(glm::vec3(M_PI2, 0.0, -M_PI2));

	if (m_up == Selection::_Y && m_forward == Selection::_Z)
		return tls::math::euler_rad_to_quat(glm::vec3(-M_PI2, 0.0, 0.0));
	if (m_up == Selection::_Y && m_forward == Selection::Z)
		return tls::math::euler_rad_to_quat(glm::vec3(-M_PI2, 0.0, M_PI));
	if (m_up == Selection::_Y && m_forward == Selection::X)
		return tls::math::euler_rad_to_quat(glm::vec3(-M_PI2, 0.0, M_PI2));
	if (m_up == Selection::_Y && m_forward == Selection::_X)
		return tls::math::euler_rad_to_quat(glm::vec3(-M_PI2, 0.0, -M_PI2));

	if (m_up == Selection::X && m_forward == Selection::_Z)
		return tls::math::euler_rad_to_quat(glm::vec3(0.0, -M_PI2, M_PI2));
	if (m_up == Selection::X && m_forward == Selection::Z)
		return tls::math::euler_rad_to_quat(glm::vec3(0.0, -M_PI2, -M_PI));
	if (m_up == Selection::X && m_forward == Selection::Y)
		return tls::math::euler_rad_to_quat(glm::vec3(0.0, 0.0, -M_PI2));
	if (m_up == Selection::X && m_forward == Selection::_Y)
		return tls::math::euler_rad_to_quat(glm::vec3(0.0, 0.0, M_PI2));

	if (m_up == Selection::_X && m_forward == Selection::_Z)
		return tls::math::euler_rad_to_quat(glm::vec3(0.0, M_PI2, -M_PI2));
	if (m_up == Selection::_X && m_forward == Selection::Z)
		return tls::math::euler_rad_to_quat(glm::vec3(M_PI, M_PI2, -M_PI2));
	if (m_up == Selection::_X && m_forward == Selection::Y)
		return tls::math::euler_rad_to_quat(glm::vec3(0.0, M_PI2, 0.0));
	if (m_up == Selection::_X && m_forward == Selection::_Y)
		return tls::math::euler_rad_to_quat(glm::vec3(0.0, M_PI2, M_PI));

	if (m_up == Selection::Z && m_forward == Selection::_X)
		return tls::math::euler_rad_to_quat(glm::vec3(0.0, 0.0, -M_PI2));
	if (m_up == Selection::Z && m_forward == Selection::X)
		return tls::math::euler_rad_to_quat(glm::vec3(0.0, 0.0, M_PI2));
	if (m_up == Selection::Z && m_forward == Selection::Y)
		return tls::math::euler_rad_to_quat(glm::vec3(0.0, 0.0, 0.0));
	if (m_up == Selection::Z && m_forward == Selection::_Y)
		return tls::math::euler_rad_to_quat(glm::vec3(0.0, 0.0, M_PI));

	if (m_up == Selection::_Z && m_forward == Selection::_X)
		return tls::math::euler_rad_to_quat(glm::vec3(M_PI, 0.0, -M_PI2));
	if (m_up == Selection::_Z && m_forward == Selection::X)
		return tls::math::euler_rad_to_quat(glm::vec3(M_PI, 0.0, M_PI2));
	if (m_up == Selection::_Z && m_forward == Selection::Y)
		return tls::math::euler_rad_to_quat(glm::vec3(M_PI, 0.0, 0.0));
	if (m_up == Selection::_Z && m_forward == Selection::_Y)
		return tls::math::euler_rad_to_quat(glm::vec3(M_PI, 0.0, M_PI));

	return glm::dquat(1.0, 0.0, 0.0, 0.0);
}