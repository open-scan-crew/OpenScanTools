#include "controller/functionSystem/ContextCreateTag.h"
#include "controller/controls/ControlFunctionTag.h"
#include "models/graph/GraphManager.h"
#include "models/graph/BoxNode.h"
#include "models/graph/ClusterNode.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/functionSystem/ContextPeopleRemover.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/Texts.hpp"
#include "pointCloudEngine/TlScanOverseer.h"
#include "pointCloudEngine/OctreeRayTracing.h"
#include "controller/controls/ControlFunction.h"
#include "utils/Logger.h"

using namespace std::chrono;

ContextPeopleRemover::ContextPeopleRemover(const ContextId& id)
	: ARayTracingContext(id), m_currentIndexCluster(0)
{
	m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, "" });
}

ContextPeopleRemover::~ContextPeopleRemover()
{
}

ContextState ContextPeopleRemover::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextPeopleRemover::feedMessage(IMessage* message, Controller& controller)
{
	ARayTracingContext::feedMessage(message, controller);
	return m_state;
}

ContextState ContextPeopleRemover::launch(Controller& controller)
{
	if (m_currentIndexCluster == 0)
	{
		Logger::log(LoggerMode::rayTracingLog) << "people remover starting..." << Logger::endl;
		auto start = high_resolution_clock::now();
		ClippingAssembly clippingAssembly;
		controller.getGraphManager().getClippingAssembly(clippingAssembly, true, false);

		TlScanOverseer::setWorkingScansTransfo(controller.getGraphManager().getVisiblePointCloudInstances(m_panoramic, true, true));
		double voxelSize = 0.1;
	
		int numberOfScans = TlScanOverseer::getInstance().getNumberOfScans();
		OctreeVoxelGrid octreeVoxelGrid(voxelSize, clippingAssembly, numberOfScans);
		m_voxelSize = octreeVoxelGrid.m_voxelSize;
		m_maxSize = octreeVoxelGrid.m_maxSize;
		VoxelGrid voxelGrid(m_voxelSize, clippingAssembly,m_maxSize/2);

		TlScanOverseer::getInstance().createOctreeVoxelGrid(octreeVoxelGrid);
		TlScanOverseer::getInstance().copyOctreeIntoGrid(octreeVoxelGrid, voxelGrid);
		//TlScanOverseer::getInstance().displayOctreeVoxelGrid(octreeVoxelGrid);

		std::vector<bool> temp(voxelGrid.m_sizeZ, false);
		std::vector<std::vector<bool>> temp1(voxelGrid.m_sizeY, temp);
		std::vector<std::vector<std::vector<bool>>> dynamicVoxels(voxelGrid.m_sizeX, temp1);
		/*std::vector<bool> temp((1 << octreeVoxelGrid.m_maxDepth), false);
		std::vector<std::vector<bool>> temp1((1 << octreeVoxelGrid.m_maxDepth), temp);
		std::vector<std::vector<std::vector<bool>>> dynamicVoxels((1 << octreeVoxelGrid.m_maxDepth), temp1);*/
		//TlScanOverseer::getInstance().classifyOctreeVoxels(octreeVoxelGrid, dynamicVoxels);
		/*TlScanOverseer::getInstance().createVoxelGrid(voxelGrid, clippingAssembly);
		
		m_totalVolume = voxelGrid.m_sizeX * voxelGrid.m_sizeY * voxelGrid.m_sizeZ;
		//TlScanOverseer::getInstance().displayVoxelGrid(voxelGrid);*/
		
		TlScanOverseer::getInstance().classifyVoxels(voxelGrid, clippingAssembly, dynamicVoxels);
		std::vector<glm::dvec3> centers;
		std::vector<int> sizes;
	
		int numberOfDisplayedBoxes = 0;


		// start the clustering and merge process

		std::vector<std::vector<std::vector<int>>> clusterLabels;
		int numberOfClusters(0), clusteringThreshold(6);
		std::vector<int> holesInLabels;
		std::vector<ClusterInfo> clusterInfo;
		//TlScanOverseer::getInstance().createClustersOfDynamicOctreeVoxels(octreeVoxelGrid, dynamicVoxels, clusterLabels, numberOfClusters, clusterInfo, clusteringThreshold);

		TlScanOverseer::getInstance().createClustersOfDynamicVoxels(voxelGrid, dynamicVoxels, clusterLabels, numberOfClusters,clusterInfo,clusteringThreshold);
		std::vector<std::vector<std::vector<int>>> boxCoverForAllClusters(0);
		
		int totalBoxesForCovering(0),totalVolume(0),clustersCovered(0);
		auto start1 = high_resolution_clock::now();

		for (int i = 1; i < numberOfClusters + 1; i++)
		{
			//check if i is a hole
			bool isAHole(false);
			for (int t = 0; t < (int)holesInLabels.size(); t++)
				if (i == holesInLabels[t])
				{
					isAHole = true;
					break;
				}
			if (isAHole)
				continue;
			/*if (!clusterInfo[i - 1].isTrueDynamic())
			{
				holesInLabels.push_back(i);
				continue;
			}*/
			//Logger::log(LoggerMode::rayTracingLog) << "current cluster : " << clustersCovered << Logger::endl;
			//Logger::log(LoggerMode::rayTracingLog) << "volume : " << clusterInfo[i-1].m_volume << Logger::endl;


			std::vector<std::vector<int>> boxCover = TlScanOverseer::getInstance().coverClusterWithBoxes(clusterLabels, i, dynamicVoxels, voxelGrid, clusterInfo[i-1]);
			//std::vector<std::vector<int>> boxCover = TlScanOverseer::getInstance().coverClusterWithBoxesOctree(clusterLabels, i, dynamicVoxels, octreeVoxelGrid, clusterInfo[i - 1]);
			clustersCovered++;

			//if(clustersCovered%100 == 0)
			//	Logger::log(LoggerMode::rayTracingLog) << "clusters covered : " << clustersCovered << Logger::endl;

			boxCoverForAllClusters.push_back(boxCover);
			totalBoxesForCovering += (int)boxCover.size();
			m_clustersInfo.push_back(clusterInfo[i - 1]);
			totalVolume += clusterInfo[i - 1].m_volume;
		}
		auto stop1 = high_resolution_clock::now();
		auto duration1 = duration_cast<milliseconds>(stop1 - start1);
		
		//Logger::log(LoggerMode::rayTracingLog) << "totalVolume of Clusters : " << totalVolume << Logger::endl;
		//Logger::log(LoggerMode::rayTracingLog) << "time to cover clusters " << duration1 << Logger::endl;
		m_boxCoverForAllClusters = boxCoverForAllClusters;
		m_totalCluster = (int)boxCoverForAllClusters.size();

		//count trueDynamic and falsePositives
		m_trueDynamic = 0;
		m_falsePositives = 0;
		for (int i = 0; i < (int)m_clustersInfo.size(); i++)
		{
			if (m_clustersInfo[i].isTrueDynamic())
				m_trueDynamic++;
			else m_falsePositives++;
		}
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<seconds>(stop - start);

		/*Logger::log(LoggerMode::rayTracingLog) << "dynamic : " << countDynamic << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << "non empty : " << countNonEmpty << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << "total : " << voxelGrid.m_sizeX * voxelGrid.m_sizeY * voxelGrid.m_sizeZ << Logger::endl;
		double totalVolume = voxelGrid.m_sizeX * voxelGrid.m_sizeY * voxelGrid.m_sizeZ * voxelGrid.m_voxelSize * voxelGrid.m_voxelSize * voxelGrid.m_voxelSize;
		Logger::log(LoggerMode::rayTracingLog) << "ratio dynamic/non empty : " << (double)countDynamic / countNonEmpty << Logger::endl;
		//Logger::log(LoggerMode::rayTracingLog) << "vertical boxes total : " << centers.size() << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << "total volume: " << totalVolume << " m^3" << Logger::endl;*/
		Logger::log(LoggerMode::rayTracingLog) << "time taken : " << duration.count() << " seconds" << Logger::endl;
		//Logger::log(LoggerMode::rayTracingLog) << "time per m^3 : " << duration.count() / totalVolume << " seconds / m^3" << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << "totalBoxesToCover : " << totalBoxesForCovering << Logger::endl;

	}
	
	//create 2 master clusters : trueDynamic, and falsePositives
	for (int clusterIndex = 0; clusterIndex < m_totalCluster; ++clusterIndex)
	{
		SafePtr<ClusterNode> cluster = make_safe<ClusterNode>();
		m_clusters.push_back(cluster);
		{
			WritePtr<ClusterNode> wCluster = cluster.get();
			//if (!wCluster)
			//	return (m_state = ContextState::abort);
			wCluster->setTreeType(TreeType::Boxes);
			wCluster->setColor({ 0, 0, 0, 1 });

			if (clusterIndex == 0)
			{
				wCluster->setName(L"trueDynamic");
			}
			else if (clusterIndex == 1)
			{
				wCluster->setName(L"falsePositive");
			}
			//create true Dynamic clusters
			else if (clusterIndex < (2 + m_trueDynamic))
			{
				wCluster->setName(L"dynamic objects cluster : " + std::to_wstring(clusterIndex - 1));
	}
			//create falsePositive clusters
			else
			{
				wCluster->setName(L"dynamic objects cluster : " + std::to_wstring(clusterIndex - 1));
			}
		}
	
		// WARNING - Release the WritePtr before editing the link. If not, this is a deadlock.
		if (clusterIndex > 1 && clusterIndex < (2 + m_trueDynamic))
		{
			AGraphNode::addOwningLink(m_clusters[0], cluster);
		}
		//create falsePositive clusters
		else if (clusterIndex > 1)
		{
			AGraphNode::addOwningLink(m_clusters[1], cluster);
		}
		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(cluster));
	}
	
	//for create cluster
	if (m_totalCluster==0)
		return waitForNextPoint(controller);
	/*Logger::log(LoggerMode::rayTracingLog) << "m_totalCluster : " << m_totalCluster << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "m_currentIndexCluster : " << m_currentIndexCluster << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "m_clustersId.size() : " << m_clustersId.size() << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "m_trueDynamic : " << m_trueDynamic << Logger::endl;*/
	// ------- DEPRECATED --------
	/*
	if (m_currentIndexCluster < (m_totalCluster +2))
	{

		//create 2 master clusters : trueDynamic, and falsePositives

		if (m_currentIndexCluster == 0)
		{
			controller.getControlListener()->notifyUIControl(new control::tree::CreateCluster(TreeType::Boxes, INVALID_TREE_ID, "trueDynamic", { 0,0,0,1 }, 1));
			m_state = ContextState::waiting_for_input;
			return m_state;
		}
		
		else if (m_currentIndexCluster == 1)
		{
			controller.getControlListener()->notifyUIControl(new control::tree::CreateCluster(TreeType::Boxes, INVALID_TREE_ID, "falsePositive", { 0,0,0,1 }, 1));
			m_state = ContextState::waiting_for_input;
			return m_state;
		}
		//create true Dynamic clusters
		else if (m_currentIndexCluster < (2 + m_trueDynamic))
		{
			controller.getControlListener()->notifyUIControl(new control::tree::CreateCluster(TreeType::Boxes, m_clustersId[0], "dynamic objects cluster : " + std::to_string(m_currentIndexCluster - 1), { 0,0,0,1 }, 1));
			m_state = ContextState::waiting_for_input;
			return m_state;
		}
		//create falsePositive clusters
		else
		{
			controller.getControlListener()->notifyUIControl(new control::tree::CreateCluster(TreeType::Boxes, m_clustersId[1], "dynamic objects cluster : " + std::to_string(m_currentIndexCluster - 1), { 0,0,0,1 }, 1));
			m_state = ContextState::waiting_for_input;
			return m_state;
		}

	}
	*/
	Logger::log(LoggerMode::rayTracingLog) << "clusters created : " << m_totalCluster << Logger::endl;
	
	int numberOfDisplayedBoxes(0),currTrueDynamic(0),currFalsePositives(0);
	std::set<int> clusteredCreated;
	
	for (int i = 0; i < (int)m_boxCoverForAllClusters.size(); i++)
	{
		//find biggest volume
		
		int currMaxVolume(0), biggestCluster(0);
		for (int j = 0; j < (int)m_clustersInfo.size(); j++)
		{
			if (clusteredCreated.find(j) != clusteredCreated.end())
			{ }
			else if (m_clustersInfo[j].m_volume > currMaxVolume)
				{
					currMaxVolume = m_clustersInfo[j].m_volume;
					biggestCluster = j;
				}
		}
		clusteredCreated.insert(biggestCluster);

		//create boxes in that cluster
		//if (!m_clustersInfo[biggestCluster].isTrueDynamic())
		//	continue;
		
		for (int t = 0; t < (int)m_boxCoverForAllClusters[biggestCluster].size(); t++)
		{
			if (numberOfDisplayedBoxes > 5000)
				break;
			numberOfDisplayedBoxes++;
			SafePtr<BoxNode> box1 = make_safe<BoxNode>(true);
			
			TransformationModule mod;
			ClippingAssembly clippingAssembly;
			controller.getGraphManager().getClippingAssembly(clippingAssembly, true, false);
			glm::dmat4 mat = glm::inverse(clippingAssembly.clippingUnion[0]->matRT_inv);
			mod.setTransformation(mat);
			
			mod.setScale(glm::dvec3(m_voxelSize*0.5*(m_boxCoverForAllClusters[biggestCluster][t][1]- m_boxCoverForAllClusters[biggestCluster][t][0]), m_voxelSize * 0.5 *(m_boxCoverForAllClusters[biggestCluster][t][3] - m_boxCoverForAllClusters[biggestCluster][t][2]), m_voxelSize * 0.5 *(m_boxCoverForAllClusters[biggestCluster][t][5] - m_boxCoverForAllClusters[biggestCluster][t][4])));
			glm::dvec3 position= glm::dvec3(-m_maxSize/2 + 0.5 * (m_boxCoverForAllClusters[biggestCluster][t][0] + m_boxCoverForAllClusters[biggestCluster][t][1]) * m_voxelSize, -m_maxSize / 2 + 0.5 * (m_boxCoverForAllClusters[biggestCluster][t][2] + m_boxCoverForAllClusters[biggestCluster][t][3]) * m_voxelSize, -m_maxSize / 2 + 0.5 * (m_boxCoverForAllClusters[biggestCluster][t][4] + m_boxCoverForAllClusters[biggestCluster][t][5]) * m_voxelSize);

			glm::dvec4 temp(position[0], position[1], position[2], 1.0);
			
			glm::dvec4 temp1 = mat * temp;
			position=glm::dvec3(temp1[0], temp1[1], temp1[2]);
			mod.setPosition(position);
			{
				WritePtr<BoxNode> wBox1 = box1.get();
				wBox1->setTransformationModule(mod);
				wBox1->setDescription_str("volume : " + std::to_string(m_clustersInfo[biggestCluster].m_volume) + "   , emptyNeighbors : " + std::to_string(m_clustersInfo[biggestCluster].m_emptyNeighbors) + "   , staticNeighbors : " + std::to_string(m_clustersInfo[biggestCluster].m_staticNeighbors));
				wBox1->setVisible(true);
				wBox1->setSelected(false);
				wBox1->setClippingActive(false);
			time_t timeNow;
				wBox1->setCreationTime(time(&timeNow));
				if (!m_clustersInfo[biggestCluster].isTrueDynamic())
					wBox1->setColor(Color32(250, 0, 0));
			}
			// Remplace la fonction CreateClippingBoxInCluster
			int clusterOffset = m_clustersInfo[biggestCluster].isTrueDynamic() ? currTrueDynamic + 2 : m_trueDynamic + currFalsePositives + 2;
			AGraphNode::addOwningLink(m_clusters[clusterOffset], box1);
			controller.getControlListener()->notifyUIControl(new control::function::AddNodes(box1));
			/*
			if (m_clustersInfo[biggestCluster].isTrueDynamic())
			{
				controller.getControlListener()->notifyUIControl(new control::function::clipping::CreateClippingBoxInCluster(box1, m_clustersId[currTrueDynamic+2]));
			}
			else
			{
				Color32 color(250, 0, 0);
				box1->setColor(color);
				controller.getControlListener()->notifyUIControl(new control::function::clipping::CreateClippingBoxInCluster(box1, m_clustersId[m_trueDynamic + currFalsePositives+2]));
			}*/
		}
		if (m_clustersInfo[biggestCluster].isTrueDynamic())
			currTrueDynamic++;
		else currFalsePositives++;
	}
	Logger::log(LoggerMode::rayTracingLog) << "box created done " << Logger::endl;
	
	//create boxes from vertical merge
	/*for (int i = 0; i < (int)centers.size(); i++)
	{
		if (numberOfDisplayedBoxes > 3000)
			break;
		numberOfDisplayedBoxes++;
		Box* box1 = new Box(controller.getContext());

		TransformationModule mod;
		mod.setScale(glm::dvec3(0.025, 0.025, 0.025*sizes[i]));
		
		mod.setPosition(centers[i]);

		box1->setTransformationModule(mod);


		box1->setVisible(true);
		box1->setSelected(false);
		box1->setClippingActive(false);
		box1->setClippingMode(controller.getContext().getDefaultClippingMode());
		time_t timeNow;
		box1->setCreationTime(time(&timeNow));
		controller.getControlListener()->notifyUIControl(new control::function::clipping::CreateClippingBox(box1));
	}*/
	

	/*Logger::log(LoggerMode::rayTracingLog) << "bucket 0 : " << (int)dynamicVoxelsBucketed[0].size() << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "bucket 1 : " << (int)dynamicVoxelsBucketed[1].size() << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "bucket 2 : " << (int)dynamicVoxelsBucketed[2].size() << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "bucket 3 : " << (int)dynamicVoxelsBucketed[3].size() << Logger::endl;*/

	m_currentIndexCluster = 0;
	//m_clustersId.clear();
	return waitForNextPoint(controller);
}

bool ContextPeopleRemover::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextPeopleRemover::getType() const
{
	return (ContextType::peopleRemover);
}
