#ifndef NORMAL_ESTIMATION_H
#define NORMAL_ESTIMATION_H

#include "OctreeRayTracing.h"
#include <glm/glm.hpp>

class TlScanFile_T;
typedef TlScanFile_T* TlScanFile;

class LocalPlane
{
public:
	LocalPlane(const std::vector<double>& plane, const glm::dvec3& center, const double& radius);
	~LocalPlane();

	std::vector<double> m_plane;
	glm::dvec3 m_center;
	double m_radius;
};

class OctreeNormal : public OctreeRayTracing
{
public:
	OctreeNormal(OctreeBase const& base, const uint64_t& pointDataOffset, TlScanFile file, glm::dmat4 matrixToGlobal);
	OctreeNormal(const OctreeRayTracing& octree);
	~OctreeNormal();
	void fillSkeleton();
	std::vector<LocalPlane> computeCoherentNeighborhoods(const double& threshold, std::vector<uint32_t>& nonPlanarLeafIds);
	

protected:
	bool computeCoherentNeighborhoodInCell(const uint32_t& cellId, std::vector<LocalPlane>& neighborhoods, const double& threshold);
	void centerOfMassLeaf(const uint32_t& cellId);
	void fillSkeletonRecursive(const uint32_t& cellId);
	int countPointsNearPlane(const std::vector<double>& plane, const double& threshold, const std::vector<glm::dvec3>& points);
	bool isPointInLocalPlane(const glm::dvec3& point, const LocalPlane& localPlane, const double& threshold);
	void getLeavesUnderCell(const uint32_t& cellId, std::vector<uint32_t>& result);
	std::vector<glm::dvec3> getRandomPointsInLeaves(const std::vector<uint32_t>& leavesId, const int& numberOfPoints);
	std::vector<glm::dvec3> getPointsFromCell(const uint32_t& cellId);
	void estimateResult(const std::vector<LocalPlane>& neighborhoods, const double& threshold);
	void computeNormalsInNonPlanarLeaves(const std::vector<uint32_t>& nonPlanarLeafIds, std::vector<LocalPlane>& neighborhoods, const double& threshold);
	std::vector<glm::dvec3> getBallPointsFromList(const std::vector<glm::dvec3>& points, const glm::dvec3& seedPoint, const double& radius);
	bool isPointInNeighborhoods(const glm::dvec3& point, const std::vector<LocalPlane>& neighborhoods, const double& threshold);

	std::vector<int> m_pointCounts;
	std::vector<glm::dvec3> m_centerOfMass;
};


#endif //NORMAL_ESTIMATION_H