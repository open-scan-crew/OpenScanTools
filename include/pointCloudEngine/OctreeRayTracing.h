#ifndef OCTREE_RAY_TRACING_H
#define OCTREE_RAY_TRACING_H

#include "OctreeBase.h"
#include "models/data/clipping/ClippingGeometry.h"
#include "models/graph/TransformationModule.h"

class GeometricBox

{
public:
	GeometricBox(const std::vector<glm::dvec3> corners);
	GeometricBox(const ClippingAssembly& clippingAssembly);
	~GeometricBox();
	glm::dvec3 getDirX() const;
	glm::dvec3 getDirY() const;
	glm::dvec3 getDirZ() const;
	bool isInside(const glm::dvec3& point) const;
	bool isInHalfSpace(const glm::dvec3& planePoint1, const glm::dvec3& planePoint2, const glm::dvec3& planePoint3, const glm::dvec3& halfPlanePoint, const glm::dvec3& testPoint) const;
	glm::dvec3 getCenter() const;
	double getRadius() const;
	void setRadius(const double& radius);

	std::vector<glm::dvec3> m_corners;
};

class ClusterInfo

{
public:
	ClusterInfo(const int& sizeX, const int& sizeY, const int& sizeZ, const int& label);
	~ClusterInfo();
	bool isTrueDynamic() const;
	void updateWithPoint(const int& x, const int& y, const int& z);
	int m_xMin, m_xMax, m_yMin, m_yMax, m_zMin, m_zMax, m_volume, m_label, m_emptyNeighbors, m_staticNeighbors;
};

enum class PlaneType
{
	vertical,
	horizontal,
	tilted
};

class RectangularPlane

{
public:
	RectangularPlane(const glm::dvec3& normal, const std::vector<glm::dvec3>& corners, const glm::dvec3& centerPoint, const PlaneType& planetype);
	void updateCorners(const std::vector<glm::dvec3>& corners);
	void updateNormal(const glm::dvec3& normal);
	void updateCenter(const glm::dvec3& centerPoint);
	TransformationModule createTransfo() const;

	glm::dvec3 m_normal;
	std::vector<glm::dvec3> m_corners;
	glm::dvec3 m_centerPoint;
	PlaneType m_planeType;
};

class AbstractPlane

{
public:
	AbstractPlane(const glm::dvec3& normal, const glm::dvec3& point);

	glm::dvec3 m_normal;
	glm::dvec3 m_point;
};

class OctreeRayTracing
{
private:
    OctreeRayTracing() {};
    ~OctreeRayTracing() {};

public:
	//ray tracing//
	static glm::dvec3 findBestPoint(const std::vector<std::vector<glm::dvec3>>& globalPointList, const glm::dvec3& rayDirection, const glm::dvec3& rayOrigin, const double& cosAngleThreshold, const double& rayRadius, const ClippingAssembly& clippingAssembly, const bool& isOrtho);
    static std::vector<int> getCellPath(const TreeCell& cell, const TreeCell& root);
	static bool isPointInCell(const glm::dvec3& localPoint, const TreeCell& cell);

	//cylinder fitting//

	static bool beginCylinderFit(const std::vector<std::vector<glm::dvec3>>& globalPointBuckets, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, const int& totalNumberPoints);
	static std::vector<glm::dvec3> makeDirectionList(const int& numberOfSteps);
	static std::vector<glm::dvec3> makeDirectionList2(const int& numberOfSteps);
	static glm::dvec3 computeCenterOfMass(const std::vector<glm::dvec3>& pointList);
	static void samplePointsUpToBucket(const std::vector<std::vector<glm::dvec3>>& globalPointBuckets, const int& bucketNumber, std::vector<glm::dvec3>& result);
	static std::vector<glm::dvec3> substractCenterOfMass(const std::vector<glm::dvec3>& pointList, const glm::dvec3& centerOfMass);

	//beam bending//

	static bool findBiggestBend(const std::vector<glm::dvec3>& neighbors, const std::vector<glm::dvec3>& discreteLine, const glm::dvec3& normalVector, glm::dvec3& bendPoint, double& maxBend, double& ratio);

	static void updateNormalVector(glm::dvec3& normalVector, const glm::dvec3& a, const glm::dvec3& b);
	
	//column offset//
	
	static bool columnOffset(const glm::dvec3& camPosition, glm::dvec3& wallPoint1, const glm::dvec3& wallPoint2, const glm::dvec3& columnPoint1, const glm::dvec3 columnPoint2, double& offset, double& ratio);
	
	//point to plane distance//
	static bool fitPlane(const std::vector<glm::dvec3>& points, std::vector<double>& result);
	static glm::dvec3 pickPointOnPlane(const std::vector<double>& plane);
	static bool fitPlaneBuckets(const std::vector<std::vector<glm::dvec3>>& points, std::vector<double>& result);
	static bool arePlanesSimilar(const std::vector<double>& plane1, const std::vector<double>& plane2);
	static double computeMeanSquaredDistanceToPlane(const std::vector<glm::dvec3>& points, std::vector<double> plane);
	static bool pointToPlaneMeasure(const std::vector<std::vector<glm::dvec3>>& pointBuckets, const glm::dvec3& seedPoint, double& distance);
	static double pointToPlaneDistance(const glm::dvec3& point, const std::vector<double>& plane);

	//beam Height
	
	static glm::dvec3 findBeamDirection(const std::vector<glm::dvec3>& points, const int& numberOfDirections, std::vector<std::vector<double>>& directionRange, const glm::dvec3& seedPoint, const glm::dvec3& normalVector, glm::dvec3& orthoDir);
	static double findBeamHeight(const std::vector<glm::dvec3>& points, const glm::dvec3& seedPoint, const glm::dvec3& normalVector, const double& heightMax, const glm::dvec3& direction, const glm::dvec3& orthoDir, const std::vector<double>& range);
	
	//bigCylinder
	static glm::dvec3 findPseudoCenter(const glm::dvec3& seed1, const glm::dvec3& seed2, const glm::dvec3& normal1, const glm::dvec3& normal2);

	//plane fitting
	static int countPointsNearPlane(const std::vector<glm::dvec3>& points, const std::vector<double>& plane, const double& threshold);
	static std::vector<glm::dvec3> listPointsNearPlane(const std::vector<glm::dvec3>& points, const std::vector<double>& plane, const double& threshold);
	static void connectPlanes(RectangularPlane& rect1, RectangularPlane& rect2);
	static double computeExtensionTowardsLine(const glm::dvec3& origin, const glm::dvec3& extDirection, const glm::dvec3& linePoint, const glm::dvec3 lineDirection);
	static bool computePlaneIntersection(const std::vector<double>& plane1, const std::vector<double>& plane2, glm::dvec3& lineDirection, glm::dvec3& linePoint);


	//multipleCylinders
    static bool isPointCloseToPreviousCylinders(const glm::dvec3& point, const std::vector<glm::dvec3>& cylinderCenters, const std::vector<glm::dvec3>& cylinderDirections, const std::vector<double>& cylinderRadii, const double& threshold);

// << From feature_better_streamings
	static int firstNode(const double& tx0, const double& ty0, const double& tz0, const double& txm, const double& tym, const double& tzm);
	static int new_node(const double& a, const double& b, const double& c, const int& p, const int& q, const int& r);
// << From develop
	//extendCylinders
	static void completeVectorToOrthonormalBasis(const glm::dvec3& u, glm::dvec3& v, glm::dvec3& w);

	//people remover
	static int relateBoxToOtherBox(const GeometricBox& box1, const GeometricBox& box2);
	static bool doBoxIntersectFromViewPoint(const glm::dvec3& origin, const GeometricBox& box1, const GeometricBox& box2, double& distanceToIntersection);
	static glm::dvec3 getTransformationCoord(const glm::dvec3& globalCoord, const ClippingAssembly& clippingAssembly);

	// >>

protected:
	// ray tracing 
	static int computeExitPlane(const double& tx0, const double& ty0, const double& tz0);

    // NOTE(robin) - Add the root as parameter to make the function static
	static std::vector<bool> getCellPathInADimension(const TreeCell& cell, const TreeCell& root, const int& dimensionIndex);
	static void displayCellInfo(const TreeCell& cell, const TreeCell& root);
	static void displayPointCoordinatesGLM(const glm::dvec3& point);
	

	// cylinderFitting
	static bool isCellInBall(const glm::dvec3& center, const double& radius, const TreeCell& cell);
	static glm::dmat3 preprocessData(const std::vector<glm::dvec3>& centeredDataPoints, std::vector<std::vector<double>>& F1, std::vector<std::vector<double>>& F2, std::vector<double>& mu);
	static std::vector<std::vector<double>> computeDeltaSkewed(const std::vector<glm::dvec3>& centeredDataPoints);
	static std::vector<double> computeMu(const std::vector<std::vector<double>>& deltaSkewed);
	static std::vector<std::vector<double>> computeDeltas(const std::vector<std::vector<double>>& deltaSkewed, const std::vector<double>& mu);
	static glm::mat3 computeF0(const std::vector<glm::dvec3>& centeredDataPoints);
	static std::vector<std::vector<double>> computeF1(const std::vector<glm::dvec3>& centeredDataPoints, const std::vector<std::vector<double>>& deltas);
	static std::vector<std::vector<double>> computeF2(const std::vector<std::vector<double>>& deltas);
	static std::vector<std::vector<double>> computeOuterProductAverage(const std::vector<std::vector<double>>& X, const std::vector<std::vector<double>>& Y);
	static glm::dvec3 fitCylinder(const std::vector<glm::dvec3>& directionList, glm::dvec3& center, double& radius, const glm::dmat3& F0, const std::vector<std::vector<double>>& F1, const std::vector<std::vector<double>>& F2, const std::vector<double>& mu, double& error);
	static glm::mat3 computeProjectionMatrix(const glm::dvec3& direction);
	static glm::mat3 computeSkewMatrix(const glm::dvec3& direction);
	static glm::mat3 computeA(const glm::mat3& projectionMatrix, const glm::mat3& F0);
	static glm::mat3 computeAHat(const glm::mat3& A, const glm::mat3& skewMatrix);
	static glm::mat3 computeQ(const glm::mat3& A, const glm::mat3& AHat);
	static std::vector<double> reshapeProjection(const glm::mat3& projectionMatrix);
	static glm::dvec3 computeAlpha(const std::vector<std::vector<double>>& F1, const std::vector<double>& reshapedProjection);
	static glm::dvec3 computeBeta(const glm::mat3& Q, const glm::dvec3& alpha);
	static double computeG(const std::vector<double>& reshapedProjection, const glm::mat3& F0, const std::vector<std::vector<double>>& F2, const glm::dvec3& alpha, const glm::dvec3& beta);
	static double computeRadius(const std::vector<double>& reshapedProjection, const std::vector<double>& mu, const glm::dvec3& beta);
	static std::vector<double> applyMatrixToVector(const std::vector<std::vector<double>>& matrix, const std::vector<double>& V);
	static double computeDotProduct(const std::vector<double>& x, const std::vector<double>& y);
	static std::vector<glm::dvec3> refineDirectionList(const glm::dvec3& direction, const double& range, const int& numberOfSteps);
	static double computeMeanSquareCylinderDistance(const std::vector<glm::dvec3>& points, const glm::dvec3& center, const glm::mat3& projectionMatrix, const double& radius);
	static double computePointToCylinderDistance(const glm::dvec3& point, const glm::dvec3& center, const glm::mat3& projectionMatrix, const double& radius);
	static std::vector<glm::dvec3> samplePoints(const glm::dvec3& center, const double& radius, const glm::dvec3& direction, const double& height, const int& numberOfTestPoints);
	static void displayBoundingBox(const std::vector<glm::dvec3>& globalPointList);
	static void appendBucketToList(const std::vector<std::vector<glm::dvec3>>& globalPointBuckets, std::vector<glm::dvec3>& pointList, const int& bucketNumber);
	static std::vector<glm::dvec3> getPointsInCylinder(const std::vector<glm::dvec3>& pointList, const glm::dvec3& cylinderDirection, const glm::dvec3& cylinderCenter, const double& cylinderRadius,const glm::dvec3& centerOfMass, const double& radiusThreshold);

	//beam bending
	static std::vector<glm::dvec3> discretize(const glm::dvec3& tip1, const glm::dvec3& tip2, const int& numberOfSteps);
	static std::vector<bool> computeSharpTransition(const std::vector<double>& values);
	static double computeMean(const std::vector<double>& values);
	static double computeSD(const std::vector<double>& values);
	static std::vector<double> computeAbsoluteDiscreteDerivative(const std::vector<double>& values);
	static std::vector<bool> mergeConfidenceList(const std::vector<bool>& confidenceList1, const std::vector<bool>& confidenceList2);
	static std::vector<bool> updateConfidenceList(const std::vector<bool>& confidenceList, const int& indexMargin);
	static std::vector<double> smoothenList(const std::vector<double>& bendList);
	static int findBiggestBendInList(const std::vector<double>& bendList, const std::vector<bool>& confidenceList);
	static std::vector<double> makeBendList(const std::vector<glm::dvec3>& neighbors, const std::vector<glm::dvec3>& discreteLine, const glm::dvec3& normalVector);


	//measure point to plane

	//I beam height
	static std::vector<glm::dvec3> sampleDirectionsXY(const int& numberOfDirections, const glm::dvec3& normalVector);
	static std::vector<std::vector<double>> computeRangeAlongDirection(const std::vector<glm::dvec3>& points, const glm::dvec3& direction, const glm::dvec3& seedPoint, const glm::dvec3& normalVector, glm::dvec3& orthoDir);
	static std::vector<double> computeRangeAlongDirection2(const std::vector<glm::dvec3>& points, const glm::dvec3& direction, const glm::dvec3& seedPoint, const glm::dvec3& normalVector, glm::dvec3& orthoDir);

	static double getValueOfRange(const std::vector<std::vector<double>>& range);
	std::vector<glm::dvec3> getPointsAlongDirection(const std::vector<glm::dvec3>& points, const glm::dvec3& seedPoint, const glm::dvec3& direction, const glm::dvec3& orthoDirection, const std::vector<double>& plane);
	static std::vector<glm::dvec3> countPointsNearHeight(const double& height, const double& heightThreshold, const std::vector<glm::dvec3>& points, glm::dvec3 seedPoint, glm::dvec3 normalVector);
    static std::vector<double> sampleHeights(const double& heightMax, const double& heightStep);

	//people remover



	//temp
	static std::vector<std::vector<double>> multiplyMatrix(const std::vector<std::vector<double>>& A, const std::vector<std::vector<double>>& B);
	static std::vector<std::vector<double>> scaleMatrix(const std::vector<std::vector<double>>& A, const double& lambda);
	static std::vector<std::vector<double>> addMatrix(const std::vector<std::vector<double>>& A, const std::vector<std::vector<double>>& B);
	static std::vector<std::vector<double>> transposeMatrix(const std::vector<std::vector<double>>& A);
	static double isSymetrical(const std::vector<std::vector<double>>& A);
};

class BeamDirectionRange
{

public:
	BeamDirectionRange(const double pointWidth);
	~BeamDirectionRange();
	void addPoint(const double& pointPosition);
	std::vector<double> getCentralInterval();
private:
	std::vector<std::vector<double>> m_intervals;
	double m_pointWidth;
};

class VoxelGrid

{
public:
	VoxelGrid(const double voxelSize, const ClippingAssembly clippingAssembly, const double& tMax);
	~VoxelGrid();
	void voxelCoordinatesOfPoint(const glm::dvec3& point, int& x, int& y, int& z) const;
	glm::dvec3 centerOfVoxel(const int& x, const int& y, const int& z) const;
	bool isVoxelOccupied(const int& i, const int& j, const int& k, const int& scanNumber) const;
	bool areVoxelCoordinatesValid(const int& i, const int& j, const int& k) const;
	double m_xMin, m_xMax, m_yMin, m_yMax, m_zMin, m_zMax, m_voxelSize;
	ClippingAssembly m_clippingAssembly;
	std::vector<std::vector<std::vector<int>>> m_grid;
	int m_sizeX, m_sizeY, m_sizeZ, m_numberOfScans;
	GeometricBox getBoxFromCoordinates(const int& i, const int& j, const int& k) const;
};

class OctreeVoxelNode {
public:
	OctreeVoxelNode(const int& leafSize);
	~OctreeVoxelNode();
	std::vector<OctreeVoxelNode*> m_children; // pointer to child nodes
	std::vector<bool> m_data; // boolean vector for leaf nodes
	int m_dataSize;
	void insertValue(int depth, int x, int y, int z, int valuePosition, int maxDepth);
	bool readValue(int depth, int x, int y, int z, int valuePosition, int maxDepth);
	int getChildIndex(int depth, int x, int y, int z, int maxDepth);

};

class OctreeVoxelGrid

{
public:
	OctreeVoxelGrid(const double voxelSize, const ClippingAssembly clippingAssembly, const int numberOfScans);
	~OctreeVoxelGrid();

	bool areVoxelCoordinatesValid(const int& x, const int& y, const int& z) const;
	bool isVoxelOccupied(const int& x, const int& y, const int& z, const int& scanNumber) const;
	bool isEmpty(const int& x, const int& y, const int& z) const;
	void voxelCoordinatesOfPoint(const glm::dvec3& point, int& x, int& y, int& z) const;
	glm::dvec3 centerOfVoxel(const int& x, const int& y, const int& z) const;
	GeometricBox getBoxFromCoordinates(const int& x, const int& y, const int& z) const;
	void addPoint(const glm::dvec3& point, const int& scanNumber);
	void addValue(const int& x, const int& y, const int& z, const int& scanNumber);
	OctreeVoxelNode* m_octree;
	ClippingAssembly m_clippingAssembly;
	double m_maxSize, m_voxelSize;
	int m_numberOfScans, m_maxDepth;

};

#endif // !OCTREE_RAYTRACING_H