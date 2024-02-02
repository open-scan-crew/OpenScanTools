#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"
#include "vulkan/VulkanManager.h"
#include "utils/Logger.h"
#include "gui/IDataDispatcher.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "vulkan/Graph/MemoryReturnCode.h"

#include "models/3d/Graph/ScanNode.h"
#include "models/3d/Graph/ScanObjectNode.h"
#include "models/3d/Graph/TagNode.h"
#include "models/3d/Graph/TorusNode.h"
#include "models/3d/Graph/MeshObjectNode.h"
#include "models/3d/Graph/ClusterNode.h"
#include "models/3d/Graph/BoxNode.h"
#include "models/3d/Graph/PolylineMeasureNode.h"
#include "models/3d/Graph/CameraNode.h"

#include "controller/controls/ControlObject3DEdition.h"
#include "controller/controls/ControlFunction.h"

#include "models/project/ProjectTypes.h"

#include "gui/texts/TreePanelTexts.hpp"

#include "pointCloudEngine/PCE_core.h"


#define SGLog Logger::log(LoggerMode::SceneGraphLog)

OpenScanToolsGraphManager::OpenScanToolsGraphManager(IDataDispatcher& dataDispatcher)
	: m_root(make_safe<ReferentialNode>(L"Root", TransformationModule()))
	, m_meshManager(&MeshManager::getInstance())
	, m_dataDispatcher(dataDispatcher)
{
	// Uncomment this code to test a root different than zero.
	//WritePtr<ReferentialNode> wRoot = m_root.get();
	//wRoot->setPosition(glm::dvec3(0.0, 5.0, 0.0));
	//wRoot->setRotation(glm::dquat(-0.0012293305, 0.0038679366, 0.688302797, 0.725412149));

	registerGuiDataFunction(guiDType::activatedFunctions, &OpenScanToolsGraphManager::onFunctionChanged);
	registerGuiDataFunction(guiDType::renderTargetClick, &OpenScanToolsGraphManager::onClick);
	registerGuiDataFunction(guiDType::projectTransformation, &OpenScanToolsGraphManager::onProjectTransformation);
	registerGuiDataFunction(guiDType::manipulatorMode, &OpenScanToolsGraphManager::onManipulationMode);
}

OpenScanToolsGraphManager::~OpenScanToolsGraphManager()
{
	cleanProjectObjects();
}

void OpenScanToolsGraphManager::registerGuiDataFunction(guiDType type, OpenScanToolsGraphManagerMethod fct)
{
	m_dataDispatcher.registerObserverOnKey(this, type);
	m_methods.insert({ type, fct });
};

void OpenScanToolsGraphManager::informData(IGuiData* data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		OpenScanToolsGraphManagerMethod method = m_methods.at(data->getType());
		(this->*method)(data, true);
	}
}

template<class NodeType>
void updateMarker(AObjectNode* node)
{
	static_cast<NodeType*>(node)->updateMarker();
}

template<class NodeType>
void updateMeasure(AObjectNode* node)
{
	static_cast<NodeType*>(node)->updateMeasure();
}

SafePtr<ReferentialNode> OpenScanToolsGraphManager::getRoot()
{
	return m_root;
}

const SafePtr<ManipulatorNode>& OpenScanToolsGraphManager::getManipulatorNode()
{
	return m_manipulatorNode;
}

const SafePtr<ClusterNode>& OpenScanToolsGraphManager::getHierarchyMasterCluster() const
{
	return m_hierarchyMasterCluster;
}

void OpenScanToolsGraphManager::setHierarchyMasterCluster(const SafePtr<ClusterNode>& hierarchy)
{
	m_hierarchyMasterCluster = hierarchy;
}

void OpenScanToolsGraphManager::createHierarchyMasterCluster()
{
	SafePtr<ClusterNode> hierarchyMasterCluster = make_safe<ClusterNode>();
	{
		WritePtr<ClusterNode> wHMC = hierarchyMasterCluster.get();
		wHMC->setTreeType(TreeType::Hierarchy);
		wHMC->setName(TEXT_HIERARCHY_TREE_NODE.toStdWString());
		wHMC->m_isMasterCluster = true;
	}

	setHierarchyMasterCluster(hierarchyMasterCluster);
}

void OpenScanToolsGraphManager::refreshScene()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	m_meshManager->emptyTrash();

    for (IGuiData* data : m_waitingDataToProceed)
    {
        if (m_methods.find(data->getType()) != m_methods.end())
        {
            OpenScanToolsGraphManagerMethod method = m_methods.at(data->getType());
            (this->*method)(data, false);
        }
        delete data;
    }
    m_waitingDataToProceed.clear();
}

void OpenScanToolsGraphManager::cleanProjectObjects()
{
	AGraphNode::cleanLinks(m_manipulatorNode);
	m_manipulatorNode.destroy();

	AGraphNode::cleanLinks(m_hierarchyMasterCluster);
	m_hierarchyMasterCluster.destroy();

	for (SafePtr<AGraphNode> projectNode : getProjectNodes())
	{
		AGraphNode::cleanLinks(projectNode);
		projectNode.destroy();
	}

	AGraphNode::cleanLinks(m_root);

	{
		WritePtr<CameraNode> wCam = m_cameraNode.get();
		if (wCam)
			wCam->resetExaminePoint();
	}

	m_targetMarkerFactory.freeClickMarkers();
	// Note(aurélien) on tente le nettoyage des geometries générées à la volé.
	VulkanManager::getInstance().waitIdle();
	m_meshManager->cleanMemory(true);
	//m_meshManager->cleanSimpleGeometryMemory();
}

TargetMarkerFactory& OpenScanToolsGraphManager::getTargetFactory()
{
	return m_targetMarkerFactory;
}

// Is this function synchrone or asynchrone ?
void OpenScanToolsGraphManager::setObjectsHovered(std::unordered_set<uint32_t>&& objectsHovered)
{
    resetObjectsHovered();

    std::unordered_set<SafePtr<AGraphNode>> nodesHovered = getNodesFromGraphicIds(objectsHovered);
    for (const SafePtr<AGraphNode>& node : nodesHovered)
    {
        SafePtr<AObjectNode> object = static_pointer_cast<AObjectNode>(node);
        WritePtr<AObjectNode> wObj = object.get();
        if (!wObj)
            continue;

        wObj->setHovered(true);
        m_objectsHovered.insert(object);
    }
}

SafePtr<AObjectNode> OpenScanToolsGraphManager::getSingleHoverObject() const
{
    if (m_objectsHovered.size() == 1)
        return (*m_objectsHovered.begin());
    else
        return SafePtr<AObjectNode>();
}

void OpenScanToolsGraphManager::resetObjectsHovered()
{
    for (const SafePtr<AObjectNode>& object : m_objectsHovered)
    {
        WritePtr<AObjectNode> wObj = object.get();
        if (!wObj)
            continue;

        wObj->setHovered(false);
    }
    m_objectsHovered.clear();
}

void OpenScanToolsGraphManager::onFunctionChanged(IGuiData* iGuiData, bool store)
{
	auto examineData = static_cast<GuiDataActivatedFunctions*>(iGuiData);

	if (store)
	{
		m_waitingDataToProceed.push_back(new GuiDataActivatedFunctions(*examineData));
		return;
	}
	if (examineData->type != ContextType::none)
	{
		m_targetMarkerFactory.freeClickMarkers();
	}
}

void OpenScanToolsGraphManager::onClick(IGuiData* iGuiData, bool store)
{
	auto targetData = static_cast<GuiDataRenderTargetClick*>(iGuiData);

	if (store)
	{
		m_waitingDataToProceed.push_back(new GuiDataRenderTargetClick(*targetData));
		return;
	}

	if (targetData->m_reset)
		m_targetMarkerFactory.freeClickMarkers();
	else if (!isnan(targetData->m_position.x))
		m_targetMarkerFactory.createClickTarget(glm::dvec4(targetData->m_position, 1.0), targetData->m_color);
}

void OpenScanToolsGraphManager::onProjectTransformation(IGuiData* iGuiData, bool store)
{
	if (store)
	{
		m_waitingDataToProceed.push_back(new GuiDataProjectTransformation(static_cast<GuiDataProjectTransformation*>(iGuiData)->m_projectTransformation));
		return;
	}
    else
    {
        // ----- Désactivé -----
        //setRootTransformation(static_cast<GuiDataProjectTransformation*>(iGuiData)->m_projectTransformation);
    }
}

void OpenScanToolsGraphManager::onManipulationMode(IGuiData* data, bool store)
{
	WritePtr<ManipulatorNode> wManip = m_manipulatorNode.get();
	if (!wManip)
		return;

	m_manipulationMode = static_cast<GuiDataManipulatorMode*>(data)->m_mode;
	wManip->setManipulationMode(m_manipulationMode);
}

void OpenScanToolsGraphManager::addNodesToGraph(const std::unordered_set<SafePtr<AGraphNode>>& nodes)
{
	for(const SafePtr<AGraphNode>& node : nodes)
		AGraphNode::addGeometricLink(m_root, node);
}

SafePtr<CameraNode> OpenScanToolsGraphManager::createCameraNode(const std::wstring& name)
{
	SafePtr<CameraNode> cameraNode = make_safe<CameraNode>(name, m_dataDispatcher);
	m_cameraNode = cameraNode;
	return cameraNode;
}

SafePtr<CameraNode> OpenScanToolsGraphManager::duplicateCamera(const SafePtr<CameraNode>& camera)
{
	ReadPtr<CameraNode> rcam = camera.cget();
	if (rcam)
		return make_safe<CameraNode>(*&rcam);
	else
		return SafePtr<CameraNode>();
}

void OpenScanToolsGraphManager::detachManipulator(AObjectNode* parent)
{
	/*
	if (parent->isGeometricChild(m_manipulator->getId()))
		ManipulatorNode::cancel(m_manipulator);
	*/
}

std::unordered_set<SafePtr<AGraphNode>> OpenScanToolsGraphManager::getNodesFromGraphicIds(const std::unordered_set<uint32_t>& graphicIds) const
{
    return getNodesOnFilter<AGraphNode>(
        [&graphicIds](ReadPtr<AGraphNode>& node) {
            return graphicIds.find(node->getGraphicId()) != graphicIds.end();
        }
    );
}

SafePtr<AGraphNode> OpenScanToolsGraphManager::getNodeFromGraphicId(uint32_t graphicId) const
{
    std::unordered_set<SafePtr<AGraphNode>> ret = getNodesOnFilter<AGraphNode>(
        [&graphicId](ReadPtr<AGraphNode>& node) {
            return node->getGraphicId() == graphicId;
        }
    );

    if (ret.empty())
        return SafePtr<AGraphNode>();

    assert(ret.size() == 1 && "Same xg::Guid node");
    return (*ret.begin());
}

void OpenScanToolsGraphManager::getMeshInfoForClick(ClickInfo& click)
{
	click.mesh = nullptr;
	click.meshTransfo = glm::dmat4();

	glm::dmat4 meshTransfo;
	ElementType type;
	{
		ReadPtr<AGraphNode> node = click.hover.cget();
		if (!node)
			return;
		type = node->getType();
		meshTransfo = node->getTransformation();
	}

	if (type == ElementType::Box ||
		type == ElementType::Grid ||
		type == ElementType::Cylinder ||
		type == ElementType::Sphere)
	{
		ReadPtr<SimpleObjectNode> rSimple = static_pointer_cast<SimpleObjectNode>(click.hover).cget();
		if (!rSimple)
			return;
		click.mesh = m_meshManager->getGenericMesh(rSimple->getGenericMeshId());
	}

	if (type == ElementType::Torus)
	{
		ReadPtr<TorusNode> rTorus = static_pointer_cast<TorusNode>(click.hover).cget();
		if (!rTorus)
			return;
		click.mesh = m_meshManager->getMesh(rTorus->getMeshId()).m_mesh;
	}

	if (type == ElementType::MeshObject)
	{
		ReadPtr<MeshObjectNode> rMesh = static_pointer_cast<MeshObjectNode>(click.hover).cget();
		if (!rMesh)
			return;
		click.mesh = m_meshManager->getMesh(rMesh->getMeshId()).m_mesh;
	}

	click.meshTransfo = meshTransfo;
}

bool isObjectFiltered(ReadPtr<AGraphNode>& rNode, ObjectStatusFilter filter)
{
	switch (filter)
	{
		case ObjectStatusFilter::ALL:
		{
			return true;
		}
		break;
		case ObjectStatusFilter::VISIBLE:
		{
			return rNode->isVisible();
		}
		break;
		case ObjectStatusFilter::SELECTED:
		{
			return rNode->isSelected();
		}
		break;
		default:
		{
			return true;
		}
		break;
	}
}

/*** From Project ****/

uint32_t OpenScanToolsGraphManager::getNextUserId(ElementType type, const IndexationMethod& indexMethod) const
{
	uint32_t i = 1;

	std::set<uint32_t> allIndex;
	if (indexMethod == IndexationMethod::FillMissingIndex)
	{
		for (const SafePtr<AGraphNode>& node : getNodesByTypes({ type }, ObjectStatusFilter::ALL))
		{
			ReadPtr<AGraphNode> readNode = node.cget();
			if (!readNode)
				continue;
			allIndex.insert(readNode->getUserIndex());
		}
		while (true)
		{
			if (allIndex.find(i) == allIndex.end())
				break;
			else
				++i;
		}
	}
	else
	{
		for (const SafePtr<AGraphNode>& node : getNodesByTypes({ type }, ObjectStatusFilter::ALL))
		{
			
			ReadPtr<AGraphNode> readNode = node.cget();

			if (!readNode)
				continue;
			if(readNode->getUserIndex() >= i)
				i = readNode->getUserIndex() + 1;
		}
	}

	return (i);
}

std::vector<uint32_t> OpenScanToolsGraphManager::getNextMultipleUserId(ElementType type, const IndexationMethod& indexMethod, int indexAmount) const
{
	std::vector<uint32_t> result(0);
	uint32_t i = 1;
	int indexAdded(0);
	std::set<uint32_t> allIndex;
	if (indexMethod == IndexationMethod::FillMissingIndex)
	{
		for (const SafePtr<AGraphNode>& node : getNodesByTypes({ type }, ObjectStatusFilter::ALL))
		{
			ReadPtr<AGraphNode> readNode = node.cget();
			if (!readNode)
				continue;
			allIndex.insert(readNode->getUserIndex());
		}
		while (indexAdded < indexAmount)
		{
			while (true)
			{
				if (allIndex.find(i) == allIndex.end())
					break;
				else
					++i;
			}
			result.push_back(i);
			allIndex.insert(i);
			indexAdded++;
		}
		
	}
	else
	{
		for (const SafePtr<AGraphNode>& node : getNodesByTypes({ type }, ObjectStatusFilter::ALL))
		{

			ReadPtr<AGraphNode> readNode = node.cget();

			if (!readNode)
				continue;
			if (readNode->getUserIndex() >= i)
				i = readNode->getUserIndex() + 1;
		}
		for (int j = 0; j < indexAmount; j++)
			result.push_back(j+i);
	}


	return result;
}

bool OpenScanToolsGraphManager::isIdAvailable(const std::unordered_set<ElementType>& types, uint32_t userid) const
{
	for (const SafePtr<AGraphNode>& node : getNodesByTypes(types, ObjectStatusFilter::ALL))
	{
		ReadPtr<AGraphNode> readNode = node.cget();
		if (readNode && readNode->getUserIndex() == userid)
			return false;
	}
	return true;
}

SafePtr<CameraNode> OpenScanToolsGraphManager::getCameraNode() const
{
	return m_cameraNode;
}

std::unordered_set<SafePtr<AGraphNode>> OpenScanToolsGraphManager::getProjectNodes() const
{
	return getProjectNodesByFilterType(ObjectStatusFilter::ALL);
}

SafePtr<AGraphNode> OpenScanToolsGraphManager::getNodeById(xg::Guid id) const
{
	std::unordered_set<SafePtr<AGraphNode>> sameIdNodes = getNodesOnFilter<AGraphNode>([&id](ReadPtr<AGraphNode>& node) { return node->getId() == id; });
	if (sameIdNodes.empty())
		return SafePtr<AGraphNode>();
	if (sameIdNodes.size() > 1)
		assert(!"Deux fois le même noeud avec le même id dans le graph");

	return *(sameIdNodes.begin());
}

std::unordered_set<SafePtr<AGraphNode>> OpenScanToolsGraphManager::getSelectedNodes() const
{
	return getNodesOnFilter<AGraphNode>([](ReadPtr<AGraphNode>& node) { return node->isSelected(); });
}

std::unordered_set<SafePtr<AGraphNode>> OpenScanToolsGraphManager::getUnSelectedNodes() const
{
	return getNodesOnFilter<AGraphNode>([](ReadPtr<AGraphNode>& node) { return !node->isSelected(); });
}

std::unordered_set<SafePtr<AGraphNode>> OpenScanToolsGraphManager::getProjectNodesByFilterType(ObjectStatusFilter filter) const
{
	return getNodesOnFilter<AGraphNode>([&filter](ReadPtr<AGraphNode>& node) {return isObjectFiltered(node, filter) && node->getGraphType() == AGraphNode::Type::Object; });
}

std::unordered_set<SafePtr<AGraphNode>> OpenScanToolsGraphManager::getNodesByTypes(std::unordered_set<ElementType> types, ObjectStatusFilter filter) const
{
	return getNodesOnFilter<AGraphNode>([&types, &filter](ReadPtr<AGraphNode>& node) { return isObjectFiltered(node, filter) && types.find(node->getType()) != types.end(); });
}

std::unordered_set<SafePtr<AClippingNode>> OpenScanToolsGraphManager::getClippingObjects(bool filterActive, bool filterSelected) const
{
	return getNodesOnFilter<AClippingNode>(
		[](ReadPtr<AGraphNode>& node) 
		{ return s_clippingTypes.find(node->getType()) != s_clippingTypes.end(); },

		[filterActive, filterSelected](ReadPtr<AClippingNode>& clip)
	{ return (!filterSelected || clip->isSelected()) && (!filterActive || clip->isClippingActive()); });
}

std::unordered_set<SafePtr<AClippingNode>> OpenScanToolsGraphManager::getRampObjects(bool filterActive, bool filterSelected) const
{
	return getNodesOnFilter<AClippingNode>(
		[](ReadPtr<AGraphNode>& node)
	{ return s_clippingTypes.find(node->getType()) != s_clippingTypes.end(); },
		[filterActive, filterSelected](ReadPtr<AClippingNode>& clip)
	{ return (!filterSelected || clip->isSelected()) && (!filterActive || clip->isRampActive()); });
}

std::unordered_set<SafePtr<AClippingNode>> OpenScanToolsGraphManager::getActivatedOrSelectedClippingObjects() const
{
	return getNodesOnFilter<AClippingNode>(
		[](ReadPtr<AGraphNode>& node)
		{ return s_clippingTypes.find(node->getType()) != s_clippingTypes.end(); },

		[](ReadPtr<AClippingNode>& clip)
		{return (clip->isClippingActive() || clip->isSelected()); });
}

std::unordered_set<SafePtr<BoxNode>> OpenScanToolsGraphManager::getGrids() const
{
	return getNodesOnFilter<BoxNode>(
		[](ReadPtr<AGraphNode>& node)
		{ return node->getType() == ElementType::Grid; });
}

uint32_t OpenScanToolsGraphManager::getActiveClippingCount() const
{
	return (uint32_t)getClippingObjects(true, false).size();
}

uint32_t OpenScanToolsGraphManager::getActiveClippingAndRampCount() const
{
	uint32_t count = 0;
	getNodesOnFilter<AClippingNode>(
		[](ReadPtr<AGraphNode>& node)
		{ return s_clippingTypes.find(node->getType()) != s_clippingTypes.end(); },

		[&count](ReadPtr<AClippingNode>& clip)
		{
			if (clip->isClippingActive())
				count++;
			if (clip->isRampActive())
				count++;
			return false; 
		});
	return count;
}

uint32_t OpenScanToolsGraphManager::getPCOcounters(const tls::ScanGuid& scan) const
{
	return (uint32_t)getNodesOnFilter<APointCloudNode>(
			[](ReadPtr<AGraphNode>& node)
			{ return node->getType() == ElementType::PCO || node->getType() == ElementType::Scan; },


			[&scan](ReadPtr<APointCloudNode>& node)
			{ return node->getScanGuid() == scan; }
		).size();
}

std::unordered_set<SafePtr<TagNode>> OpenScanToolsGraphManager::getTagsWithTemplate(SafePtr<sma::TagTemplate> tagTemplate) const
{
	return getNodesOnFilter<TagNode>(
		[](ReadPtr<AGraphNode>& node)
		{ return node->getType() == ElementType::Tag; },

		[&tagTemplate](ReadPtr<TagNode>& tagNode)
		{return tagNode->getTemplate() == tagTemplate; });
}

bool OpenScanToolsGraphManager::isFilePathOrScanExists(const std::wstring& name, const std::filesystem::path& filePath) const
{
	//A remplacer avec un scanRootNode ?
	for (const SafePtr<AGraphNode>& objectPtr : AGraphNode::getGeometricChildren(m_root)) //A utiliser autre chose que le lien géometrique
	{
		ElementType type;
		{
			ReadPtr<AGraphNode> rObject = objectPtr.cget();
			if (!rObject)
				continue;
			type = rObject->getType();
		}

		if (type == ElementType::Scan)
		{
			ReadPtr<ScanNode> readScan = static_pointer_cast<ScanNode>(objectPtr).cget();
			if (readScan && (readScan->getName() == name || readScan->getScanPath() == filePath))
				return (true);
		}
	}
	return (false);
}

void OpenScanToolsGraphManager::replaceObjectsSelected(std::unordered_set<SafePtr<AGraphNode>> toSelectDatas)
{
	for (const SafePtr<AGraphNode>& nodePtr : getSelectedNodes())
	{
		WritePtr<AGraphNode> writeNode = nodePtr.get();
		if(writeNode)
			writeNode->setSelected(false);
	}

	for (const SafePtr<AGraphNode>& toSelectData : toSelectDatas)
	{
		WritePtr<AGraphNode> writeNode = toSelectData.get();
		if (writeNode)
			writeNode->setSelected(true);
	}

	AGraphNode::cleanLinks(m_manipulatorNode);
	m_manipulatorNode.reset();

	std::unordered_set<SafePtr<AObjectNode>> toManipObject;
	for (const SafePtr<AGraphNode>& selectData : toSelectDatas)
	{
		{
			ReadPtr<AGraphNode> rNode = selectData.cget();
			if (!rNode || rNode->getGraphType() != AGraphNode::Type::Object || !ManipulatorNode::isAcceptingObjectToManip(rNode->getType()))
				continue;
		}
		toManipObject.insert(static_pointer_cast<AObjectNode>(selectData));
	}

	if (!toManipObject.empty())
	{
		m_manipulatorNode = make_safe<ManipulatorNode>(m_dataDispatcher);
		addNodesToGraph({ m_manipulatorNode });
		{
			WritePtr<ManipulatorNode> wManip = m_manipulatorNode.get();
			wManip->setTarget(toManipObject);
			wManip->setManipulationMode(m_manipulationMode);
		}
	}
}

void OpenScanToolsGraphManager::getClippingAssembly(ClippingAssembly& retAssembly, bool filterActive, bool filterSelected) const
{
	std::unordered_set<SafePtr<AClippingNode>> clippings = getClippingObjects(filterActive, filterSelected);
	for (const SafePtr<AClippingNode>& clip : clippings)
	{
		ReadPtr<AClippingNode> rClip = clip.cget();
		if (!rClip)
			continue;

		// NOTE - Il a été décrété que l'on doit faire l'union des clipping intérieur 
		//        et l'intersection des clipping extérieur.
		//        Bien sûr d'autres assemblages logiques sont possibles mais il faudrait pour cela
		//        créer les opérateurs adéquats et rendre cela lisible pour l'utilisateur.
		rClip->pushClippingGeometries(retAssembly, TransformationModule(*&rClip));
	}
	return;
}

std::unordered_set<SafePtr<ScanNode>> OpenScanToolsGraphManager::getVisibleScans(const tls::ScanGuid& pano) const
{
	return getNodesOnFilter<ScanNode>(
		[](ReadPtr<AGraphNode>& node) {
			return node->getType() == ElementType::Scan; },
		[&pano](ReadPtr<ScanNode>& scan) {
			return ((pano == xg::Guid() && scan->isVisible()) || scan->getScanGuid() == pano); }
			);
}

std::vector<tls::PointCloudInstance> OpenScanToolsGraphManager::getVisiblePointCloudInstances(const tls::ScanGuid& pano, bool scans, bool pcos) const
{
	return getPointCloudInstances(pano, scans, pcos, ObjectStatusFilter::VISIBLE);
}

std::vector<tls::PointCloudInstance> OpenScanToolsGraphManager::getPointCloudInstances(const tls::ScanGuid& pano, bool getScans, bool getPcos, ObjectStatusFilter filterStatus) const
{
	std::vector<tls::PointCloudInstance> result;

	if (pano != xg::Guid())
	{
		std::unordered_set<SafePtr<ScanNode>> scans = 
			getNodesOnFilter<ScanNode>([&pano, &getScans, &getPcos](ReadPtr<AGraphNode>& node)
											{ return node->getType() == ElementType::Scan; },
									   [&pano](ReadPtr<ScanNode>& node)
											{ return (pano != xg::Guid() && node->getScanGuid() == pano); }
			);

		if (scans.size() > 1)
			assert(false);

		SafePtr<ScanNode> scan = *scans.begin();
		ReadPtr<ScanNode> rScan = scan.cget();
		if (!rScan)
			return result;

		tls::ScanHeader header;
		tlGetScanHeader(rScan->getScanGuid(), header);
		result.emplace_back(scan, header, *&rScan, rScan->getClippable());

		// QUESTION - Est-ce que le scan panoramique annule l'export des pcos ?
		return result;
	}

	std::unordered_set<SafePtr<APointCloudNode>> pcs =
		getNodesOnFilter<APointCloudNode>([&getScans, &getPcos, &filterStatus](ReadPtr<AGraphNode>& node)
			{ 
				bool verifType = getScans && node->getType() == ElementType::Scan
					|| getPcos && node->getType() == ElementType::PCO;
				bool verifState = (filterStatus == ObjectStatusFilter::ALL ||
					(filterStatus == ObjectStatusFilter::VISIBLE && node->isVisible()) ||
					(filterStatus == ObjectStatusFilter::SELECTED && node->isSelected()));
				return verifType && verifState;
			}
			);

	for (const SafePtr<APointCloudNode>& pc : pcs)
	{
		ReadPtr<APointCloudNode> rPc = pc.cget();
		if (!rPc)
			continue;

		tls::ScanHeader header;
		tlGetScanHeader(rPc->getScanGuid(), header);
		result.emplace_back(pc, header, rPc->getTransformationModule(), rPc->getClippable());
	}

	return (result);
}

uint64_t OpenScanToolsGraphManager::getProjectPointsCount() const
{
	uint64_t points = 0;
	for (const SafePtr<AGraphNode>& scan : getNodesByTypes({ ElementType::Scan }, ObjectStatusFilter::ALL))
	{
		ReadPtr<ScanNode> rScan = static_pointer_cast<ScanNode>(scan).cget();
		if (!rScan)
			continue;

		points += rScan->getNbPoint();
	}
	return (points);
}

std::unordered_set<SafePtr<AGraphNode>> OpenScanToolsGraphManager::getNodesOnFilter(std::function<bool(const SafePtr<AGraphNode>&)> graphNodeFilter) const
{
	std::unordered_set<SafePtr<AGraphNode>> nodes;

	std::unordered_set<SafePtr<AGraphNode>> geoChildren = AGraphNode::getGeometricChildren_rec(m_root);

	for (const SafePtr<AGraphNode>& graphNode : geoChildren)
	{
		if (!graphNodeFilter(graphNode))
			continue;
		nodes.insert(graphNode);
	}
	return (nodes);
}
