#ifndef GRAPH_MANAGER_H
#define GRAPH_MANAGER_H

#include "models/data/Clipping/ClippingGeometry.h"
#include "models/graph/ManipulatorNode.h"
#include "models/3d/CLickInfo.h"
#include "models/3d/BoundingBox.h"
#include "models/pointCloud/PointCloudInstance.h"
#include "models/application/TagTemplate.h"
#include "models/OpenScanToolsModelEssentials.h"
#include "gui/IPanel.h"
#include "gui/GuiData/IGuiData.h"
#include "pointCloudEngine/TargetMarkerFactory.h"
#include "vulkan/MeshManager.h"

#include <deque>
#include <vector>
#include <unordered_set>

class AClippingNode;
class CameraNode;
class BoxNode;
class ClusterNode;
class ScanNode;
class ScanObjectNode;
class TagNode;

class GraphManager;
class IDataDispatcher;

enum class IndexationMethod;

typedef void (GraphManager::* GraphManagerMethod)(IGuiData*, bool);

class GraphManager : public IPanel
{
public:
	GraphManager(IDataDispatcher& dataDispatcher);
	~GraphManager();

    void informData(IGuiData* iGuiData);
    void refreshScene();
	void cleanProjectObjects(); // Scan, tag, clippings, measures, etc

	SafePtr<AGraphNode> getRoot();
	const SafePtr<ManipulatorNode>& getManipulatorNode();

	const SafePtr<ClusterNode>& getHierarchyMasterCluster() const;
	void setHierarchyMasterCluster(const SafePtr<ClusterNode>& hierarchy);
	void createHierarchyMasterCluster();

	template<class NodeClass> 
	SafePtr<NodeClass> createCopyNode(const NodeClass& nodeClass);

	void addNodesToGraph(const std::unordered_set<SafePtr<AGraphNode>>& nodes);

	template<class MeasureClass>
	SafePtr<MeasureClass> createMeasureNode();

	SafePtr<CameraNode> createCameraNode(const std::wstring& name);
	SafePtr<CameraNode> duplicateCamera(const SafePtr<CameraNode>& camera);

	TargetMarkerFactory& getTargetFactory();
    void deleteClickTargets();

    void setObjectsHovered(std::unordered_set<uint32_t>&& objectsHovered);
    SafePtr<AObjectNode> getSingleHoverObject() const;
    void resetObjectsHovered();

	std::unordered_set<SafePtr<AGraphNode>> getNodesFromGraphicIds(const std::unordered_set<uint32_t>& graphicIds) const;
	SafePtr<AGraphNode> getNodeFromGraphicId(uint32_t graphicId) const;
	void getMeshInfoForClick(ClickInfo& click);

	/** From Project **/

	uint32_t getNextUserId(ElementType type, const IndexationMethod& indexMethod) const;
	std::vector<uint32_t> getNextMultipleUserId(ElementType type, const IndexationMethod& indexMethod, int indexAmount) const;
	bool isIdAvailable(const std::unordered_set<ElementType>& types, uint32_t id) const;

	SafePtr<CameraNode> getCameraNode() const;

	std::unordered_set<SafePtr<AGraphNode>> getProjectNodes() const;
	std::unordered_set<SafePtr<AGraphNode>> getProjectNodesByFilterType(ObjectStatusFilter filter) const;
	SafePtr<AGraphNode> getNodeById(xg::Guid id) const;

	std::unordered_set<SafePtr<AGraphNode>> getSelectedNodes() const;
	std::unordered_set<SafePtr<AGraphNode>> getUnSelectedNodes() const;
	std::unordered_set<SafePtr<AGraphNode>> getNodesByTypes(std::unordered_set<ElementType> types, ObjectStatusFilter filter = ObjectStatusFilter::ALL) const;

	std::unordered_set<SafePtr<AClippingNode>> getClippingObjects(bool filterActive, bool filterSelected) const;
	std::unordered_set<SafePtr<AClippingNode>> getRampObjects(bool filterActive, bool filterSelected) const;
	std::unordered_set<SafePtr<AClippingNode>> getActivatedOrSelectedClippingObjects() const;

	std::unordered_set<SafePtr<BoxNode>> getGrids() const;
	std::unordered_set<SafePtr<TagNode>> getTagsWithTemplate(SafePtr<sma::TagTemplate> tagTemplate) const;

	uint32_t getActiveClippingCount() const;
	uint32_t getActiveClippingAndRampCount() const;

	uint32_t getPCOcounters(const tls::ScanGuid& scan) const;
	bool isFilePathOrScanExists(const std::wstring& name, const std::filesystem::path& filePath) const;

	void replaceObjectsSelected(std::unordered_set<SafePtr<AGraphNode>> toSelectDatas);

	void getClippingAssembly(ClippingAssembly& retAssembly, bool filterActive, bool filterSelected) const;

	BoundingBoxD getScanBoundingBox(ObjectStatusFilter status) const;
	std::unordered_set<SafePtr<ScanNode>> getVisibleScans(const tls::ScanGuid& pano) const;
	std::vector<tls::PointCloudInstance> getVisiblePointCloudInstances(const tls::ScanGuid& pano, bool scans, bool pcos) const;
	std::vector<tls::PointCloudInstance> getPointCloudInstances(const tls::ScanGuid& pano, bool scans, bool pcos, ObjectStatusFilter status) const;
	uint64_t getProjectPointsCount() const;

	template<typename T>
	std::unordered_set<SafePtr<T>> getNodesOnFilter(
		std::function<bool(ReadPtr<AGraphNode>&)> graphNodeFilter,
		std::function<bool(ReadPtr<T>&)> objectFilter = [](ReadPtr<T>&) {return true;}
	) const;
	std::unordered_set<SafePtr<AGraphNode>> getNodesOnFilter(
		std::function<bool(const SafePtr<AGraphNode>&)> graphNodeFilter
	) const;

	/******/
private:
	void onFunctionChanged(IGuiData* iGuiData, bool store);
	void onClick(IGuiData* iGuiData, bool store);
	void onManipulationMode(IGuiData* data, bool store);

	void detachManipulator(AObjectNode* parent);

	inline void registerGuiDataFunction(guiDType type, GraphManagerMethod fct);

private:
	SafePtr<AGraphNode> m_root;
    std::mutex m_mutex;
    std::deque<IGuiData*> m_waitingDataToProceed;

	SafePtr<ManipulatorNode>								m_manipulatorNode;
	SafePtr<CameraNode>										m_cameraNode;
	ManipulationMode										m_manipulationMode = ManipulationMode::Translation;
	SafePtr<ClusterNode>									m_hierarchyMasterCluster;
	IDataDispatcher&										m_dataDispatcher;
	MeshManager*											m_meshManager;
	std::unordered_map<guiDType, GraphManagerMethod>	m_methods;

    std::unordered_set<SafePtr<AObjectNode>>                m_objectsHovered;
	std::unordered_set<uint32_t>							m_selectedIds;

	// TargetFactory
	TargetMarkerFactory         m_targetMarkerFactory;

	//From Project
	std::unordered_set<xg::Guid>							m_projectAuthorsId;

};

#endif //! GRAPH_MANAGER_H