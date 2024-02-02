#include "pointCloudEngine/NormalEstimation.h"
#include "utils/Logger.h"
#include <ctime>


OctreeNormal::OctreeNormal(OctreeBase const& base, const uint64_t& pointDataOffset, TlScanFile file, glm::dmat4 matrixToGlobal)
	: OctreeRayTracing(base, pointDataOffset, file, matrixToGlobal)

{
	m_pointCounts = std::vector<int>(m_vTreeCells.size());
	m_centerOfMass = std::vector<glm::dvec3>(m_vTreeCells.size());
}

OctreeNormal::OctreeNormal(const OctreeRayTracing& octree): OctreeRayTracing(octree)
{
	m_pointCounts = std::vector<int>(m_vTreeCells.size());
	m_centerOfMass = std::vector<glm::dvec3>(m_vTreeCells.size());
}

OctreeNormal::~OctreeNormal()
{}

bool OctreeNormal::computeCoherentNeighborhoodInCell(const uint32_t& cellId, std::vector<LocalPlane>& neighborhoods, const double& threshold)
{
	srand((unsigned int)time(NULL));
	TreeCell cell = m_vTreeCells[cellId];
	double radius = cell.m_size*0.707;
	
	std::vector<glm::dvec3> cellPoints, availablePoints;
	std::vector<uint32_t> leavesId;
	getLeavesUnderCell(cellId, leavesId);
	
	if (m_pointCounts[cellId] < 500)
	{
		cellPoints = getPointsFromCell(cellId);
	}
	else
	{
		cellPoints = getRandomPointsInLeaves(leavesId, 500);
	}


	for (int i = 0; i < (int)cellPoints.size(); i++)
	{
		bool isAvailable(true);
		for (int j = 0; j < (int)neighborhoods.size(); j++)
		{
			if (isPointInLocalPlane(cellPoints[i], neighborhoods[j], threshold))
			{
				isAvailable = false;
				break;
			}
		}
		if (isAvailable)
			availablePoints.push_back(cellPoints[i]);
	}
	int relevantCount = (int)availablePoints.size();
	if (relevantCount < 50)
		return false;
	for (int tryOut = 0; tryOut < 10; tryOut++)
	{
		std::vector<glm::dvec3> planePoints;
		for (int i = 0; i < 3; i++)
		{
			int v = rand() % relevantCount;
			planePoints.push_back(availablePoints[v]);
		}
		
		std::vector<double> plane;
		if (!fitPlane(planePoints, plane))
			continue;
		
		int pointsInPlane = countPointsNearPlane(plane, threshold, availablePoints);
		if (2*pointsInPlane > (int)availablePoints.size())
		{
			LocalPlane localPlane(plane, m_centerOfMass[cellId], radius);
			neighborhoods.push_back(localPlane);
			Logger::log(LoggerMode::rayTracingLog) << "plane " << neighborhoods.size() << " found ! tryOut " << tryOut << Logger::endl;
			return true;
		}
	}
	return false;
	//Logger::log(LoggerMode::rayTracingLog) << "availablePoints end" << Logger::endl;


	/*std::vector<double> plane;
	if (!fitPlane(availablePoints, plane))
		return;		//probably should use hough transform here
	int pointsInPlane = countPointsNearPlane(plane, threshold, availablePoints);
	if (2 * pointsInPlane > (int)availablePoints.size())
	{
		LocalPlane localPlane(plane,m_centerOfMass[cellId],radius);
		neighborhoods.push_back(localPlane);
		Logger::log(LoggerMode::rayTracingLog) << "plane found! scale : " << radius << " , planes found thus far : " << neighborhoods.size() << Logger::endl;
	}*/
}

std::vector<LocalPlane> OctreeNormal::computeCoherentNeighborhoods(const double& threshold, std::vector<uint32_t>& nonPlanarLeafIds)
{
	std::vector<LocalPlane> result;
	std::vector<uint32_t> cellsLeftToCompute;
	cellsLeftToCompute.push_back(m_uRootCell);
	int totalCells(0);
	while (cellsLeftToCompute.size() > 0)
	{
		std::vector<uint32_t> nextCells;
		for (int i = 0; i < (int)cellsLeftToCompute.size(); i++)
		{
			bool test=computeCoherentNeighborhoodInCell(cellsLeftToCompute[i], result, threshold);
			totalCells++;
			
			if(totalCells%100==0)
				Logger::log(LoggerMode::rayTracingLog) << "cells computed : " << totalCells << Logger::endl;
			if (test)
				continue;
			for (int j = 0; j < 8; j++)
			{
				if (m_vTreeCells[cellsLeftToCompute[i]].m_children[j] != NO_CHILD)
				{
					nextCells.push_back(m_vTreeCells[cellsLeftToCompute[i]].m_children[j]);
				}
			}
			/*if (test)
				continue;*/
			if (m_vTreeCells[cellsLeftToCompute[i]].m_isLeaf)
			{
				nonPlanarLeafIds.push_back(cellsLeftToCompute[i]);
				Logger::log(LoggerMode::rayTracingLog) << "non planar leaf size : " << m_vTreeCells[cellsLeftToCompute[i]].m_size << Logger::endl;
			}


		}
		cellsLeftToCompute = nextCells;
	}
	Logger::log(LoggerMode::rayTracingLog) << "END : total cells : " << totalCells << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "number of local planes : " << result.size() << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "total non planar leaves : " << nonPlanarLeafIds.size() << Logger::endl;

	int nonPlanarPoints(0);
	double volume(0);
	for (int i = 0; i < (int)nonPlanarLeafIds.size(); i++)
	{
		nonPlanarPoints += m_pointCounts[nonPlanarLeafIds[i]];
		volume += m_vTreeCells[nonPlanarLeafIds[i]].m_size*m_vTreeCells[nonPlanarLeafIds[i]].m_size*m_vTreeCells[nonPlanarLeafIds[i]].m_size;
	}
	Logger::log(LoggerMode::rayTracingLog) << "total non planar points : " << nonPlanarPoints << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "average density : " << (double)nonPlanarPoints/volume << Logger::endl;


	estimateResult(result, threshold);

	computeNormalsInNonPlanarLeaves(nonPlanarLeafIds, result,threshold);
	Logger::log(LoggerMode::rayTracingLog) << "number of local planes : " << result.size() << Logger::endl;

	return result;
}

bool OctreeNormal::isPointInLocalPlane(const glm::dvec3& point, const LocalPlane& localPlane, const double& threshold)
{
	if (glm::length(point - localPlane.m_center) < localPlane.m_radius)
	{
		if (pointToPlaneDistance(point, localPlane.m_plane) < threshold)
			return true;
	}
	return false;
	/*if (pointToPlaneDistance(point, localPlane.m_plane) < threshold)
		return true;
	return false;*/
}

int OctreeNormal::countPointsNearPlane(const std::vector<double>& plane, const double& threshold, const std::vector<glm::dvec3>& points)
{
	int result(0);
	for (int i = 0; i < (int)points.size(); i++)
	{
		if (pointToPlaneDistance(points[i], plane) < threshold)
			result++;
	}
	return result;
}

void OctreeNormal::centerOfMassLeaf(const uint32_t& cellId)
{
	glm::dvec3 centerOfMass(0.0, 0.0, 0.0);
	int pointsInCell(0);
	TreeCell cell = m_vTreeCells[cellId];
	if (cell.m_isLeaf)
	{
		std::vector<glm::dvec3> leafPoints = listPointsFromLeaf(cell);
		pointsInCell = (int)leafPoints.size();
		for (int i = 0; (int)i < leafPoints.size(); i++)
		{
			centerOfMass += leafPoints[i];
		}
		centerOfMass /= pointsInCell;
		m_pointCounts[cellId] = pointsInCell;
		m_centerOfMass[cellId] = centerOfMass;

		return;
	}
}

void OctreeNormal::fillSkeletonRecursive(const uint32_t& cellId)
{
	TreeCell cell = m_vTreeCells[cellId];
	if (cell.m_isLeaf)
		centerOfMassLeaf(cellId);
	else
	{
		for (int i = 0; i < 8; i++)
		{
			if (cell.m_children[i] != NO_CHILD)
			{
				fillSkeletonRecursive(cell.m_children[i]);
			}
		}
		int pointsInCell(0);
		glm::dvec3 centerOfMass(0.0, 0.0, 0.0);
		for (int i = 0; i < 8; i++)
		{
			if (cell.m_children[i] != NO_CHILD)
			{
				int currPointSize = m_pointCounts[cell.m_children[i]];
				glm::dvec3 currCenterOfMass = m_centerOfMass[cell.m_children[i]];
				centerOfMass += (double)currPointSize * currCenterOfMass;
				pointsInCell += currPointSize;
			}
		}
		centerOfMass /= pointsInCell;
		m_pointCounts[cellId] = pointsInCell;
		m_centerOfMass[cellId] = centerOfMass;
	}
	return;
}

void OctreeNormal::fillSkeleton()
{
	fillSkeletonRecursive(m_uRootCell);
	Logger::log(LoggerMode::rayTracingLog) << "skeleton filled" << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "skeleton size : "<< m_pointCounts.size() << Logger::endl;

}

LocalPlane::LocalPlane(const std::vector<double>& plane, const glm::dvec3& center, const double& radius)
{
	m_plane = plane;
	m_center = center;
	m_radius = radius;
}

LocalPlane::~LocalPlane()
{}

void OctreeNormal::getLeavesUnderCell(const uint32_t& cellId, std::vector<uint32_t>& result)
{
	TreeCell cell = m_vTreeCells[cellId];
	if (cell.m_isLeaf)
		result.push_back(cellId);
	else
	{
		for (int i = 0; i < 8; i++)
		{
			if (cell.m_children[i] != NO_CHILD)
				getLeavesUnderCell(cell.m_children[i], result);
		}
	}
}

std::vector<glm::dvec3> OctreeNormal::getRandomPointsInLeaves(const std::vector<uint32_t>& leavesId, const int& numberOfPoints)
{
	int totalPoints(0);
	for (int i = 0; i < (int)leavesId.size(); i++)
	{
		totalPoints += m_pointCounts[leavesId[i]];
	}
	std::vector<int> indexList;
	for (int loop = 0; loop < numberOfPoints; loop++)
	{
		//srand((unsigned int)time(NULL));
		int v1 = rand() % 10000;
		int index;
		if (totalPoints > 10000)
		{
			int v2 = rand() % (totalPoints / (int)10000);
			index = (v1 + 10000 * v2) % totalPoints;
		}
		else
			index = v1 % totalPoints;
		
		indexList.push_back(index);			//insert stuff to pick points at random instead
	}
	std::sort(indexList.begin(), indexList.end());
	std::vector<std::pair<uint32_t, std::vector<int>>> pointRequest;
	int count(m_pointCounts[leavesId[0]]);
	int leafIndex(0),previousCount(0);
	for (int i = 0; i < (int)indexList.size(); i++)
	{
		bool newLeaf(false);
		int currIndex = indexList[i];
		if (i > 0)
		{
			if (currIndex == indexList[i-1])
				continue;
		}
		while (currIndex > count)
		{
			leafIndex++;
			previousCount = count;
			count += m_pointCounts[leavesId[leafIndex]];
			newLeaf = true;
		}
		if ((newLeaf) || (i == 0))
		{
			std::vector<int> temp;
			temp.push_back(indexList[i] - previousCount);
			pointRequest.push_back(make_pair(leavesId[leafIndex], temp));
		}
		else pointRequest[(int)pointRequest.size() - 1].second.push_back(indexList[i] - previousCount);
	}

	//pointRequest is filled up

	std::vector<TreeCell> relevantCells;
	for (int i = 0; i < (int)pointRequest.size(); i++)
	{
		relevantCells.push_back(m_vTreeCells[pointRequest[i].first]);
	}
	getCellsPointsFromFile(relevantCells);
	std::vector<glm::dvec3> result;
	for (int i = 0; i < (int)pointRequest.size(); i++)
	{
		PointXYZIRGB *localPoint_ = m_decodedBuffers[i];
		for (int j = 0; j < (int)pointRequest[i].second.size(); j++)
		{
			glm::dvec3 localPoint = glm::dvec3(localPoint_[pointRequest[i].second[j]].x, localPoint_[pointRequest[i].second[j]].y, localPoint_[pointRequest[i].second[j]].z);
			result.push_back(pointLocalToGlobal(localPoint, m_globalMatrix));		
		}
	}
	return result;		
}

std::vector<glm::dvec3> OctreeNormal::getPointsFromCell(const uint32_t& cellId)
{
	std::vector<uint32_t> leavesId;
	getLeavesUnderCell(cellId, leavesId);
	std::vector<glm::dvec3> result, temp;
	for (int i = 0; i < (int)leavesId.size(); i++)
	{
		temp = listPointsFromLeaf(m_vTreeCells[leavesId[i]]);
		result.insert(result.end(), temp.begin(), temp.end());
	}
	return result;
}

void OctreeNormal::estimateResult(const std::vector<LocalPlane>& neighborhoods, const double& threshold)
{
	std::vector<uint32_t> temp;
	getLeavesUnderCell(m_uRootCell, temp);
	
	int count(0);
	std::vector<glm::dvec3> testPoints = getRandomPointsInLeaves(temp, 10000);
	for (int i = 0; i < (int)testPoints.size(); i++)
	{
		for (int j = 0; j < (int)neighborhoods.size(); j++)
		{
			if (isPointInLocalPlane(testPoints[i], neighborhoods[j],threshold))
			{
				count++;
				break;
			}
		}
	}
	Logger::log(LoggerMode::rayTracingLog) << "estimate result : " << count << " / " << 10000 << Logger::endl;

}

void OctreeNormal::computeNormalsInNonPlanarLeaves(const std::vector<uint32_t>& nonPlanarLeafIds, std::vector<LocalPlane>& neighborhoods, const double& threshold)
{
	double radius = 0.01;
	for (int i = 0; i < (int)nonPlanarLeafIds.size(); i++)
	{
		int pointProcessed(0),newPlanes(0);
		double averageBallSize(0);
		std::vector<glm::dvec3> points = listPointsFromLeaf(m_vTreeCells[nonPlanarLeafIds[i]]);
		for (int j = 0; j < (int)points.size(); j++)
		{
			if (isPointInNeighborhoods(points[j], neighborhoods, threshold))
				continue;
			pointProcessed++;
			std::vector<glm::dvec3> smallBall = getBallPointsFromList(points, points[j], radius);
			averageBallSize += smallBall.size();
			std::vector<double> plane;
			if (fitPlane(smallBall, plane))
			{
				LocalPlane localPlane(plane, points[j], radius);
				neighborhoods.push_back(localPlane);
				newPlanes++;
			}
		}
		averageBallSize /= (double)pointProcessed;
		Logger::log(LoggerMode::rayTracingLog) << i << "th non planar cells done (size : " << m_pointCounts[nonPlanarLeafIds[i]] << "), points processed :" << pointProcessed << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << "average ball size : " << averageBallSize << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << "newPlanes : " << newPlanes << Logger::endl;

	}
}

std::vector<glm::dvec3> OctreeNormal::getBallPointsFromList(const std::vector<glm::dvec3>& points, const glm::dvec3& seedPoint, const double& radius)
{
	std::vector<glm::dvec3> result;
	for (int i = 0; i < (int)points.size(); i++)
	{
		if (glm::length(points[i] - seedPoint) < radius)
			result.push_back(points[i]);
	}
	return result;
}

bool OctreeNormal::isPointInNeighborhoods(const glm::dvec3& point, const std::vector<LocalPlane>& neighborhoods, const double& threshold)
{
	for (int i = 0; i < (int)neighborhoods.size(); i++)
	{
		if (isPointInLocalPlane(point, neighborhoods[i], threshold))
			return true;
	}
	return false;
}

