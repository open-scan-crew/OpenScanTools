#include "pointCloudEngine/OctreeRayTracing.h"
#include "pointCloudEngine/MeasureClass.h"
#include "utils/Logger.h"
#include <glm/gtx/quaternion.hpp>

#include <ctime>

#define M_PI           3.14159265358979323846  /* pi */


int OctreeRayTracing::computeExitPlane(const double& tx0, const double& ty0, const double& tz0)
{
	int result(3);
	if ((tx0 >= ty0) && (tx0 >= tz0))
	{
		result = 0;
	}
	if ((ty0 >= tx0) && (ty0 >= tz0))
	{
		result = 1;
	}
	if ((tz0 >= tx0) && (tz0 >= ty0))
	{
		result = 2;
	}

	return result;
}

int OctreeRayTracing::firstNode(const double& tx0, const double& ty0, const double& tz0, const double& txm, const double& tym, const double& tzm)
{
	int exitPlane, result(0);
	int test(0);
	exitPlane = computeExitPlane(tx0, ty0, tz0);
	switch (exitPlane)
	{
	case 0:
	{
		if (tym < tx0) { result = result | 2; test++; }
		if (tzm < tx0) { result = result | 1; test++; }
	}

	case 1:
	{
		if (txm < ty0) { result = result | 4; test++; }
		if (tzm < ty0) { result = result | 1; test++; }
	}

	case 2:
	{
		if (txm < tz0) { result = result | 4; test++; }
		if (tym < tz0) { result = result | 2; test++; }
	}
	}
	return result;
}

int OctreeRayTracing::new_node(const double& a, const double& b, const double& c, const int& p, const int& q, const int& r)
{
	if (a < b)
	{
		if (a < c) { return p; }
		else { return r; }
	}
	else
	{
		if (b < c) { return q; }
		else { return r; }
	}
}

std::vector<bool> OctreeRayTracing::getCellPathInADimension(const TreeCell& cell, const TreeCell& root, const int& dimensionIndex)
{
	std::vector<bool> result;
	int depth(0);
	double currSize = (double)cell.m_size;
	double rootSize = root.m_size;

	while (currSize < rootSize)
	{
		depth++;
		currSize *= 2;
	}
	int k = (int) (cell.m_position[dimensionIndex] - root.m_position[dimensionIndex])*(2 << depth) / (int)rootSize;
	for (int loop = 0; loop < depth; loop++)
	{
		result.push_back((k&(2 << (depth - loop - 1))) == (2 << (depth - loop - 1)));
	}
	return result;
}

std::vector<int> OctreeRayTracing::getCellPath(const TreeCell& cell, const TreeCell& root)
{
	std::vector<int> result;
	std::vector<std::vector<bool>> temp;
	for (int dimensionIndex = 0; dimensionIndex < 3; dimensionIndex++)
	{
		std::vector<bool> dimensionPath = getCellPathInADimension(cell, root, dimensionIndex);
		temp.push_back(dimensionPath);
	}
	for (int loop = 0; loop < temp[0].size(); loop++)
	{
		result.push_back(4 * temp[0][loop] + 2 * temp[1][loop] + temp[2][loop]);
	}
	return result;
}

void OctreeRayTracing::displayCellInfo(const TreeCell& cell, const TreeCell& root)
{
	/*std::vector<int> path = getCellPath(cell);
	glm::dvec3 localCorner,globalCorner;
	double zMin(DBL_MAX), zMax(-DBL_MAX);
	localCorner = glm::dvec3(cell.m_position[0], cell.m_position[1], cell.m_position[2]);	
	globalCorner = pointLocalToGlobal(localCorner, m_globalMatrix);
	for (int a = 0; a < 2; a++)
	{
		for (int b = 0; b < 2; b++)
		{
			for (int c = 0; c < 2; c++)
			{
				glm::dvec3 tempLocalCorner = localCorner + glm::dvec3((double)a*cell.m_size, (double)b*cell.m_size, (double)c*cell.m_size);
				glm::dvec3 tempGlobalCorner = pointLocalToGlobal(tempLocalCorner, m_globalMatrix);
				if (tempGlobalCorner.z > zMax)
					zMax = tempGlobalCorner.z;
				if (tempGlobalCorner.z < zMin)
					zMin = tempGlobalCorner.z;
			}
		}
	}

	Logger::log(LoggerMode::rayTracingLog) << "DISPLAYING CELL INFO" << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "depth : " << path.size() << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "zMin : " << zMin << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "zMax : " << zMax << Logger::endl;

	Logger::log(LoggerMode::rayTracingLog) << "path : " << Logger::endl;
	for (int loop = 0; loop < path.size(); loop++)
	{
		Logger::log(LoggerMode::rayTracingLog) << path[loop] << Logger::endl;
	}
	Logger::log(LoggerMode::rayTracingLog) << "END OF DISPLAY" << Logger::endl << Logger::endl;

	return;*/

	std::vector<int> path = getCellPath(cell, root);
	float x0(cell.m_position[0]), y0(cell.m_position[1]), z0(cell.m_position[2]);
	float x1(cell.m_position[0] + cell.m_size), y1(cell.m_position[1] + cell.m_size), z1(cell.m_position[2] + cell.m_size);

	Logger::log(LoggerMode::rayTracingLog) << "DISPLAYING CELL INFO" << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "depth : " << path.size() << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "range : [" <<
		x0 << ", " << x1 << "] [" << y0 << ", " << y1 << "] [" << z0 << ", " << z1 << "]" << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "path : " << Logger::endl;
	for (int loop = 0; loop < path.size(); loop++)
	{
		Logger::log(LoggerMode::rayTracingLog) << path[loop] << Logger::endl;
	}

	Logger::log(LoggerMode::rayTracingLog) << "children : " << Logger::endl;
	for (int index = 0; index < 8; index++)
	{
		if (cell.m_children[index] != NO_CHILD)
			Logger::log(LoggerMode::rayTracingLog) << index << Logger::endl;
	}
	Logger::log(LoggerMode::rayTracingLog) << "END OF DISPLAY" << Logger::endl << Logger::endl;

	return;
}

void OctreeRayTracing::displayPointCoordinatesGLM(const glm::dvec3& point)
{
	Logger::log(LoggerMode::rayTracingLog) << "POINT INFO : " << point.x << " " << point.y << " " << point.z << Logger::endl << Logger::endl;
}


glm::dvec3 OctreeRayTracing::findBestPoint(const std::vector<std::vector<glm::dvec3>>& globalPointList, const glm::dvec3& rayDirection, const glm::dvec3& rayOrigin, const double& cosAngleThreshold, const double& rayRadius, const ClippingAssembly& clippingAssembly, const bool& isOrtho)
{
	/*double minDistance(DBL_MAX),cosAngleReturn,distanceIfNotFound, cosAngleReturnIfNotFound(0);
	glm::dvec3 result,resultIfNotFound;
	bool hasFoundAPoint(false);
	int indexOfResult,indexIfNotFound;
	for (int loop = 0; loop < globalPointList.size(); loop++)
	{
		glm::dvec3 point = globalPointList[loop];
		glm::dvec3 pointRay(point-rayOrigin);
		distance = glm::length(pointRay);
		if(glm::dot(pointRay,rayDirection)<0)
		{
			continue;
		}
		glm::dvec3 proj = glm::dot(pointRay, rayDirection)*rayDirection - pointRay;
		cosAngle = glm::dot(rayDirection, pointRay / distance);
		double projLength = glm::length(proj);
		proj = proj / projLength;
		bool test(false);
		double bestCosAngle(0);
		if ((rayRadius/projLength) >= 1) { test = true; }
		else {
			pointRay = pointRay + proj*rayRadius;
			pointRay = pointRay / glm::length(pointRay);
			double currCosAngle = glm::dot(pointRay, rayDirection);
			if (currCosAngle > cosAngleThreshold)
			{
				test = true;
			}
				
		}
		if (test)
		{
			if (distance < minDistance)
			{
				hasFoundAPoint = true;
				minDistance = distance;
				result = point;
				indexOfResult = loop;
				cosAngleReturn = cosAngle;
			}
		}
		if ((cosAngle > cosAngleReturnIfNotFound) && (!hasFoundAPoint)) {
			cosAngleReturnIfNotFound = cosAngle;
			resultIfNotFound = point;
			distanceIfNotFound = distance;
			indexIfNotFound = loop;
		}
			
	}
	distance = minDistance;
	cosAngle = cosAngleReturn;

	if (!hasFoundAPoint)
	{
		distance = distanceIfNotFound;
		cosAngle = cosAngleReturnIfNotFound;
		result = resultIfNotFound;
		indexOfResult = indexIfNotFound;
		//Logger::log(LoggerMode::rayTracingLog) << "NO POINTS FOUND" << Logger::endl;
	}
	//Logger::log(LoggerMode::rayTracingLog) << "index of result " << indexOfResult << Logger::endl;
	return result;*/
	double dMin(DBL_MAX), bestCosAngle(-1),currCosAngle(0),currDistance(0),distanceThreshold(0.01);
	if (isOrtho)
		bestCosAngle = cosAngleThreshold + 1;
	bool hasGoodAngle(false), oneMoreLeaf(true);
	glm::dvec3 bestAnglePoint = glm::dvec3(NAN);
	std::vector<glm::dvec3> goodAnglePoints;
	int index(0),leafIndex(0);

	glm::dvec3 result(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());
	/*for (int leafStep = 0; leafStep < (int)globalPointList.size(); leafStep++)			//first for loop computes dMin, but only for points which are close enough to ray
	{
		for (int i = 0; i < (int)globalPointList[leafStep].size(); i++)
		{
			glm::dvec3 point = globalPointList[leafStep][i];
			if (!isPointClipped(point, clippingBoxes))
			{
				continue;
			}
			glm::dvec3 pointRay(point - rayOrigin);
			currDistance = glm::length(pointRay);
			if (glm::dot(pointRay, rayDirection) < 0)
			{
				continue;
			}
			glm::dvec3 proj = glm::dot(pointRay, rayDirection)*rayDirection - pointRay;
			currCosAngle = glm::dot(rayDirection, pointRay / currDistance);
			double projLength = glm::length(proj);
			proj = proj / projLength;
			if ((rayRadius / projLength) >= 1) {
				hasGoodAngle = true;
				if (currDistance < dMin)
				{
					dMin = currDistance;
					index = i;
					leafIndex = leafStep;
					currCosAngle = rayRadius / projLength;
				}
			}
			else {
				pointRay = pointRay + proj * rayRadius;
				pointRay = pointRay / glm::length(pointRay);
				if (currCosAngle > cosAngleThreshold)
				{
					hasGoodAngle = true;
					if (currDistance < dMin)
					{
						dMin = currDistance;
						index = i;
						leafIndex = leafStep;
					}
				}
			}
		}
	}
	Logger::log(LoggerMode::rayTracingLog) << "dMin : " << dMin << Logger::endl;
	double distance;
	int index1(0);
	for (int loop = 0; loop < globalPointList.size(); loop++)		//second loop looks at point at a distance close to dMin, and choose the best angle among them
	{
		glm::dvec3 point = globalPointList[loop];
		if (!isPointClipped(point, clippingBoxes))
			continue;
		glm::dvec3 pointRay(point - rayOrigin);
		currDistance = glm::length(pointRay);
		if (glm::dot(pointRay, rayDirection) < 0)
		{
			continue;
		}
		glm::dvec3 proj = glm::dot(pointRay, rayDirection)*rayDirection - pointRay;
		currCosAngle = glm::dot(rayDirection, pointRay / currDistance);

		if ((rayRadius / glm::length(proj)) >= 1) {
			pointRay = pointRay + proj * rayRadius;
			pointRay /= glm::length(pointRay);
			currCosAngle = rayRadius/ glm::length(proj);
		}
		if (hasGoodAngle)
		{			
			if ((currDistance < (dMin + distanceThreshold)) && (currCosAngle > cosAngleThreshold))
			{
				if (currCosAngle > bestCosAngle)
				{
					bestCosAngle = currCosAngle;
					result = point;
					distance = currDistance;
				}
			}
		}
		else
		{
			if (currCosAngle > bestCosAngle)
			{
				bestCosAngle = currCosAngle;
				result = point;
				distance = currDistance;
				index1 = loop;
			}
		}
	}
	//Logger::log(LoggerMode::rayTracingLog) << "distance : " << distance << Logger::endl;
	//Logger::log(LoggerMode::rayTracingLog) << "hasGoodAngle : "<<hasGoodAngle << Logger::endl;*/
	for (int leafStep = 0; leafStep < (int)globalPointList.size(); leafStep++)		
	{
		for (int i = 0; i < (int)globalPointList[leafStep].size(); i++)
		{
			glm::dvec3 point = globalPointList[leafStep][i];
            glm::dvec4 point4 = glm::dvec4(point, 1.0);
            if (!clippingAssembly.testPoint(point4))
            {
                continue;
            }
			//if (!isPointClipped(point, clippingBoxes))
			//{
			//	continue;
			//}

			glm::dvec3 pointRay(point - rayOrigin);
			currDistance = glm::length(pointRay);
			if (glm::dot(pointRay, rayDirection) < 0)
			{
				continue;
			}
			glm::dvec3 proj = glm::dot(pointRay, rayDirection)*rayDirection - pointRay;
			currCosAngle = glm::dot(rayDirection, pointRay / currDistance);
			if(isOrtho)
				currCosAngle = glm::length(glm::cross(rayDirection, pointRay)) / glm::length(rayDirection);
			double projLength = glm::length(proj);
			proj = proj / projLength;

			if ((currCosAngle > bestCosAngle)&&(!isOrtho))
			{
				bestCosAngle = currCosAngle;
				bestAnglePoint = point;
			}
			if ((currCosAngle < bestCosAngle) && (isOrtho))
			{
				bestCosAngle = currCosAngle;
				bestAnglePoint = point;
			}
			if (!isOrtho)
			{
				if ((rayRadius / projLength) >= 1) {
					hasGoodAngle = true;
					currCosAngle = rayRadius / projLength;
					if (currDistance < (dMin*1.05))
						goodAnglePoints.push_back(point);
					if (currDistance < dMin)
					{
						dMin = currDistance;
						index = i;
						leafIndex = leafStep;
					}
				}
				else {
					pointRay = pointRay + proj * rayRadius;
					pointRay = pointRay / glm::length(pointRay);
					currCosAngle = glm::dot(rayDirection, pointRay);

					if (currCosAngle > cosAngleThreshold)
					{
						hasGoodAngle = true;
						if (currDistance < (dMin*1.05))
							goodAnglePoints.push_back(point);
						if (currDistance < dMin)
						{
							dMin = currDistance;
							index = i;
							leafIndex = leafStep;
						}
					}
				}		
			}
			if (isOrtho)
			{
				if (currCosAngle < cosAngleThreshold)
				{
					hasGoodAngle = true;
					if (currDistance < (dMin + distanceThreshold))
						goodAnglePoints.push_back(point);
					if (currDistance < dMin)
					{
						dMin = currDistance;
						index = i;
						leafIndex = leafStep;
					}
				}
			}		
		}
		if (!oneMoreLeaf)
			break;
		else if (hasGoodAngle)
			oneMoreLeaf = false;
	}
	if (!hasGoodAngle)
		result=bestAnglePoint;
	else
	{
		double currScore,bestScore;
		bestCosAngle = -1;
		if (isOrtho)
			bestCosAngle = cosAngleThreshold + 1;
		for (int i = 0; i < (int)goodAnglePoints.size(); i++)
		{
			glm::dvec3 point = goodAnglePoints[i];
			glm::dvec3 pointRay(point - rayOrigin);
			currDistance = glm::length(pointRay);
			glm::dvec3 proj = glm::dot(pointRay, rayDirection)*rayDirection - pointRay;
			currCosAngle = glm::dot(rayDirection, pointRay / currDistance);
			if (isOrtho)
				currCosAngle = glm::length(glm::cross(rayDirection, pointRay)) / glm::length(rayDirection);
			if (!isOrtho)
			{
				currScore = 1.7 * glm::length(proj) + currDistance;
				if (i == 0)
				{
					bestScore = currScore;
					result = point;
				}
				if (currScore < bestScore)
				{
					bestScore = currScore;
					result = point;
				}
				/*if ((rayRadius / glm::length(proj)) >= 1) {
					currCosAngle = rayRadius / glm::length(proj);
				}
				if ((currDistance < (dMin*1.05)) && (currCosAngle > bestCosAngle))
				{
					result = point;
					bestCosAngle = currCosAngle;
				}*/
			}
			if (isOrtho)
			{
				if ((currDistance < (dMin+distanceThreshold)) && (currCosAngle < bestCosAngle))
				{
					result = point;
					bestCosAngle = currCosAngle;
				}
			}
		}
	}
	return result;
}


///////////////////////////////CYLINDER FITTING WIP/////////////////////////////////////


bool OctreeRayTracing::isPointInCell(const glm::dvec3& localPoint, const TreeCell& cell)
{
	double x(localPoint.x), y(localPoint.y), z(localPoint.z), x0(cell.m_position[0]), y0(cell.m_position[1]), z0(cell.m_position[2]);
	double x1(x0 + cell.m_size), y1(y0 + cell.m_size), z1(z0 + cell.m_size);
	return ((x > x0) && (x < x1) && (y > y0) && (y < y1) && (z > z0) && (z < z1));
}

bool OctreeRayTracing::isCellInBall(const glm::dvec3& center, const double& radius, const TreeCell& cell)
{
	glm::dvec3 corner;
	for (int dimIndex = 0; dimIndex < 8; dimIndex++)
	{
		corner = glm::dvec3(cell.m_position[0], cell.m_position[1], cell.m_position[2]) +(double) cell.m_size*glm::dvec3(dimIndex & 1, dimIndex & 2, dimIndex & 4);
		if (glm::length(corner - center) > radius)
		{
			return false;
		}
	}
	return true;
}


/*bool OctreeRayTracing::beginCylinderFit(const std::vector<std::vector<glm::dvec3>>& globalPointBuckets, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter)
{
	//////////////////////////////////////Get Scanrelevant parameters and data/////////////////////////////////////////

	double error;
	//displayBoundingBox(globalPointList);
					
	//glm::dvec3 localSeedPoint = pointLocalToGlobal(globalSeedPoint, glm::inverse(matrixToGlobal));
	std::vector<double> temp(3), refinedDirection, mu, listOfError, heights;
	std::vector<std::vector<double>> F1, F2, occlusionHeights;
	double angleRange(0.05), radius;			// starting radius is used as an initialisation to the region growing
	int compressionRatio(1), numberOfSteps(50), numberOfStepsForRefine(50);				// threshold is used when extruding the cylinder, hopefully taking into account good points that would be excluded due to a poor radius estimate
	glm::dvec3 centerOfMass, center, centerOfMassBall, xyzDirection, cylinderCenterOfMass;
	std::vector<glm::dvec3> ballPoints, centeredBallPoints, directions, newDirectionList, cylinderPoints, centeredCylinderPoints, totalPoints;			

	double MSE, heightThreshold(0.005), heightStep(0.08);
	int maxNumberOfPoints(30000);
	srand((unsigned int)time(NULL));
	bool jump(false);

	for (int tryOut = 0; tryOut < globalPointBuckets.size(); tryOut++)
	{
		Logger::log(LoggerMode::rayTracingLog) << "tryOut : " << tryOut << Logger::endl;
		appendBucketToList(globalPointBuckets, totalPoints, tryOut);
		if (totalPoints.size() > maxNumberOfPoints)
		{
			compressionRatio = (int)totalPoints.size() / maxNumberOfPoints;
			std::vector<glm::dvec3> otherTemp;
			for (int loop = 0; loop < maxNumberOfPoints; loop++)
			{
				int v1 = rand() % 1000;
				int v2 = rand() % (totalPoints.size()/(int)1000);
				int index = (v1 + 1000 * v2) % totalPoints.size();

				otherTemp.push_back(totalPoints[index]);			//insert stuff to pick points at random instead
			}

			ballPoints = otherTemp;
		}
		else { ballPoints = totalPoints; }
		if ((int)ballPoints.size() < 1000)
			continue;
		/////////////////////////////////////////////////////////// search direction/////////////////////////////////////////////////

		directions = makeDirectionList(numberOfSteps);								//list of directions to try



		centerOfMassBall = computeCenterOfMass(ballPoints);

		centeredBallPoints = substractCenterOfMass(ballPoints, centerOfMassBall);


		////////////////////////////////////////////////////////// first preprocess /////////////////////////////////////////////////
		Logger::log(LoggerMode::rayTracingLog) << "preprocessing data ... " << Logger::endl;

		glm::dmat3 F0 = preprocessData(centeredBallPoints, F1, F2, mu);
		Logger::log(LoggerMode::rayTracingLog) << "preprocessing complete" << Logger::endl;

		/////////////////////////////////////// first cylinder fit around the starting ball ////////////////////////////////////

		xyzDirection = fitCylinder(directions, center, radius, F0, F1, F2, mu, error);						// center and radius are updated to the fitted value

		//listOfErrors = computeListOfErrors(center, computeProjectionMatrix(xyzDirection), centeredBallPoints, radius);

		/////////////////////////////////////// refining direction of fit //////////////////////////////////////////////////


		newDirectionList = refineDirectionList(xyzDirection, angleRange, numberOfStepsForRefine);   // list of directions close to xyzDirection

		xyzDirection = fitCylinder(newDirectionList, center, radius, F0, F1, F2, mu, error);
		Logger::log(LoggerMode::rayTracingLog) << "numberOfBallPoints : " << ballPoints.size() << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << "direction : " << xyzDirection[0] << " " << xyzDirection[1] << " " << xyzDirection[2] << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << "radius : " << radius << Logger::endl;
		MSE = computeMeanSquareCylinderDistance(centeredBallPoints, center, computeProjectionMatrix(xyzDirection), radius);
		Logger::log(LoggerMode::rayTracingLog) << "MSE : " << MSE << Logger::endl;
		center = center + centerOfMassBall;
		if ((radius > 0.2)&&!jump)
		{
			tryOut = (int)globalPointBuckets.size()-3;
			jump = true;
			continue;
		}
		if (MSE < 0.0015)
		{
			cylinderRadius = radius;
			cylinderCenter = center;
			cylinderDirection = xyzDirection;
			return true;
		}
		//second fit around cylinderPoints //
		/*double radiusThreshold = radius/(double)20;
		cylinderPoints = getPointsInCylinder(ballPoints, xyzDirection, center, radius, centerOfMassBall, radiusThreshold);
		Logger::log(LoggerMode::rayTracingLog) << "cylinderPoints : " << cylinderPoints.size() << Logger::endl;
		int cylinderSize1 = (int)cylinderPoints.size();
		if (cylinderPoints.size() < 500) { continue; }

		cylinderCenterOfMass = computeCenterOfMass(cylinderPoints);
		centeredCylinderPoints = substractCenterOfMass(cylinderPoints, cylinderCenterOfMass);
		F0 = preprocessData(centeredCylinderPoints, F1, F2, mu);
		newDirectionList = refineDirectionList(xyzDirection, angleRange / (double)5, numberOfStepsForRefine);
		xyzDirection = fitCylinder(newDirectionList, center, radius, F0, F1, F2, mu, error);*/
		
		//and another fit
		

		//MSE = computeMeanSquareCylinderDistance(centeredCylinderPoints, center, computeProjectionMatrix(xyzDirection), radius);
		//Logger::log(LoggerMode::rayTracingLog) << "MSE : " << MSE << Logger::endl;
		/*if (MSE < 0.002)
		{
			cylinderRadius = radius;
			cylinderCenter = center + cylinderCenterOfMass;
			cylinderDirection = xyzDirection;
			return true;
		}
		center = center + cylinderCenterOfMass;

		cylinderPoints = getPointsInCylinder(ballPoints, xyzDirection, center, radius, cylinderCenterOfMass, radiusThreshold);
		Logger::log(LoggerMode::rayTracingLog) << "cylinderPoints : " << cylinderPoints.size() << Logger::endl;

		if (cylinderPoints.size() < 500) { continue; }
		bool getout(false);
		if ((double)cylinderPoints.size() / cylinderSize1 > 1.5) {
			for (int insist = 0; insist < 3; insist++)
			{
				cylinderCenterOfMass = computeCenterOfMass(cylinderPoints);
				centeredCylinderPoints = substractCenterOfMass(cylinderPoints, cylinderCenterOfMass);
				F0 = preprocessData(centeredCylinderPoints, F1, F2, mu);
				newDirectionList = refineDirectionList(xyzDirection, angleRange / (double)5, numberOfStepsForRefine);
				xyzDirection = fitCylinder(newDirectionList, center, radius, F0, F1, F2, mu, error);
				cylinderPoints = getPointsInCylinder(ballPoints, xyzDirection, center+cylinderCenterOfMass, radius, cylinderCenterOfMass, radiusThreshold);
				Logger::log(LoggerMode::rayTracingLog) << "cylinderPoints : " << cylinderPoints.size() << Logger::endl;



				if (cylinderPoints.size() < 500) { getout = true; break; }
				MSE = computeMeanSquareCylinderDistance(centeredCylinderPoints, center, computeProjectionMatrix(xyzDirection), radius);
				Logger::log(LoggerMode::rayTracingLog) << "MSE : " << MSE << Logger::endl;

				if (MSE > 0.002) { break; }
				else {
					cylinderRadius = radius;
					cylinderCenter = center + cylinderCenterOfMass;
					cylinderDirection = xyzDirection;
					return true;
				}
			}
		}
		if (getout) { continue; }
		cylinderCenterOfMass = computeCenterOfMass(cylinderPoints);
		centeredCylinderPoints = substractCenterOfMass(cylinderPoints, cylinderCenterOfMass);
		F0 = preprocessData(centeredCylinderPoints, F1, F2, mu);
		newDirectionList = refineDirectionList(xyzDirection, angleRange / (double)5, numberOfStepsForRefine);
		xyzDirection = fitCylinder(newDirectionList, center, radius, F0, F1, F2, mu, error);




		MSE = computeMeanSquareCylinderDistance(centeredCylinderPoints, center, computeProjectionMatrix(xyzDirection), radius);
		Logger::log(LoggerMode::rayTracingLog) << "MSE : " << MSE << Logger::endl;

		if (MSE > 0.002) { continue; }
		else {
			cylinderRadius = radius;
			cylinderCenter = center+cylinderCenterOfMass;
			cylinderDirection = xyzDirection;
			return true;
		}		*/
	/*}
	return false;
}*/
bool OctreeRayTracing::beginCylinderFit(const std::vector<std::vector<glm::dvec3>>& globalPointBuckets, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, const int& totalNumberPoints)
{

	// algorithm from : https://www.geometrictools.com/Documentation/LeastSquaresFitting.pdf
	//////////////////////////////////////Get Scanrelevant parameters and data/////////////////////////////////////////

	double error;
	//displayBoundingBox(globalPointList);

	//glm::dvec3 localSeedPoint = pointLocalToGlobal(globalSeedPoint, glm::inverse(matrixToGlobal));
	std::vector<double> temp(3), refinedDirection, mu, listOfError, heights;
	std::vector<std::vector<double>> F1, F2, occlusionHeights;
	double angleRange(0.05), radius;			// starting radius is used as an initialisation to the region growing
	int numberOfSteps(50), numberOfStepsForRefine(50);				// threshold is used when extruding the cylinder, hopefully taking into account good points that would be excluded due to a poor radius estimate
	glm::dvec3 center, centerOfMassBall, xyzDirection;
	std::vector<glm::dvec3> ballPoints, centeredBallPoints, directions, newDirectionList, cylinderPoints, centeredCylinderPoints;
	glm::dvec3 cDirection1, cDirection2, cCenter1, cCenter2;
	double cRadius1, cRadius2;
	bool try1(false), try2(false);
	double MSE;// heightThreshold(0.005), heightStep(0.08);
	int maxNumberOfPoints(totalNumberPoints);
	srand((unsigned int)time(NULL));
	
	for (int tryOut = 0; tryOut < globalPointBuckets.size(); tryOut++)
	{
		std::vector<glm::dvec3> totalPoints;
		Logger::log(LoggerMode::rayTracingLog) << "tryOut : " << tryOut << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << "displaying pointBuckets : " <<  Logger::endl;
		//for (int i=0;i<(int)globalPointBuckets.size();i++)

		samplePointsUpToBucket(globalPointBuckets, (int)globalPointBuckets.size() - tryOut, totalPoints);
		if (totalPoints.size() < 10)
			continue;
		if (totalPoints.size() > maxNumberOfPoints)
		{
			std::vector<glm::dvec3> otherTemp;
			for (int loop = 0; loop < maxNumberOfPoints; loop++)
			{
				int v1 = rand() % maxNumberOfPoints;
				int v2 = rand() % (totalPoints.size() / (int)maxNumberOfPoints);
				int index = (v1 + maxNumberOfPoints * v2) % totalPoints.size();

				otherTemp.push_back(totalPoints[index]);			//insert stuff to pick points at random instead
			}

			ballPoints = otherTemp;
		}
		else { ballPoints = totalPoints; }
		Logger::log(LoggerMode::rayTracingLog) << "numberOfBallPoints : " << ballPoints.size() << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << "numberOfTotalPoints : " << totalPoints.size() << Logger::endl;
		if ((int)ballPoints.size() < 10)
			continue;
		/////////////////////////////////////////////////////////// search direction/////////////////////////////////////////////////

		directions = makeDirectionList(numberOfSteps);								//list of directions to try



		centerOfMassBall = computeCenterOfMass(ballPoints);

		centeredBallPoints = substractCenterOfMass(ballPoints, centerOfMassBall);


		////////////////////////////////////////////////////////// first preprocess /////////////////////////////////////////////////
		Logger::log(LoggerMode::rayTracingLog) << "preprocessing data ... " << Logger::endl;

		glm::dmat3 F0 = preprocessData(centeredBallPoints, F1, F2, mu);
		Logger::log(LoggerMode::rayTracingLog) << "preprocessing complete" << Logger::endl;

		/////////////////////////////////////// first cylinder fit around the starting ball ////////////////////////////////////

		xyzDirection = fitCylinder(directions, center, radius, F0, F1, F2, mu, error);						// center and radius are updated to the fitted value

		//listOfErrors = computeListOfErrors(center, computeProjectionMatrix(xyzDirection), centeredBallPoints, radius);

		/////////////////////////////////////// refining direction of fit //////////////////////////////////////////////////


		newDirectionList = refineDirectionList(xyzDirection, angleRange, numberOfStepsForRefine);   // list of directions close to xyzDirection

		xyzDirection = fitCylinder(newDirectionList, center, radius, F0, F1, F2, mu, error);

		Logger::log(LoggerMode::rayTracingLog) << "direction : " << xyzDirection[0] << " " << xyzDirection[1] << " " << xyzDirection[2] << Logger::endl;
		Logger::log(LoggerMode::FunctionLog) << "diameter : " << 2*radius << Logger::endl;
		MSE = computeMeanSquareCylinderDistance(centeredBallPoints, center, computeProjectionMatrix(xyzDirection), radius);
		center = center + centerOfMassBall;
		Logger::log(LoggerMode::FunctionLog) << "MSE : " << MSE << Logger::endl;

		// test filtering bad points//
		if ((int)globalPointBuckets.size() > 4)
		{
			std::vector<glm::dvec3> filteredBallPoints;
			std::vector<glm::dvec3> betterBallPoints;
			samplePointsUpToBucket(globalPointBuckets, (int)globalPointBuckets.size() - tryOut, totalPoints);
			if (totalPoints.size() < 10)
				continue;
			if (totalPoints.size() > maxNumberOfPoints)
			{
				std::vector<glm::dvec3> otherTemp;
				for (int loop = 0; loop < maxNumberOfPoints; loop++)
				{
					int v1 = rand() % maxNumberOfPoints;
					int v2 = rand() % (totalPoints.size() / (int)maxNumberOfPoints);
					int index = (v1 + maxNumberOfPoints * v2) % totalPoints.size();

					otherTemp.push_back(totalPoints[index]);			//insert stuff to pick points at random instead
				}

				betterBallPoints = otherTemp;
			}
			else { betterBallPoints = totalPoints; }
			if ((int)betterBallPoints.size() < 10)
				continue;
			for (int i = 0; i < (int)betterBallPoints.size(); i++)
			{
				if (computePointToCylinderDistance(betterBallPoints[i], center, computeProjectionMatrix(xyzDirection), radius) < (1 * MSE))
					filteredBallPoints.push_back(betterBallPoints[i]);
			}
			if ((int)filteredBallPoints.size() < 10)
				continue;
			Logger::log(LoggerMode::rayTracingLog) << "numberOfFilteredPoints : " << filteredBallPoints.size() << Logger::endl;

			centerOfMassBall = computeCenterOfMass(filteredBallPoints);

			centeredBallPoints = substractCenterOfMass(filteredBallPoints, centerOfMassBall);

			glm::dmat3 F0 = preprocessData(centeredBallPoints, F1, F2, mu);

			newDirectionList = refineDirectionList(xyzDirection, angleRange, numberOfStepsForRefine);   // list of directions close to xyzDirection

			xyzDirection = fitCylinder(newDirectionList, center, radius, F0, F1, F2, mu, error);

			Logger::log(LoggerMode::rayTracingLog) << "direction : " << xyzDirection[0] << " " << xyzDirection[1] << " " << xyzDirection[2] << Logger::endl;
			Logger::log(LoggerMode::FunctionLog) << "(after refine) diameter : " << 2*radius << Logger::endl;
			MSE = computeMeanSquareCylinderDistance(centeredBallPoints, center, computeProjectionMatrix(xyzDirection), radius);
			Logger::log(LoggerMode::FunctionLog) << "MSE : " << MSE << Logger::endl;


			center = center + centerOfMassBall;
		}
		

		if (MSE < threshold)
		{
			cylinderRadius = radius;
			cylinderDirection = xyzDirection;
			cylinderCenter = center;

			return true;
		}
		if ((MSE < (1.5*threshold))&&(!try1))
		{
			try1 = true;
			cRadius1 = radius; cCenter1 = center; cDirection1 = xyzDirection;
		}
		if ((MSE < (2 * threshold))&&(!try2))
		{
			try2 = true;
			cRadius2 = radius; cCenter2 = center; cDirection2 = xyzDirection;
		}
	}
	if (try1)
	{
		cylinderCenter = cCenter1;
		cylinderDirection = cDirection1;
		cylinderRadius = cRadius1;
		return true;
	}
	if (try2)
	{
		cylinderCenter = cCenter2;
		cylinderDirection = cDirection2;
		cylinderRadius = cRadius2;
		return true;
	}
	
	return false;
}


std::vector<glm::dvec3> OctreeRayTracing:: makeDirectionList(const int& numberOfSteps)
{
	std::vector<glm::dvec3> result;
	glm::dvec3 direction, normalizedDirection;
	int i(0), j(0);
	for (i = 0; i < numberOfSteps; i++)
	{
		for (j = 0; j < numberOfSteps; j++)
		{
			direction[0] = 1;
			direction[1] = (double)i / numberOfSteps;
			direction[2] = (double)j / numberOfSteps;
			normalizedDirection = direction / glm::length(direction);
			result.push_back(normalizedDirection);

			direction[1] = -(double)i / numberOfSteps;
			normalizedDirection = direction / glm::length(direction);
			result.push_back(normalizedDirection);

			direction[2] = -(double)j / numberOfSteps;
			normalizedDirection = direction / glm::length(direction);
			result.push_back(normalizedDirection);

			direction[1] = (double)i / numberOfSteps;
			normalizedDirection = direction / glm::length(direction);
			result.push_back(normalizedDirection);

			direction[1] = 1;
			direction[0] = (double)i / numberOfSteps;
			direction[2] = (double)j / numberOfSteps;
			normalizedDirection = direction / glm::length(direction);
			result.push_back(normalizedDirection);

			direction[0] = -(double)i / numberOfSteps;
			normalizedDirection = direction / glm::length(direction);
			result.push_back(normalizedDirection);

			direction[2] = -(double)j / numberOfSteps;
			normalizedDirection = direction / glm::length(direction);
			result.push_back(normalizedDirection);

			direction[0] = (double)i / numberOfSteps;
			normalizedDirection = direction / glm::length(direction);
			result.push_back(normalizedDirection);

			direction[2] = 1;
			direction[1] = (double)i / numberOfSteps;
			direction[0] = (double)j / numberOfSteps;
			normalizedDirection = direction / glm::length(direction);
			result.push_back(normalizedDirection);

			direction[1] = -(double)i / numberOfSteps;
			normalizedDirection = direction / glm::length(direction);
			result.push_back(normalizedDirection);

			direction[0] = -(double)j / numberOfSteps;
			normalizedDirection = direction / glm::length(direction);
			result.push_back(normalizedDirection);

			direction[1] = (double)i / numberOfSteps;
			normalizedDirection = direction / glm::length(direction);
			result.push_back(normalizedDirection);
		}
	}

	return result;
}

std::vector<glm::dvec3> OctreeRayTracing::makeDirectionList2(const int& numberOfSteps)
{
	std::vector<glm::dvec3> result;
	glm::dvec3 direction;
	
	//z=-1, y stays positive

	for (int i = 0; i < numberOfSteps; i++)
	{
		for (int j = 0; j < numberOfSteps; j++)
		{
			direction[0] = (double)i / numberOfSteps;
			direction[1] = (double)j / numberOfSteps;
			direction[2] = -1.0;
			result.push_back(direction / glm::length(direction));

			if (i>0)
			{
				direction[0] = -(double)i / numberOfSteps;
				direction[1] = (double)j / numberOfSteps;
				direction[2] = -1.0;
				result.push_back(direction / glm::length(direction));
			}
		}
	}

	//z=1, y stays positive

	for (int i = 0; i < numberOfSteps; i++)
	{
		for (int j = 0; j < numberOfSteps; j++)
		{
			direction[0] = (double)i / numberOfSteps;
			direction[1] = (double)j / numberOfSteps;
			direction[2] = 1.0;
			result.push_back(direction / glm::length(direction));

			if (i > 0)
			{
				direction[0] = -(double)i / numberOfSteps;
				direction[1] = (double)j / numberOfSteps;
				direction[2] = 1.0;
				result.push_back(direction / glm::length(direction));
			}
		}
	}

	//y=1

	for (int i = 0; i < numberOfSteps; i++)
	{
		for (int j = 0; j < numberOfSteps; j++)
		{
			direction[0] = (double)i / numberOfSteps;
			direction[1] = 1.0;
			direction[2] = (double)j / numberOfSteps;
			result.push_back(direction / glm::length(direction));

			if (i > 0)
			{
				direction[0] = -(double)i / numberOfSteps;
				direction[1] = 1.0;
				direction[2] = (double)j / numberOfSteps;
				result.push_back(direction / glm::length(direction));
				if (j > 0)
				{
					direction[0] = -(double)i / numberOfSteps;
					direction[1] = 1.0;
					direction[2] = -(double)j / numberOfSteps;
					result.push_back(direction / glm::length(direction));
					direction[0] = (double)i / numberOfSteps;
					direction[1] = 1.0;
					direction[2] = -(double)j / numberOfSteps;
					result.push_back(direction / glm::length(direction));
				}
			}
			else if (j > 0)
			{
				direction[0] = (double)i / numberOfSteps;
				direction[1] = 1.0;
				direction[2] = -(double)j / numberOfSteps;
				result.push_back(direction / glm::length(direction));
			}
		}
	}

	//x=-1, y stays positive

	for (int i = 0; i < numberOfSteps; i++)
	{
		for (int j = 0; j < numberOfSteps; j++)
		{
			direction[0] = -1.0;
			direction[1] = (double)j / numberOfSteps;
			direction[2] = (double)i / numberOfSteps;
			result.push_back(direction / glm::length(direction));

			if (i > 0)
			{
				direction[0] = -1.0;
				direction[1] = (double)j / numberOfSteps;
				direction[2] = -(double)i / numberOfSteps;
				result.push_back(direction / glm::length(direction));
			}
		}
	}

	//x=1, y stays positive

	for (int i = 0; i < numberOfSteps; i++)
	{
		for (int j = 0; j < numberOfSteps; j++)
		{
			direction[0] = 1.0;
			direction[1] = (double)j / numberOfSteps;
			direction[2] = (double)i / numberOfSteps;
			result.push_back(direction / glm::length(direction));

			if (i > 0)
			{
				direction[0] = 1.0;
				direction[1] = (double)j / numberOfSteps;
				direction[2] = -(double)i / numberOfSteps;
				result.push_back(direction / glm::length(direction));
			}
		}
	}

	return result;
}

glm::dvec3 OctreeRayTracing::computeCenterOfMass(const std::vector<glm::dvec3>& pointList)
{
	glm::dvec3 result(0.0, 0.0, 0.0);
	for (int i = 0; i < pointList.size(); i++)
	{
		result = result + pointList[i];
	}
	result = result /(double) pointList.size();
	return result;
}

std::vector<glm::dvec3> OctreeRayTracing::substractCenterOfMass(const std::vector<glm::dvec3>& pointList, const glm::dvec3& centerOfMass)
{
	std::vector<glm::dvec3> result;
	for (int i = 0; i < pointList.size(); i++)
	{
		result.push_back(pointList[i] - centerOfMass);
	}
	return result;
}

glm::dmat3 OctreeRayTracing::preprocessData(const std::vector<glm::dvec3>& centeredDataPoints, std::vector<std::vector<double>>& F1, std::vector<std::vector<double>>& F2, std::vector<double>& mu)
{
	std::vector<std::vector<double>> deltas, deltasSkewed;

	deltasSkewed = computeDeltaSkewed(centeredDataPoints);

	mu = computeMu(deltasSkewed);

	deltas = computeDeltas(deltasSkewed, mu);
		
	glm::dmat3 F0 = computeF0(centeredDataPoints);

	F1 = computeF1(centeredDataPoints, deltas);

	F2 = computeF2(deltas);
	return F0;
}

std::vector<std::vector<double>> OctreeRayTracing::computeDeltaSkewed(const std::vector<glm::dvec3>& centeredDataPoints)
{
	std::vector<double> tempResult(6,0);
	std::vector<std::vector<double>> result;

	double x, y, z;

	for (int i = 0; i < (int)centeredDataPoints.size(); i++)
	{
		x = centeredDataPoints[i][0];
		y = centeredDataPoints[i][1];
		z = centeredDataPoints[i][2];

		tempResult[0] = x * x;
		tempResult[1] = 2 * x*y;
		tempResult[2] = 2 * x*z;
		tempResult[3] = y * y;
		tempResult[4] = 2 * y*z;
		tempResult[5] = z * z;
		result.push_back(tempResult);
	}
	return result;
}

std::vector<double> OctreeRayTracing::computeMu(const std::vector<std::vector<double>>& deltaSkewed)
{
	std::vector<double> result(6, 0);
	for (int loop = 0; loop < (int)deltaSkewed.size(); loop++)
	{
		for (int i = 0; i < 6; i++)
		{
			result[i] += deltaSkewed[loop][i];
		}
	}
	for (int i = 0; i < 6; i++)
	{
		result[i] = result[i] / (double)deltaSkewed.size();
	}
	return result;
}

std::vector<std::vector<double>>  OctreeRayTracing::computeDeltas(const std::vector<std::vector<double>>& deltaSkewed, const std::vector<double>& mu)
{
	std::vector<std::vector<double>> result;

	for (int i = 0; i < (int)deltaSkewed.size(); i++)
	{
		std::vector<double> temp;
		for (int j = 0; j < 6; j++)
		{
			temp.push_back(deltaSkewed[i][j] - mu[j]);
		}
		result.push_back(temp);
	}
	return result;
}

glm::mat3 OctreeRayTracing::computeF0(const std::vector<glm::dvec3>& centeredDataPoints)
{
	glm::mat3 result(0.0);
	glm::mat3 scale(1/(float)centeredDataPoints.size());

	for (int loop = 0; loop < (int)centeredDataPoints.size(); loop++)
	{
		result += glm::outerProduct(centeredDataPoints[loop],centeredDataPoints[loop]);
	}
	
	result =scale*result;
	return result;
}

std::vector<std::vector<double>> OctreeRayTracing::computeF1(const std::vector<glm::dvec3>& centeredDataPoints, const std::vector<std::vector<double>>& deltas)
{
	std::vector<std::vector<double>> result(3,std::vector<double>(6,0.0));	//result is a matrix of size 3 x 6

	for (int loop = 0; loop < (int) centeredDataPoints.size(); loop++)
	{
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 6; j++)
			{
				result[i][j]+= centeredDataPoints[loop][i] * deltas[loop][j];
			}
		}
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 6; j++)
		{
			result[i][j] /= (double)centeredDataPoints.size();
		}
	}
	return result;
}

std::vector<std::vector<double>> OctreeRayTracing::computeF2(const std::vector<std::vector<double>>& deltas)
{
	std::vector<std::vector<double>> result(6, std::vector<double>(6, 0.0));	//result is a matrix of size 6 x 6

	for (int loop = 0; loop < (int)deltas.size(); loop++)
	{
		for (int i = 0; i < 6; i++) {
			for (int j = 0; j < 6; j++)
			{
				result[i][j] += deltas[loop][i] * deltas[loop][j];
			}
		}
	}
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++)
		{
			result[i][j] /= (double)deltas.size();
		}
	}
	return result;
}

std::vector<std::vector<double>> OctreeRayTracing::computeOuterProductAverage(const std::vector<std::vector<double>>& X, const std::vector<std::vector<double>>& Y)
{
	std::vector<std::vector<double>> result(X[0].size(), std::vector<double>(Y[0].size(), 0.0));	

	if (((X.size()) != (Y.size())) || (X.size() == 0))		//check if X and Y have correct format
	{
		return result;
	}
	for (int loop = 0; loop < X.size(); loop++)
	{
		for (int i = 0; i < X[0].size(); i++) {
			double coord = X[loop][i];
			for (int j = 0; j < Y[0].size(); j++)
			{
				result[i][j] += coord * Y[loop][j];
			}
		}
	}
	for (int i = 0; i < X[0].size(); i++) {
		for (int j = 0; j < Y[0].size(); j++)
		{
			result[i][j] /= (double)X.size();
		}
	}
	return result;
}

glm::dvec3 OctreeRayTracing::fitCylinder(const std::vector<glm::dvec3>& directionList, glm::dvec3& center, double& radius, const glm::dmat3& F0, const std::vector<std::vector<double>>& F1, const std::vector<std::vector<double>>& F2, const std::vector<double>& mu, double& error)
{
	double errorCurrent, errorMin(DBL_MAX), radius_;
	std::vector<double> p;
	int numberOfDirections((int)directionList.size()), i(0), index(0);
	glm::dvec3 direction, alpha, beta, finalDirection;
	glm::dmat3 A, AHat, Q, P, S;

	for (i = 0; i < numberOfDirections; i++)
	{
		//direction = getDirectionFromAngles(directionList[i][0],directionList[i][1]);
		direction = directionList[i];
		direction=direction/glm::length(direction);
		P = computeProjectionMatrix(direction);
		S = computeSkewMatrix(direction);
		A = computeA(P, F0);
		AHat = computeAHat(A, S);
		Q = computeQ(A, AHat);
		p = reshapeProjection(P);
		alpha = computeAlpha(F1, p);
		beta = computeBeta(Q, alpha);

		//alternate computation //
		std::vector<std::vector<double>> tempQ(3, std::vector<double>(3, 0.0));
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				tempQ[i][j] = Q[i][j];
		std::vector<std::vector<double>> tempF0(3, std::vector<double>(3, 0.0));
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				tempF0[i][j] = F0[i][j];
		std::vector<std::vector<double>> H1,H2,H3,H4,H5;
		H1 = F2;
		H2 = scaleMatrix(multiplyMatrix(transposeMatrix(F1), multiplyMatrix(tempQ, F1)),-4);
		H3 = multiplyMatrix(tempQ, F1);
		H4 = scaleMatrix(multiplyMatrix(transposeMatrix(H3), multiplyMatrix(tempF0, H3)), 4);
		H5 = addMatrix(H1, addMatrix(H2, H4));
		/*Logger::log(LoggerMode::rayTracingLog) << "display symetric matrix : " << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << H5[0][0] << " " << H5[0][1] << " " << H5[0][2] << " " << H5[0][3] << H5[0][4] << " " << H5[0][5] << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << H5[1][0] << " " << H5[1][1] << " " << H5[1][2] << " " << H5[1][3] << H5[1][4] << " " << H5[1][5] << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << H5[2][0] << " " << H5[2][1] << " " << H5[2][2] << " " << H5[2][3] << H5[2][4] << " " << H5[2][5] << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << H5[3][0] << " " << H5[3][1] << " " << H5[3][2] << " " << H5[3][3] << H5[3][4] << " " << H5[3][5] << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << H5[4][0] << " " << H5[4][1] << " " << H5[4][2] << " " << H5[4][3] << H5[4][4] << " " << H5[4][5] << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << H5[5][0] << " " << H5[5][1] << " " << H5[5][2] << " " << H5[5][3] << H5[5][4] << " " << H5[5][5] << Logger::endl;*/

		double dissymetric = isSymetrical(H5);


		///////////////////////
		errorCurrent = computeG(p, F0, F2, alpha, beta);
		if (errorCurrent < 0)
		{
			double testError = computeDotProduct(p, applyMatrixToVector(H5, p));
			Logger::log(LoggerMode::rayTracingLog) << "test error : " << testError << Logger::endl;
			Logger::log(LoggerMode::rayTracingLog) << "G error : " << errorCurrent << Logger::endl;
			Logger::log(LoggerMode::rayTracingLog) << "dissymetric : " << dissymetric << Logger::endl;


		}
		radius_ = computeRadius(p, mu, beta);
		// cout << "new error : i=	" << i << ", error =" << error << ", diameter :"<<2*radius_ << endl;
		/*if (direction[0] > 0.998)
		{
			Logger::log(LoggerMode::rayTracingLog) << "radius : "<< radius_<< Logger::endl;
			Logger::log(LoggerMode::rayTracingLog) << "error : " << errorCurrent << Logger::endl;

		}*/
		
		if (errorCurrent < 0)
		{
			Logger::log(LoggerMode::rayTracingLog) << "problem error<0 : " << errorCurrent << Logger::endl;
			Logger::log(LoggerMode::rayTracingLog) << "direction : " << direction[0] << " " << direction[1] << " " << direction[2] << Logger::endl;
		}

		if (errorCurrent < errorMin)
		{
			errorMin = errorCurrent;
			finalDirection = direction;
			radius = radius_;
			center = glm::dvec3(beta[0], beta[1], beta[2]);
			center = P * center;
			index = i;
			// cout << "center : " << center[0] << ", " << center[1] << ", " << center[2] << endl;
			// cout << "new error : i=	"<< i<<", error =" << error << ", direction : "<<finalDirection[0]<<" "<<finalDirection[1]<<endl;
		}
	}
	//Logger::log(LoggerMode::rayTracingLog) << "errorMin : " << errorMin << Logger::endl;

	error = errorMin;
	return finalDirection;
}

glm::mat3 OctreeRayTracing::computeProjectionMatrix(const glm::dvec3& direction)
{
	glm::mat3 result, identity(1.0);
	result = identity - (glm::mat3)glm::outerProduct(direction,direction);		// result = I- W*W^T
	return result;
}

glm::mat3 OctreeRayTracing::computeSkewMatrix(const glm::dvec3& direction)
{
	glm::mat3 result;
	result[0][0] = 0;
	result[0][1] = (float)-direction[2];
	result[0][2] = (float)direction[1];
	result[1][0] = (float)direction[2];
	result[1][1] = 0;
	result[1][2] = (float)-direction[0];
	result[2][0] = (float)-direction[1];
	result[2][1] = (float)direction[0];
	result[2][2] = 0;
	return result;
}

glm::mat3 OctreeRayTracing::computeA(const glm::mat3& projectionMatrix, const glm::mat3& F0)
{
	return projectionMatrix * F0*projectionMatrix;						// result = P*F0*P
}

glm::mat3 OctreeRayTracing::computeAHat(const glm::mat3& A, const glm::mat3& skewMatrix)
{
	return skewMatrix * A*glm::transpose(skewMatrix);					// result = S*A*S^T
}

glm::mat3 OctreeRayTracing::computeQ(const glm::mat3& A, const glm::mat3& AHat)
{
	glm::mat3 B = AHat * A;
	float trace(B[0][0]+B[1][1]+B[2][2]);
	glm::mat3 scale(1/ (float)trace);

	return AHat * scale;							// result = AHat/tr(AHat*A)
}

std::vector<double> OctreeRayTracing::reshapeProjection(const glm::mat3& projectionMatrix)
{
	std::vector<double> result(6);
	result[0] = projectionMatrix[0][0];
	result[1] = projectionMatrix[0][1];
	result[2] = projectionMatrix[0][2];
	result[3] = projectionMatrix[1][1];
	result[4] = projectionMatrix[1][2];
	result[5] = projectionMatrix[2][2];
	return result;
}

glm::dvec3 OctreeRayTracing::computeAlpha(const std::vector<std::vector<double>>& F1, const std::vector<double>& reshapedProjection)
{
	std::vector<double> temp = applyMatrixToVector(F1, reshapedProjection);
	if(temp.size()!=3)
		Logger::log(LoggerMode::rayTracingLog) << "problem in computeAlpha" << Logger::endl;

	return glm::dvec3(temp[0], temp[1], temp[2]);
}

glm::dvec3 OctreeRayTracing::computeBeta(const glm::mat3& Q, const glm::dvec3& alpha)
{
	return Q * alpha;
}

double OctreeRayTracing::computeG(const std::vector<double>& reshapedProjection, const glm::mat3& F0, const std::vector<std::vector<double>>& F2, const glm::dvec3& alpha, const glm::dvec3& beta)
{
	double result(0),a,b,c;

	result += computeDotProduct(reshapedProjection, applyMatrixToVector(F2, reshapedProjection));
	a = result;
	glm::dvec3 temp = F0 * beta;
	result += 4 * glm::dot(beta, temp);
	b = result;
	result -= 4 * glm::dot(alpha,beta);
	c = result;
	if (c < 0)
	{
		Logger::log(LoggerMode::rayTracingLog) << "display stuff in computeG" << Logger::endl;

		Logger::log(LoggerMode::rayTracingLog) << a << " " << b << " " << c << Logger::endl;
	}
	
	return result;
}

double OctreeRayTracing::computeRadius(const std::vector<double>& reshapedProjection, const std::vector<double>& mu, const glm::dvec3& beta)
{
	double result(0);
	result = computeDotProduct(reshapedProjection, mu) + glm::dot(beta,beta);
	result = sqrt(result);
	return result;
}

std::vector<double> OctreeRayTracing::applyMatrixToVector(const std::vector<std::vector<double>>& matrix, const std::vector<double>& V)
{
	std::vector<double> result(matrix.size(),0.0);
	if (matrix.size() == 0) { 
		Logger::log(LoggerMode::rayTracingLog) << "problem in applyMatrixToVector" << Logger::endl;
		return result; }
	if (matrix[0].size() != V.size()) {
		Logger::log(LoggerMode::rayTracingLog) << "problem in applyMatrixToVector" << Logger::endl;
		return result; }

	for (int i = 0; i < (int)matrix.size(); i++)
	{
		for (int j = 0; j < (int)V.size(); j++)
		{
			result[i] += (double)matrix[i][j] * V[j];
		}
	}
	return result;
}

double OctreeRayTracing::computeDotProduct(const std::vector<double>& x, const std::vector<double>& y)
{
	double result(0);
	if (x.size() != y.size()) { 
		Logger::log(LoggerMode::rayTracingLog) << "problem in computeDotProduct" << Logger::endl;
		return result; }
	for (int loop = 0; loop < (int)x.size(); loop++)
	{
		result += x[loop] * y[loop];
	}
	return result;
}

std::vector<glm::dvec3> OctreeRayTracing::refineDirectionList(const glm::dvec3& direction, const double& range, const int& numberOfSteps)
{
	glm::dvec3 v, w, newDirection;
	int i(0), j(0);
	std::vector<glm::dvec3> result;
	completeVectorToOrthonormalBasis(direction, v, w);
	for (i = 0; i < numberOfSteps; i++)
	{
		for (j = 0; j < numberOfSteps; j++)
		{
			newDirection = direction + (range*i /(double) numberOfSteps) *v;
			newDirection = newDirection +(range*j /(double) numberOfSteps)*w;
			newDirection = newDirection / glm::length(newDirection);
			result.push_back(newDirection);

			newDirection = direction - (range*i / (double)numberOfSteps) *v;
			newDirection = newDirection + (range*j / (double)numberOfSteps)*w;
			newDirection = newDirection / glm::length(newDirection);
			result.push_back(newDirection);

			newDirection = direction + (range*i / (double)numberOfSteps) *v;
			newDirection = newDirection - (range*j / (double)numberOfSteps)*w;
			newDirection = newDirection / glm::length(newDirection);
			result.push_back(newDirection);

			newDirection = direction - (range*i / (double)numberOfSteps) *v;
			newDirection = newDirection - (range*j / (double)numberOfSteps)*w;
			newDirection = newDirection / glm::length(newDirection);
			result.push_back(newDirection);
		}
	}
	return result;
}

void OctreeRayTracing::completeVectorToOrthonormalBasis(const glm::dvec3& u, glm::dvec3& v, glm::dvec3& w)
{
	int i(0);
	double temp(abs(u[0]));
	if (temp > abs(u[1]))
	{
		temp = abs(u[1]);
		i = 1;
	}
	if (temp > abs(u[2]))									// i = argmin(abs(u[i]))

	{
		i = 2;
	}

	if (i == 0)
	{
		v[0] = 0;
		v[1] = u[2];
		v[2] = -u[1];
	}
	if (i == 1)									// to choose v, discard the lowest u coordinate then pick the obvious vector
	{											// this is done for numerical robustness purposes
		v[0] = u[2];
		v[1] = 0;
		v[2] = -u[0];
	}
	if (i == 2)
	{
		v[0] = u[1];
		v[1] = -u[0];
		v[2] = 0;
	}
	v = v / glm::length(v);;
	w = glm::cross(u,v);
	w=w/glm::length(w);							// not necessary, but probably a good safety measure
}

double OctreeRayTracing::computeMeanSquareCylinderDistance(const std::vector<glm::dvec3>& points, const glm::dvec3& center, const glm::mat3& projectionMatrix, const double& radius)
{
	glm::dvec3 currentPoint;
	double result(0);
	for (int loop = 0; loop < points.size(); loop++)
	{
		currentPoint = points[loop];
		double distance = computePointToCylinderDistance(currentPoint, center, projectionMatrix, radius);
		result += distance * distance;
	}
	result /= (double)points.size();
	result = sqrt(result);
	return result;
}

double OctreeRayTracing::computePointToCylinderDistance(const glm::dvec3& point, const glm::dvec3& center, const glm::mat3& projectionMatrix, const double& radius)
{
	double distance;
	glm::dvec3 temp = projectionMatrix * center - projectionMatrix * point;
	distance = abs(radius - glm::length(temp));
	return distance;
}

std::vector<glm::dvec3> OctreeRayTracing::samplePoints(const glm::dvec3& center, const double& radius, const glm::dvec3& direction, const double& height, const int& numberOfTestPoints)
{
	glm::dvec3 v, w, radialDirection, point;
	std::vector<glm::dvec3> result;
	completeVectorToOrthonormalBasis(direction, v, w);
	double angle(0);
	for (int loop = 0; loop < numberOfTestPoints; loop++)
	{
		angle = (double)loop*M_PI / (double)numberOfTestPoints;
		radialDirection = cos(angle)*v + sin(angle)*w;
		radialDirection = radialDirection / glm::length(radialDirection);
		point = center + height * direction + radius * radialDirection;

		result.push_back(point);
		radialDirection = -radialDirection;
		point = center + height * direction + radius * radialDirection;
		result.push_back(point);
	}

	return result;
}


void OctreeRayTracing::displayBoundingBox(const std::vector<glm::dvec3>& globalPointList)
{
	double minX(DBL_MAX), minY(DBL_MAX), minZ(DBL_MAX), maxX(-DBL_MAX), maxY(-DBL_MAX), maxZ(-DBL_MAX);
	for (int loop = 0; loop < globalPointList.size(); loop++)
	{
		double x(globalPointList[loop][0]), y(globalPointList[loop][1]), z(globalPointList[loop][2]);
		if (x < minX) { minX = x; }
		if (y < minY) { minY = y; }
		if (z < minZ) { minZ = z; }
		if (x > maxX) { maxX = x; }
		if (y > maxY) { maxY = y; }
		if (z > maxZ) { maxZ = z; }
	}
	Logger::log(LoggerMode::rayTracingLog) << "DISPLAY BOUNDING BOX OF BALLPOINTS :" << Logger::endl << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "x : " << minX << " " << maxX << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "y : " << minY << " " << maxY << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "z : " << minZ << " " << maxZ << Logger::endl;
	return;
}

void OctreeRayTracing::appendBucketToList(const std::vector<std::vector<glm::dvec3>>& globalPointBuckets, std::vector<glm::dvec3>& pointList, const int& bucketNumber)
{

	if (bucketNumber >= globalPointBuckets.size())
	{
		Logger::log(LoggerMode::rayTracingLog) << "problem in convertBucketsIntoList" << Logger::endl;
		return ;
	}
	for (int i = 0; i < globalPointBuckets[bucketNumber].size(); i++)
	{
		pointList.push_back(globalPointBuckets[bucketNumber][i]);
	}

	return ;
}

std::vector<glm::dvec3> OctreeRayTracing::getPointsInCylinder(const std::vector<glm::dvec3>& pointList, const glm::dvec3& cylinderDirection, const glm::dvec3& cylinderCenter, const double& cylinderRadius, const glm::dvec3& centerOfMass, const double& radiusThreshold)
{
	std::vector<glm::dvec3> result;
	double angleThreshold(0.005);
	for (int loop = 0; loop < pointList.size(); loop++)
	{
		double dist = computePointToCylinderDistance(pointList[loop], cylinderCenter, computeProjectionMatrix(cylinderDirection), cylinderRadius);
		double currentHeight = glm::dot(pointList[loop] - centerOfMass, cylinderDirection);
		if (dist < ((radiusThreshold)*(1 + angleThreshold * abs(currentHeight))))
		{
			result.push_back(pointList[loop]);
		}
	}
	return result;
}

bool OctreeRayTracing::findBiggestBend(const std::vector<glm::dvec3>& neighbors, const std::vector<glm::dvec3>& discreteLine, const glm::dvec3& normalVector, glm::dvec3& bendPoint, double& maxBend, double& ratio)
{
	int indexRange(3),numberOfSteps((int)neighbors.size());													// error margin for occlusions (instead of 2, could be |ab|*margin/numberOfSteps)
	
	// locate occlusions
	std::vector<double> bendList(numberOfSteps);
	bendList = makeBendList(neighbors, discreteLine, normalVector);

	std::vector<bool> confidentList1(numberOfSteps), confidentList2(numberOfSteps), mergedConfidenceList(numberOfSteps), finalConfidentList(numberOfSteps);
	std::vector<double> bendDerivated(numberOfSteps - 1);
	confidentList1 = computeSharpTransition(bendList);										// points that are unusually far from mean are tagged as "true"
	Logger::log(LoggerMode::rayTracingLog) << "displaying confidentList1 ..." << Logger::endl;
	for (int i = 0; i < confidentList1.size(); i++)
	{
		if (confidentList1[i])
		{
			Logger::log(LoggerMode::rayTracingLog) << i << Logger::endl;
		}
	}
	bendDerivated = computeAbsoluteDiscreteDerivative(bendList);								// computes |u[i+1]-u[i]|

	confidentList2 = computeSharpTransition(bendDerivated);								// points that are unusually far from their closest neighbors are tagged as "true"
	Logger::log(LoggerMode::rayTracingLog) << "displaying confidentList2 ..." << Logger::endl;
	for (int i = 0; i < confidentList2.size(); i++)
	{
		if (confidentList2[i])
		{
			Logger::log(LoggerMode::rayTracingLog) << i << Logger::endl;
		}
	}
	mergedConfidenceList = mergeConfidenceList(confidentList1, confidentList2);				// index wise OR operator

	//display merged confidence list : 
	Logger::log(LoggerMode::rayTracingLog) << "displaying mergedConfidentList ..." << Logger::endl;

	for (int i = 0; i < mergedConfidenceList.size(); i++)
	{
		if (mergedConfidenceList[i])
		{
			Logger::log(LoggerMode::rayTracingLog) << i << Logger::endl;
		}
	}
	Logger::log(LoggerMode::rayTracingLog) << "normalVector.z : " << normalVector.z << Logger::endl;


	finalConfidentList = updateConfidenceList(mergedConfidenceList, indexRange);				// adds a margin of size indexRange

	// find biggest bend, excluding occluded points

	std::vector<double> smoothedBendList(numberOfSteps);															// each bend is averaged with its neighbors to mitigate noise																									
	int indexOFBiggestBend(0);

	smoothedBendList = smoothenList(bendList);


	//display bendList 
	for (int i = 0; i < smoothedBendList.size(); i++)
	{
		Logger::log(LoggerMode::rayTracingLog) << "bend[" << i << "] : " << bendList[i] << Logger::endl;
	}
	Logger::log(LoggerMode::rayTracingLog) << "normalVector.z : " << normalVector.z << Logger::endl;

	indexOFBiggestBend = findBiggestBendInList(smoothedBendList, finalConfidentList);
	Logger::log(LoggerMode::rayTracingLog) << "indexOfBiggestBend : " << indexOFBiggestBend << Logger::endl;

	bendPoint = neighbors[indexOFBiggestBend];
	maxBend = smoothedBendList[indexOFBiggestBend];
	ratio = (double)1000 * maxBend / glm::length(discreteLine[0] - discreteLine[(int)discreteLine.size() - 1]);
	return true;
}

void OctreeRayTracing::updateNormalVector(glm::dvec3& normalVector, const glm::dvec3& a, const glm::dvec3& b)
{
	glm::dvec3 temp = a - b;
	temp = temp / glm::length(temp);
	normalVector = normalVector - glm::dot(normalVector, temp)*(temp);
	normalVector = normalVector / glm::length(normalVector);
}

std::vector<glm::dvec3> OctreeRayTracing::discretize(const glm::dvec3& tip1, const glm::dvec3& tip2, const int& numberOfSteps)

{
	std::vector<glm::dvec3> result;
	glm::dvec3 ray(3), dummy(3);
	ray = tip2 - tip1;											// ray holds the line direction from tip1 to tip2
																					// dummy is just a placeholder used to increment the size of result
	//ray = scaleVector(ray, 1 / mynorm(ray));

	for (int i = 0; i < numberOfSteps; i++)
	{
		result.push_back(dummy);
		result[i][0] = tip1[0] + i * ray[0] / (numberOfSteps - 1);
		result[i][1] = tip1[1] + i * ray[1] / (numberOfSteps - 1);
		result[i][2] = tip1[2] + i * ray[2] / (numberOfSteps - 1);
	}

	return result;
}

std::vector<bool> OctreeRayTracing::computeSharpTransition(const std::vector<double>& values)
{
	std::vector<bool> result(values.size());
	double mean, standardDev;
	mean = computeMean(values);
	standardDev = computeSD(values);
	int i(0);
	for (i = 0; i < values.size(); i++)
	{
		if (standardDev > 0.005)
		{
			result[i] = (values[i] > mean + 2.5 * standardDev);
		}
		else result[i] = (values[i]>mean+0.02);
		if (result[i])
		{
			Logger::log(LoggerMode::rayTracingLog) << "SD : " << standardDev << Logger::endl;
			Logger::log(LoggerMode::rayTracingLog) << "mean : " << mean << Logger::endl;
			Logger::log(LoggerMode::rayTracingLog) << "value : " << values[i] << Logger::endl;

		}
	}
	return result;
}

double OctreeRayTracing::computeMean(const std::vector<double>& values)
{
	double result(0);
	int i(0), size;
	size = (int)values.size();
	for (i = 0; i < size; i++)
	{
		result += values[i];
	}
	result = result / size;
	return result;
}

double OctreeRayTracing::computeSD(const std::vector<double>& values)
{
	double result(0), mean;
	int i(0),size((int)values.size());
	mean = computeMean(values);
	for (i = 0; i < size; i++)
	{
		result += (mean - values[i])*(mean - values[i]);
	}
	result = result / size;
	result = sqrt(result);
	return result;
}

std::vector<double> OctreeRayTracing::computeAbsoluteDiscreteDerivative(const std::vector<double>& values)
{
	std::vector<double> result(values.size() - 1);
	int i(0);
	for (i = 0; i < values.size() - 1; i++)
	{
		result[i] = abs(values[i + 1] - values[i]);
	}
	return result;
}

std::vector<bool> OctreeRayTracing::mergeConfidenceList(const std::vector<bool>& confidenceList1, const std::vector<bool>& confidenceList2)
{
	int i(0),size((int)confidenceList1.size());
	std::vector<bool> result(size);
	result[0] = confidenceList1[0] || confidenceList2[0];
	result[size - 1] = confidenceList1[size - 1] || confidenceList2[size - 2];
	for (i = 1; i < size - 2; i++)
	{
		result[i] = (confidenceList1[i] || confidenceList2[i]) || confidenceList2[i - 1];
	}

	return result;
}

std::vector<bool> OctreeRayTracing::updateConfidenceList(const std::vector<bool>& confidenceList, const int& indexMargin)
{
	int size((int)confidenceList.size());
	bool temp(false);
	std::vector<bool> result(size);
	for (int i = 0; i < size; i++)
	{
		temp = false;
		for (int j = -indexMargin; j < indexMargin + 1; j++)
		{
			if ((i + j > -1) && (i + j < size))
			{
				temp = temp || confidenceList[i + j];
			}
			if (temp)
			{
				break;
			}
		}
		result[i] = temp;
	}
	return result;
}

std::vector<double> OctreeRayTracing::smoothenList(const std::vector<double>& bendList)
{
	int size((int)bendList.size());
	std::vector<double> result(size);
	result[0] = bendList[0] * 3 / 4 + bendList[1] * 1 / 4;
	result[size - 1] = bendList[size - 1] * 3 / 4 + bendList[size - 2] * 1 / 4;
	for (int i = 1; i < size - 1; i++)
	{
		result[i] = bendList[i] / 2 + bendList[i - 1] / 4 + bendList[i + 1] / 4;
	}
	return result;
}

int OctreeRayTracing::findBiggestBendInList(const std::vector<double>& bendList, const std::vector<bool>& confidenceList)
{
	int index(0);
	double bendMax(0);
	for (int i = 0; i < bendList.size(); i++)
	{
		if ((!confidenceList[i]) && (bendList[i] > bendMax))
		{
			bendMax = bendList[i];
			index = i;
		}
	}
	return index;
}

std::vector<double> OctreeRayTracing::makeBendList(const std::vector<glm::dvec3>& neighbors, const std::vector<glm::dvec3>& discreteLine, const glm::dvec3& normalVector)
{
	std::vector<double> result(neighbors.size());
	int i(0);
	for (i = 0; i < neighbors.size(); i++)
	{
		result[i] = glm::dot(neighbors[i] - discreteLine[i], normalVector);
	}
	return result;
}

bool OctreeRayTracing::columnOffset(const glm::dvec3& camPosition, glm::dvec3& wallPoint1, const glm::dvec3& wallPoint2, const glm::dvec3& columnPoint1, const glm::dvec3 columnPoint2, double& offset, double& ratio)
{
	glm::dvec3 normalVector = glm::cross(wallPoint1 - wallPoint2, glm::dvec3(0.0, 0.0, 1.0));
	normalVector = normalVector / glm::length(normalVector);
	if (glm::dot(normalVector, wallPoint1 - camPosition) > 0)
		normalVector = -normalVector;
	offset = glm::dot(columnPoint1 - columnPoint2, normalVector);
	if (columnPoint1.z > columnPoint2.z)
		offset = -offset;
	ratio = offset / glm::length(columnPoint1 - columnPoint2);
	return true;
}

void OctreeRayTracing::samplePointsUpToBucket(const std::vector<std::vector<glm::dvec3>>& globalPointBuckets, const int& bucketNumber, std::vector<glm::dvec3>& result)
{
	result = std::vector<glm::dvec3>(0);
	for (int i = 0; i < bucketNumber; i++)
	{
		for (int j = 0; j < globalPointBuckets[i].size(); j++)
			result.push_back(globalPointBuckets[i][j]);
	}
}

			///////////pointToPlane measure/////////////////

bool OctreeRayTracing::fitPlane(const std::vector<glm::dvec3>& points, std::vector<double>& result)
{
	//Logger::log(LoggerMode::rayTracingLog) << "plane fit start" << Logger::endl;
	if (points.size() < 3)
		return false;
	//Scandata for smallest coordinate range 
	double xMin(DBL_MAX), yMin(DBL_MAX), zMin(DBL_MAX), xMax(-DBL_MAX), yMax(-DBL_MAX), zMax(-DBL_MAX);
	for (int i = 0; i < (int)points.size(); i++)
	{
		if (points[i][0] < xMin)
			xMin = points[i][0];
		if (points[i][0] > xMax)
			xMax = points[i][0];
		if (points[i][1] < yMin)
			yMin = points[i][1];
		if (points[i][1] > yMax)
			yMax = points[i][1];
		if (points[i][2] < zMin)
			zMin = points[i][2];
		if (points[i][2] > zMax)
			zMax = points[i][2];
	}

	int relevantDimension(0);
	if (((xMax - xMin) < (yMax - yMin)) && ((xMax - xMin) < (zMax - zMin)))
		relevantDimension = 0;
	if (((yMax - yMin) < (xMax - xMin)) && ((yMax - yMin) < (zMax - zMin)))
		relevantDimension = 1;
	if (((zMax - zMin) < (xMax - xMin)) && ((zMax - zMin) < (xMax - xMin)))
		relevantDimension = 2;
	int c(relevantDimension), b((relevantDimension + 1) % 3), a((relevantDimension + 2) % 3);
	result = std::vector<double>(0);
	double xxSum(0), xySum(0), xhSum(0), yySum(0), yhSum(0);
	double barA0, barA1, barH, barX, barY;
	glm::dvec3 centerOfMass = computeCenterOfMass(points);
	for (int loop = 0; loop < (int)points.size(); loop++)
	{
		glm::dvec3 diff = points[loop] - centerOfMass;
		xxSum += diff[a] * diff[a];
		xySum += diff[a] * diff[b];
		xhSum += diff[a] * diff[c];
		yySum += diff[b] * diff[b];
		yhSum += diff[b] * diff[c];
	}
	double det = xxSum * yySum - xySum * xySum;
	if (100*det == 0)
	{
		Logger::log(LoggerMode::rayTracingLog) << "plane fit failed, #points : " << points.size() << Logger::endl;
		return false;
	}
	else {
		barA0 = (yySum*xhSum - xySum * yhSum) / det;
		barA1 = (xxSum*yhSum - xySum * xhSum) / det;
		barX = centerOfMass[a];
		barY = centerOfMass[b];
		barH = centerOfMass[c];
	}
	result = std::vector<double>(4);
	result[a]=barA0;			// result=(a,b,c,d) represents the plane ax+by+cz+d=0
	result[b]=barA1;
	result[c]=-1;
	result[3]=barH - barA0 * barX - barY * barA1;
	double length = glm::length(glm::dvec3(result[0], result[1], result[2]));
	for (int i = 0; i < 4; i++)
		result[i] /= length;
	//Logger::log(LoggerMode::rayTracingLog) << "plane fit end" << Logger::endl;


	return true;
}

glm::dvec3 OctreeRayTracing::pickPointOnPlane(const std::vector<double>& plane)
{
	glm::dvec3 planePoint = glm::dvec3(0.0, 0.0, 0.0);
	if ((abs(plane[0]) >= abs(plane[1])) && (abs(plane[0]) >= abs(plane[2])))
		planePoint = glm::dvec3(-plane[3] / plane[0], 0.0, 0.0);
	if ((abs(plane[1]) >= abs(plane[0])) && (abs(plane[1]) >= abs(plane[2])))
		planePoint = glm::dvec3(0.0, -plane[3] / plane[1], 0.0);
	if ((abs(plane[2]) >= abs(plane[0])) && (abs(plane[2]) >= abs(plane[1])))
		planePoint = glm::dvec3(0.0, 0.0, -plane[3] / plane[2]);
	return planePoint;
}

bool OctreeRayTracing::fitPlaneBuckets(const std::vector<std::vector<glm::dvec3>>& points, std::vector<double>& result)
{
	std::vector<glm::dvec3> pointList = points[0];
	std::vector<double> lastResult;
	if (!fitPlane(pointList, result))
		return false;
	lastResult = result;
	for (int i = 1; i < (int)points.size(); i++)
	{
		if (!fitPlane(pointList, result))
		{
			result = lastResult;
			return true;
		}
		if (!arePlanesSimilar(result, lastResult))
		{
			result = lastResult;
			return true;
		}
		else lastResult = result;
		pointList.insert(pointList.end(), points[i].begin(), points[i].end());
	}
	return true;
}
//make a function that tells if 2 planes are close enough

bool OctreeRayTracing::arePlanesSimilar(const std::vector<double>& plane1, const std::vector<double>& plane2)
{
	glm::dvec3 normal1, normal2;
	normal1 = glm::dvec3(plane1[0], plane1[1], plane1[2]);
	normal1 /= glm::length(normal1);
	normal2 = glm::dvec3(plane2[0], plane2[1], plane2[2]);
	normal2 /= glm::length(normal2);
	double normalAngle = abs(glm::dot(normal1, normal2));
	Logger::log(LoggerMode::rayTracingLog) << "normalAngle : " << 1-normalAngle << Logger::endl;

	if (normalAngle < 0.9999)
		return false;
	/*glm::dvec3 point = pickPointOnPlane(plane1);
	double planeDistance = pointToPlaneDistance(point, plane2);
	if (planeDistance > 0.002)
	{
		Logger::log(LoggerMode::rayTracingLog) << "planeDistance : " << planeDistance << Logger::endl;
		return false;
	}*/
	return true;
}


double OctreeRayTracing::computeMeanSquaredDistanceToPlane(const std::vector<glm::dvec3>& points, std::vector<double> plane)
{
	double result(0);
	for (int loop = 0; loop < points.size(); loop++)
	{
		double distance = pointToPlaneDistance(points[loop],plane);
		distance *= distance;
		result += distance;
	}
	result /= (double)points.size();
	result = sqrt(result);
	return result;
}
bool OctreeRayTracing::pointToPlaneMeasure(const std::vector<std::vector<glm::dvec3>>& pointBuckets, const glm::dvec3& seedPoint, double& distance)
{
	std::vector<double> plane;
	if (!fitPlaneBuckets(pointBuckets, plane))
		return false;
	distance = pointToPlaneDistance(seedPoint, plane);
	return true;
}

double OctreeRayTracing::pointToPlaneDistance(const glm::dvec3& point, const std::vector<double>& plane)
{
	if ((int)plane.size() == 4)
	{
		glm::dvec3 normal(plane[0], plane[1], plane[2]);
		normal /= glm::length(normal);
		glm::dvec3 planePoint = pickPointOnPlane(plane);

		return abs(glm::dot(point - planePoint, normal));
	}
	else return 0.0;
}

std::vector<glm::dvec3> OctreeRayTracing::sampleDirectionsXY(const int& numberOfDirections, const glm::dvec3& normalVector)
{
	std::vector<glm::dvec3> result;
	glm::dvec3 X, Y;
	completeVectorToOrthonormalBasis(normalVector, X, Y);
	
	double angle;
	for (int loop = 0; loop < numberOfDirections; loop++)
	{
		angle = M_PI * loop / (double)numberOfDirections;
		glm::dvec3 temp(sin(angle)*X + cos(angle)*Y);
		result.push_back(temp);
	}
	return result;
}

std::vector<std::vector<double>> OctreeRayTracing::computeRangeAlongDirection(const std::vector<glm::dvec3>& points, const glm::dvec3& direction, const glm::dvec3& seedPoint, const glm::dvec3& normalVector, glm::dvec3& orthoDir)
{
	std::vector<std::vector<double>> result;
	double dirMin(DBL_MAX), dirMax(-DBL_MAX), orthoDirMin(DBL_MAX), orthoDirMax(-DBL_MAX);
	glm::dvec3 orthoDirection = glm::cross(direction,normalVector);
	for (int loop = 0; loop < points.size(); loop++)
	{
		glm::dvec3 currentPoint =points[loop];
		double currentDir = glm::dot(currentPoint - seedPoint, direction);
		double orthoDir = glm::dot(currentPoint - seedPoint, orthoDirection);
		if (currentDir < dirMin) { dirMin = currentDir; }
		if (currentDir > dirMax) { dirMax = currentDir; }
		if (orthoDir < orthoDirMin) { orthoDirMin = orthoDir; }
		if (orthoDir > orthoDirMax) { orthoDirMax = orthoDir; }
	}
	std::vector<double> temp;
	temp.push_back(dirMin);
	temp.push_back(dirMax);
	result.push_back(temp);
	temp[0] = orthoDirMin;
	temp[1] = orthoDirMax;
	result.push_back(temp);
	orthoDir = orthoDirection;
	return result;
}

std::vector<double> OctreeRayTracing::computeRangeAlongDirection2(const std::vector<glm::dvec3>& points, const glm::dvec3& direction, const glm::dvec3& seedPoint, const glm::dvec3& normalVector, glm::dvec3& orthoDir)
{
	double pointWidth(0.03);
	BeamDirectionRange beamRange= BeamDirectionRange(pointWidth);
	for (int loop = 0; loop < points.size(); loop++)
	{
		beamRange.addPoint(glm::dot(points[loop]-seedPoint,direction));
	}
	orthoDir = glm::cross(normalVector, direction);
	return beamRange.getCentralInterval();
}
double OctreeRayTracing::getValueOfRange(const std::vector<std::vector<double>>& range)
{
	return range[0][1] - range[0][0] - range[1][1] + range[1][0];			//returns directionRange - orthoDirRange
}

glm::dvec3 OctreeRayTracing::findBeamDirection(const std::vector<glm::dvec3>& points, const int& numberOfDirections, std::vector<std::vector<double>>& directionRange, const glm::dvec3& seedPoint, const glm::dvec3& normalVector, glm::dvec3& orthoDir)
{
	std::vector<glm::dvec3> directionList = sampleDirectionsXY(numberOfDirections, normalVector);
	double maxRange(0), minRange(DBL_MAX),maxRatio(0);
	//std::vector<std::vector<double>> currentRange;
	std::vector<double> currentRange;
	glm::dvec3 bestOrthoDirection,bestDir;
	for (int loop = 0; loop < directionList.size(); loop++)
	{
		currentRange = computeRangeAlongDirection2(points, directionList[loop], seedPoint, normalVector,orthoDir);
		if ((currentRange[1] - currentRange[0]) < minRange)
		{
			minRange = currentRange[1] - currentRange[0];
			bestOrthoDirection = directionList[loop];
			bestDir = orthoDir;
		}
	}
	std::vector<double> temp = computeRangeAlongDirection2(points, bestDir, seedPoint, normalVector, bestOrthoDirection);
	directionRange.push_back(temp);
	glm::dvec3 temp1;
	temp= computeRangeAlongDirection2(points, bestOrthoDirection, seedPoint, normalVector, temp1);
	orthoDir = bestOrthoDirection;
	directionRange.push_back(temp);
	return bestDir;
}

std::vector<glm::dvec3> OctreeRayTracing::getPointsAlongDirection(const std::vector<glm::dvec3>& points, const glm::dvec3& seedPoint, const glm::dvec3& direction, const glm::dvec3& orthoDirection, const std::vector<double>& plane)
{
	std::vector<glm::dvec3> result;
	double distanceThreshold(0.01);
	double dirRadius(0.5), orthoRadius(0.03);
	for (int i = 0; i < (int)points.size(); i++)
	{
		glm::dvec3 currPoint = points[i]-seedPoint;
		double d1 = glm::dot(currPoint,direction);
		double d2 = glm::dot(currPoint,orthoDirection);
		if ((d1 > -dirRadius) && (d1 < dirRadius) && (d2 > -orthoRadius) && (d2 < orthoRadius)&&(pointToPlaneDistance(currPoint+seedPoint,plane)<distanceThreshold))	
			result.push_back(currPoint+seedPoint);		
	}
	return result;
}

std::vector<glm::dvec3> OctreeRayTracing::countPointsNearHeight(const double& height, const double& heightThreshold, const std::vector<glm::dvec3>& points, glm::dvec3 seedPoint, glm::dvec3 normalVector)
{
	std::vector<glm::dvec3> result(0);	
	for(int i=0;i<points.size();i++)
	{
		glm::dvec3 currentPoint = points[i];		
		if (abs(glm::dot(currentPoint-seedPoint, normalVector) - height) < heightThreshold)
			result.push_back(currentPoint);
	}
	return result;
}

double OctreeRayTracing::findBeamHeight(const std::vector<glm::dvec3>& points, const glm::dvec3& seedPoint, const glm::dvec3& normalVector, const double& heightMax, const glm::dvec3& direction, const glm::dvec3& orthoDir, const std::vector<double>& range)
{
	//idée : réutiliser BeamDirectionRange pour checker si les nombreux points de la semelle supérieure couvrent une part non négligeable de la zone anticipée
	double result(0), heightStep(0.002),heightThreshold(0.0005),heightStart(0.01), rangeWidth(abs(range[0] - range[1]));
	size_t previousNumber(0),maxPoint(0);
	std::vector<double> heights = sampleHeights(heightMax, heightStep);
	Logger::log(LoggerMode::rayTracingLog) << "range : " << rangeWidth << Logger::endl;

	for (int i = 0; i < (int)heights.size(); i++)
	{
		std::vector<glm::dvec3> currHeightPoints(0);
		currHeightPoints = countPointsNearHeight(heights[i] + heightStart, heightThreshold, points, seedPoint, normalVector);
		Logger::log(LoggerMode::rayTracingLog) << "point number for that height : " << currHeightPoints.size() << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << "height= " << heights[i] + heightStart << Logger::endl;
		glm::dvec3 tempDir=direction;
		std::vector<double> currRange = computeRangeAlongDirection2(currHeightPoints, orthoDir, seedPoint, normalVector, tempDir);
		if ((int)currRange.size() < 2)
			continue;
		double currRangeWidth(abs(currRange[0] - currRange[1]));
		Logger::log(LoggerMode::rayTracingLog) << "(currRange ; range) : (" << currRangeWidth << " ; " << rangeWidth << ")" << Logger::endl;
		if (i == 0)
			previousNumber = (int) currHeightPoints.size();
		else if (currHeightPoints.size() > 2*previousNumber)
		{
			if (previousNumber < 20)
			{
				previousNumber = (int) currHeightPoints.size();
			}
			
			

			if ((abs(currRange[0] - currRange[1])) > 0.4 * abs(range[0] - range[1]))
			{
				result = heights[i] + heightStart;
				return result;
			}	
		}
		if ((int) currHeightPoints.size() > maxPoint)
		{
			maxPoint = (int) currHeightPoints.size();
			result = heights[i] + heightStart;
		}
		previousNumber = (int) currHeightPoints.size();
	}
	return result;
}

std::vector<double> OctreeRayTracing::sampleHeights(const double& heightMax, const double& heightStep)
{
	std::vector<double> result;
	int steps = (int)((heightMax - 0.05) /heightStep);
	for (int i = 0; i < steps; i++)
	{
		result.push_back(0.05 + i * heightStep);
	}
	return result;
}

std::vector<std::vector<double>> OctreeRayTracing::multiplyMatrix(const std::vector<std::vector<double>>& A, const std::vector<std::vector<double>>& B)
{
	if (((int)A.size() == 0) || ((int)B.size() == 0) || (A[0].size() != B.size()))
	{
		Logger::log(LoggerMode::rayTracingLog) << "problem in multiplyMatrix" << Logger::endl;
		return std::vector<std::vector<double>>(0);
	}

	std::vector<std::vector<double>> result((int)A.size(), std::vector<double>((int)B[0].size(),0.0));
	for (int i = 0; i < (int)A.size(); i++)
	{
		for (int j = 0; j < (int)B[0].size(); j++)
		{
			for (int k = 0; k < (int)B.size(); k++)
				result[i][j] += A[i][k] * B[k][j];
		}
	}
	return result;
}

std::vector<std::vector<double>> OctreeRayTracing::scaleMatrix(const std::vector<std::vector<double>>& A, const double& lambda)
{
	std::vector<std::vector<double>> result(A.size(), std::vector<double>(A[0].size(), 0.0));
	for (int i = 0; i < (int)A.size(); i++)
	{
		for (int j = 0; j < (int)A[0].size(); j++)
		{
			result[i][j] = A[i][j] * lambda;
		}
	}
	return result;
}

std::vector<std::vector<double>> OctreeRayTracing::addMatrix(const std::vector<std::vector<double>>& A, const std::vector<std::vector<double>>& B)
{
	std::vector<std::vector<double>> result(A.size(), std::vector<double>(A[0].size(), 0.0));
	for (int i = 0; i < (int)A.size(); i++)
	{
		for (int j = 0; j < (int)A[0].size(); j++)
		{
			result[i][j] = A[i][j]+B[i][j];
		}
	}
	return result;
}

std::vector<std::vector<double>> OctreeRayTracing::transposeMatrix(const std::vector<std::vector<double>>& A)
{
	std::vector<std::vector<double>> result(A[0].size(), std::vector<double>(A.size(), 0.0));
	for (int i = 0; i < (int)A.size(); i++)
	{
		for (int j = 0; j < (int)A[0].size(); j++)
		{
			result[j][i] = A[i][j];
		}
	}
	return result;
}

double OctreeRayTracing::isSymetrical(const std::vector<std::vector<double>>& A)
{
	double result(-1);
	int i0, j0;
	if(A.size()==0)
		Logger::log(LoggerMode::rayTracingLog) << "matrix is empty" << Logger::endl;

	if (A.size() != A[0].size())
	{
		Logger::log(LoggerMode::rayTracingLog) << "matrix isnt square" << Logger::endl;
		return result;
	}
	for (int i = 0; i < (int)A.size(); i++)
	{
		for (int j = i; j < (int)A.size(); j++)
		{
			if (result < abs(A[i][j] - A[j][i]))
			{
				result = abs(A[i][j] - A[j][i]);
				i0 = i;
				j0 = j;
			}
		}
	}
	/*if (result > 0.00001)
	{
		Logger::log(LoggerMode::rayTracingLog) << "i = " << i0 << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << "j = " << j0 << Logger::endl;
		Logger::log(LoggerMode::rayTracingLog) << result << Logger::endl;

	}*/


	return result;
}

glm::dvec3 OctreeRayTracing::findPseudoCenter(const glm::dvec3& seed1, const glm::dvec3& seed2, const glm::dvec3& normal1, const glm::dvec3& normal2)
{
	glm::dvec3 result = (seed1 + seed2) / (double)2;
	double geom = glm::length(seed1 - seed2) / (2 * tan(0.5*acos(glm::dot(normal1, normal2))));
	glm::dvec3 dir = normal1 + normal2;
	dir /= glm::length(dir);
	result += geom * dir;
	return result;
}

int OctreeRayTracing::countPointsNearPlane(const std::vector<glm::dvec3>& points, const std::vector<double>& plane, const double& threshold)
{
	int result(0);
	double distance;
	for (int i = 0; i < (int)points.size(); i++)
	{
		distance = glm::length(points[i] - MeasureClass::projectPointToPlane(points[i], plane));
		if (distance < threshold)
			result++;
	}
	return result;
}

std::vector<glm::dvec3> OctreeRayTracing::listPointsNearPlane(const std::vector<glm::dvec3>& points, const std::vector<double>& plane, const double& threshold)
{
	std::vector<glm::dvec3> result;
	double distance;
	for (int i = 0; i < (int)points.size(); i++)
	{
		distance = glm::length(points[i] - MeasureClass::projectPointToPlane(points[i], plane));
		if (distance < threshold)
			result.push_back(points[i]);
	}
	return result;
}

/*
bool OctreeRayTracing::isPointClipped(const glm::dvec3& point, const std::unordered_map<ClippingMode, std::list<glm::dmat4>>& clippingBoxes)
{
	bool insideResult(false);
	ClippingMode inside = ClippingMode::showInterior;
	ClippingMode outside = ClippingMode::showExterior;
	std::list<glm::dmat4> insideBoxes, outsideBoxes;
	auto it = clippingBoxes.find(inside);
	if (it != clippingBoxes.end())
	{
		insideBoxes = it->second;
	}
	it = clippingBoxes.find(outside);
	if (it != clippingBoxes.end())
	{
		outsideBoxes = it->second;
	}
	for (auto itr=insideBoxes.begin();itr!=insideBoxes.end();itr++)
	{
		if (isPointInsideBox(point, *itr))
		{
			insideResult=true;
			break;
		}
	}
	if (insideResult || (insideBoxes.size() == 0))
	{
		for (auto itr = outsideBoxes.begin(); itr != outsideBoxes.end(); itr++)
		{
			if (isPointInsideBox(point, *itr))
				return false;
		}
		return true;
	}
	return false;
}
*/

/*
bool OctreeRayTracing::isPointInsideBox(const glm::dvec3& point, const glm::dmat4& box)
{
	glm::dvec4 tempPoint(point[0], point[1], point[2], 1.0);
	tempPoint = box * tempPoint;
	return ((tempPoint[0] > -1) && (tempPoint[0] < 1) && (tempPoint[1] > -1) && (tempPoint[1] < 1) && (tempPoint[2] > -1) && (tempPoint[2] < 1));
}
*/

/*
bool OctreeRayTracing::isCellClipped(const TreeCell& cell, const std::unordered_map<ClippingMode, std::list<glm::dmat4>>& clippingBoxes)
{
	bool insideResult(false);
	ClippingMode inside = ClippingMode::showInterior;
	
	std::list<glm::dmat4> insideBoxes;
	auto it = clippingBoxes.find(inside);
	if (it != clippingBoxes.end())
	{
		insideBoxes = it->second;
	}
	
	for (auto itr = insideBoxes.begin(); itr != insideBoxes.end(); itr++)
	{
		if (isCellInsideBox(cell, *itr))
		{
			insideResult = true;
			break;
		}
	}
	if (insideBoxes.size() == 0)
	{
		insideResult = true;
	}
	return insideResult;
}
*/

/*
bool OctreeRayTracing::isCellInsideBox(const TreeCell& cell, const glm::dmat4& box)
{
	double xMin(DBL_MAX), xMax(-DBL_MAX), yMin(DBL_MAX), yMax(-DBL_MAX), zMin(DBL_MAX), zMax(-DBL_MAX);
	for (int x = 0; x < 2; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			for (int z = 0; z < 2; z++)
			{
			glm::dvec4 corner4;
				corner4[0] = cell.m_position[0] + x * cell.m_size;
				corner4[1] = cell.m_position[1] + y * cell.m_size;
				corner4[2] = cell.m_position[2] + z * cell.m_size;
				corner4 = box * corner4;

				if (xMin > corner4[0])
					xMin = corner4[0];
				if (xMax < corner4[0])
					xMax = corner4[0];
				if (yMin > corner4[1])
					yMin = corner4[1];
				if (yMax < corner4[1])
					yMax = corner4[1];
				if (zMin > corner4[2])
					zMin = corner4[2];
				if (zMax < corner4[2])
					zMax = corner4[2];
			}
		}
	}
	if (xMax < -1)
		return false;
	if (xMin > 1)
		return false;
	if (yMax < -1)
		return false;
	if (yMin > 1)
		return false;
	if (zMax < -1)
		return false;
	if (zMin > 1)
		return false;
	return true;
}
*/


bool OctreeRayTracing::isPointCloseToPreviousCylinders(const glm::dvec3& point, const std::vector<glm::dvec3>& cylinderCenters, const std::vector<glm::dvec3>& cylinderDirections, const std::vector<double>& cylinderRadii, const double& threshold)
{
    for (int i = 0; i < (int)cylinderCenters.size(); i++)
    {
        if (computePointToCylinderDistance(point, cylinderCenters[i], computeProjectionMatrix(cylinderDirections[i]), cylinderRadii[i]) < threshold)
            return true;
    }
    return false;
}

BeamDirectionRange::BeamDirectionRange(const double pointWidth)
{
	m_pointWidth = pointWidth;
}

BeamDirectionRange::~BeamDirectionRange()
{
}

void BeamDirectionRange::addPoint(const double& pointPosition)
{
	double pMin = pointPosition - m_pointWidth;
	double pMax = pointPosition + m_pointWidth;

	//if m_intervals is empty, add one interval centered around that point
	if ((int)m_intervals.size() == 0)
	{
		std::vector<double> temp;
		temp.push_back(pMin);
		temp.push_back(pMax);
		m_intervals.push_back(temp);
	}
	//else check if the point touches an existing interval
	for (int i = 0; i < (int)m_intervals.size(); i++)
	{
		if ((pMin < m_intervals[i][1])&&(pMin>m_intervals[i][0]))
		{
			//the point hits the ith interval from above
			if (i + 1 < (int)m_intervals.size())
			{
				if (pMax > m_intervals[i + 1][0])
				{
					//in this case, the point links two intervals together
					m_intervals[i][1] = std::max(0.1*pMax+0.9*m_intervals[i + 1][1], m_intervals[i + 1][1]);
					m_intervals.erase(m_intervals.begin()+i+1);
					return;
				}				
			}
			//in this case we may extend the ith interval if need be
			m_intervals[i][1] = std::max(0.1*pMax+ 0.9*m_intervals[i][1], m_intervals[i][1]);
			return;
		}
		if ((pMax > m_intervals[i][0])&&(pMax<m_intervals[i][1]))
		{
			//the point hits the ith interval from below, and didnt hit the previous one from above
			m_intervals[i][0] = std::min(0.1*pMin+0.9*m_intervals[i][0],m_intervals[i][0]);
			return;
		}
		
		if (pMax < m_intervals[i][0])
		{
			//the interval went beyond the point without hitting, so we insert a new one
			std::vector<double> temp;
			temp.push_back(pMin);
			temp.push_back(pMax);
			m_intervals.insert(m_intervals.begin() + i, temp);
			return;
		}
	}
	//the remaining case is the point is after every interval, so we add one
	std::vector<double> temp;
	temp.push_back(pMin);
	temp.push_back(pMax);
	m_intervals.push_back(temp);
	return;
}

std::vector<double> BeamDirectionRange::getCentralInterval()
{
	std::vector<double> result;
	//return the interval containing 0, or the interval closest to 0 (should not happen)
	for (int i = 0; i < (int)m_intervals.size(); i++)
	{
		if (m_intervals[i][1] < 0)
			continue;
		else if (m_intervals[i][0] < 0)
		{
			result.push_back(m_intervals[i][0] + m_pointWidth);
			result.push_back(m_intervals[i][1] - m_pointWidth);
			return result;
		}
		else if (i > 0)
		{
			if (-m_intervals[i - 1][1] < m_intervals[i][0])
			{
				result.push_back(m_intervals[i - 1][0] + m_pointWidth);
				result.push_back(m_intervals[i - 1][1] - m_pointWidth);
				return result;
			}

			else
			{
				result.push_back(m_intervals[i][0] + m_pointWidth);
				result.push_back(m_intervals[i][1] - m_pointWidth);
				return result;
			}
		}
	}
	if (m_intervals.size() > 0)
		return m_intervals[0];
	return std::vector<double>(2,0);
}

VoxelGrid::VoxelGrid(const double voxelSize, const ClippingAssembly clippingAssembly, const double& tMax)
{
	m_voxelSize = voxelSize;
	m_clippingAssembly = clippingAssembly;
	m_numberOfScans = 0;

	//glm::dvec4 temp(0.0,0.0, 0.0, 1.0);
	//glm::dmat4 mat = glm::inverse( m_clippingAssembly.clippingUnion[0]->matRT_inv);
	//glm::dvec4 tempCenter = mat * temp;
	//glm::dvec3 realCenter= glm::dvec3(tempCenter[0], tempCenter[1], tempCenter[2]);
	//glm::dvec3 center = glm::dvec3(temp[0], temp[1], temp[2]);
	//glm::dvec3 center = glm::dvec3(0.0, 0.0, 0.0);
	//glm::dvec3 scale = m_clippingAssembly.clippingUnion[0]->scale;
	/*m_xMin = center[0] - scale[0];
	m_xMax = center[0] + scale[0];
	m_yMin = center[1] - scale[1];
	m_yMax = center[1] + scale[1];
	m_zMin = center[2] - scale[2];
	m_zMax = center[2] + scale[2];*/
	m_xMin = -tMax;
	m_xMax = tMax;
	m_yMin = -tMax;
	m_yMax = tMax;
	m_zMin = -tMax;
	m_zMax = tMax;
	
	m_sizeX =1+ (int)((m_xMax - m_xMin) / m_voxelSize);
	m_sizeY =1+ (int)((m_yMax - m_yMin) / m_voxelSize);
	m_sizeZ =1+ (int)((m_zMax - m_zMin) / m_voxelSize);
	std::vector<int> temp1(m_sizeZ);
	std::vector<std::vector<int>> temp2(m_sizeY, temp1);
	m_grid = std::vector<std::vector<std::vector<int>>>(m_sizeX, temp2);
}

void VoxelGrid::voxelCoordinatesOfPoint(const glm::dvec3& point, int& x, int& y, int& z) const
{
	x = (int)((point[0] - m_xMin) / m_voxelSize);
	y = (int)((point[1] - m_yMin) / m_voxelSize);
	z = (int)((point[2] - m_zMin) / m_voxelSize);
	return;
}

glm::dvec3 VoxelGrid::centerOfVoxel(const int& x, const int& y, const int& z) const
{
	return glm::dvec3(m_xMin + (x + 0.5) * m_voxelSize, m_yMin + (y + 0.5) * m_voxelSize, m_zMin + (z + 0.5) * m_voxelSize);
}

bool VoxelGrid::isVoxelOccupied(const int& i, const int& j, const int& k, const int& scanNumber) const
{
	//if scanNumber is present in cell i,j,k, return true, otherwise return false
	if (((m_grid[i][j][k] >> scanNumber) % 2) == 1)
		return true;
	return false;
}

bool VoxelGrid::areVoxelCoordinatesValid(const int& i, const int& j, const int& k) const
{
	return ((i >= 0) && (j >= 0) && (k >= 0) && (i < m_sizeX) && (j < m_sizeY) && (k < m_sizeZ));
}

GeometricBox VoxelGrid::getBoxFromCoordinates(const int& i, const int& j, const int& k) const
{
	std::vector<glm::dvec3> boxCorners;
	boxCorners.push_back(glm::dvec3(m_xMin + i * m_voxelSize, m_yMin + j * m_voxelSize, m_zMin + k * m_voxelSize));
	boxCorners.push_back(glm::dvec3(m_xMin + (i + 1) * m_voxelSize, m_yMin + j * m_voxelSize, m_zMin + k * m_voxelSize));
	boxCorners.push_back(glm::dvec3(m_xMin + i * m_voxelSize, m_yMin + (j + 1) * m_voxelSize, m_zMin + k * m_voxelSize));
	boxCorners.push_back(glm::dvec3(m_xMin + i * m_voxelSize, m_yMin + j * m_voxelSize, m_zMin + (k + 1) * m_voxelSize));
	boxCorners.push_back(glm::dvec3(m_xMin + (i + 1) * m_voxelSize, m_yMin + (j + 1) * m_voxelSize, m_zMin + k * m_voxelSize));
	boxCorners.push_back(glm::dvec3(m_xMin + (i + 1) * m_voxelSize, m_yMin + j * m_voxelSize, m_zMin + (k + 1) * m_voxelSize));
	boxCorners.push_back(glm::dvec3(m_xMin + i * m_voxelSize, m_yMin + (j + 1) * m_voxelSize, m_zMin + (k + 1) * m_voxelSize));
	boxCorners.push_back(glm::dvec3(m_xMin + (i + 1) * m_voxelSize, m_yMin + (j + 1) * m_voxelSize, m_zMin + (k + 1) * m_voxelSize));
	GeometricBox gridBox(boxCorners);
	return gridBox;
}

VoxelGrid::~VoxelGrid()
{}

GeometricBox::GeometricBox(const std::vector<glm::dvec3> corners)
{
	m_corners=corners;
}

GeometricBox::GeometricBox(const ClippingAssembly& clippingAssembly)
{
	if (clippingAssembly.clippingUnion.size() > 0)
	{
		std::vector<glm::dvec3> corners(0);
		glm::dmat4 transfo = glm::inverse(clippingAssembly.clippingUnion[0]->matRT_inv);
		glm::dvec4 corner4 = transfo * glm::dvec4(-1.0, -1.0, -1.0, 1.0);
		glm::dvec3 corner(corner4[0], corner4[1], corner4[2]);
		corners.push_back(corner);
		corner4 = transfo * glm::dvec4(1.0, -1.0, -1.0, 1.0);
		corner=glm::dvec3(corner4[0], corner4[1], corner4[2]);
		corners.push_back(corner);
		corner4 = transfo * glm::dvec4(-1.0, 1.0, -1.0, 1.0);
		corner = glm::dvec3(corner4[0], corner4[1], corner4[2]);
		corners.push_back(corner);
		corner4 = transfo * glm::dvec4(-1.0, -1.0, 1.0, 1.0);
		corner = glm::dvec3(corner4[0], corner4[1], corner4[2]);
		corners.push_back(corner);
		corner4 = transfo * glm::dvec4(1.0, 1.0, -1.0, 1.0);
		corner = glm::dvec3(corner4[0], corner4[1], corner4[2]);
		corners.push_back(corner);
		corner4 = transfo * glm::dvec4(1.0, -1.0, 1.0, 1.0);
		corner = glm::dvec3(corner4[0], corner4[1], corner4[2]);
		corners.push_back(corner);
		corner4 = transfo * glm::dvec4(1.0, 1.0, -1.0, 1.0);
		corner = glm::dvec3(corner4[0], corner4[1], corner4[2]);
		corners.push_back(corner);
		corner4 = transfo * glm::dvec4(1.0, 1.0, 1.0, 1.0);
		corner = glm::dvec3(corner4[0], corner4[1], corner4[2]);
		corners.push_back(corner);
		m_corners = corners;
		setRadius(glm::length(m_corners[7]-m_corners[0]));
	}
	else {
		m_corners = std::vector<glm::dvec3>(8, glm::dvec3(0.0, 0.0, 0.0));
	}
}

GeometricBox::~GeometricBox()
{};

glm::dvec3 GeometricBox::getDirX() const
{
	glm::dvec3 result = m_corners[1] - m_corners[0];
	result /= glm::length(result);
	return result;
}

glm::dvec3 GeometricBox::getDirY() const
{
	glm::dvec3 result = m_corners[2] - m_corners[0];
	result /= glm::length(result);
	return result;
}

glm::dvec3 GeometricBox::getDirZ() const
{
	glm::dvec3 result = m_corners[3] - m_corners[0];
	result /= glm::length(result);
	return result;
}

void GeometricBox::setRadius(const double& radius)
{
	glm::dvec3 center = getCenter();
	for (int i = 0; i < 8; i++)
	{
		m_corners[i] = center + radius * (m_corners[i] - center) / glm::length(m_corners[i] - center);
	}

}

bool GeometricBox::isInside(const glm::dvec3& point) const
{
	//test each face

	if (!isInHalfSpace(m_corners[0], m_corners[1], m_corners[2], m_corners[7], point))
		return false;
	if (!isInHalfSpace(m_corners[0], m_corners[1], m_corners[3], m_corners[7], point))
		return false;
	if (!isInHalfSpace(m_corners[0], m_corners[2], m_corners[3], m_corners[7], point))
		return false;
	if (!isInHalfSpace(m_corners[7], m_corners[4], m_corners[5], m_corners[0], point))
		return false;
	if (!isInHalfSpace(m_corners[7], m_corners[4], m_corners[6], m_corners[0], point))
		return false;
	if (!isInHalfSpace(m_corners[7], m_corners[5], m_corners[6], m_corners[0], point))
		return false;
	return true;
}

bool GeometricBox::isInHalfSpace(const glm::dvec3& planePoint1, const glm::dvec3& planePoint2, const glm::dvec3& planePoint3, const glm::dvec3& halfPlanePoint, const glm::dvec3& testPoint) const
{
	glm::dvec3 normal = glm::cross(planePoint1 - planePoint2, planePoint1 - planePoint3);
	if (glm::dot(halfPlanePoint - planePoint1, normal) < 0)
		normal = -normal;
	return (glm::dot(testPoint - planePoint1, normal) > 0);
}

glm::dvec3 GeometricBox::getCenter() const
{
	return 0.125 * (m_corners[0] + m_corners[1] + m_corners[2] + m_corners[3] + m_corners[4] + m_corners[5] + m_corners[6] + m_corners[7]);
}

double GeometricBox::getRadius() const
{
	return (0.5*glm::length(m_corners[0] - m_corners[7]));
}

ClusterInfo::ClusterInfo(const int& sizeX, const int& sizeY, const int& sizeZ, const int& label)
{
	m_volume = 0;
	m_xMax = 0;
	m_xMin = sizeX;
	m_yMax = 0;
	m_yMin = sizeY;
	m_zMax = 0;
	m_zMin = sizeZ;
	m_label = label;
	m_emptyNeighbors = 0;
	m_staticNeighbors = 0;
}

ClusterInfo::~ClusterInfo()
{};

bool ClusterInfo::isTrueDynamic() const
{
	bool result(true);
	double staticPerVolume((double)m_staticNeighbors / (double)m_volume);
	int ratioStaticPerEmpty = std::max(5, (int)(staticPerVolume * 15));
	if ((ratioStaticPerEmpty * m_staticNeighbors) > m_emptyNeighbors)
		result = false;
	return result;
}

void ClusterInfo::updateWithPoint(const int& x, const int& y, const int& z)
{
	m_volume++;
	if (x < m_xMin)
		m_xMin = x;

	if (x >= m_xMax)
		m_xMax = x+1;

	if (y < m_yMin)
		m_yMin = y;

	if (y >= m_yMax)
		m_yMax = y+1;

	if (z < m_zMin)
		m_zMin = z;

	if (z >= m_zMax)
		m_zMax = z+1;
}

glm::dvec3 OctreeRayTracing::getTransformationCoord(const glm::dvec3& globalCoord, const ClippingAssembly& clippingAssembly)
{

	glm::dvec4 temp(globalCoord[0], globalCoord[1], globalCoord[2], 1.0);
	glm::dmat4 mat = clippingAssembly.clippingUnion[0]->matRT_inv;
	glm::dvec4 temp1 = mat * temp;
	return glm::dvec3(temp1[0], temp1[1], temp1[2]);
}

bool OctreeRayTracing::doBoxIntersectFromViewPoint(const glm::dvec3& origin, const GeometricBox& box1, const GeometricBox& box2, double& distanceToIntersection)
{
	//approx boxes with spheres
	glm::dvec3 center1(box1.getCenter()), center2(box2.getCenter());
	double d1(glm::length(origin - center1)), d2(glm::length(origin - center2));
	double d = std::min(d1, d2);
	//check if origin is inside a sphere

	if ((d1 < (box1.getRadius())) || (d2 < (box2.getRadius())))
	{
		distanceToIntersection = 0;
		return true;
	}
	double updatedRadius1(box1.getRadius() *d/ d1), updatedRadius2(box2.getRadius() *d/ d2);
	glm::dvec3 updatedCenter1, updatedCenter2;
	updatedCenter1 = origin + (center1 - origin)*d/d1;
	updatedCenter2 = origin + (center2 - origin)*d/d2;
	bool result = (glm::length(updatedCenter1 - updatedCenter2)) < (updatedRadius1 + updatedRadius2);
	//if true, we compute the distance from origin to intersection
	if (result)
	{
		/*double A = glm::length(updatedCenter1 - updatedCenter2);
		double offsetSquared = pow(updatedRadius1, 2) - (pow(pow(A, 2) + pow(updatedRadius1, 2) - pow(updatedRadius2, 2), 2) / (4 * pow(A, 2)));*/
		
		double offsetSquared = pow(std::max(updatedRadius1, updatedRadius2), 2) - pow(0.5 * glm::length(updatedCenter1 - updatedCenter2), 2);
		if (offsetSquared < 0)
			distanceToIntersection = 0;
		else
			distanceToIntersection = glm::length(0.5 * (updatedCenter1 + updatedCenter2) - origin) - sqrt(offsetSquared);
	}

	return result;
}

int OctreeRayTracing::relateBoxToOtherBox(const GeometricBox& box1, const GeometricBox& box2)
{
	// 0 : box1 is strictly inside box2
	// 1 : box2 is strictly inside box1
	// 2 : boxes intersect
	// 3 : boxes don't intersect

	GeometricBox box = box2;
	bool corner1Inside2(false), corner2Inside1(false);
	bool box2InsideBox1(true);
	bool box1InsideBox2(true);
	for (int i = 0; i < 8; i++)
	{
		if (!box.isInside(box1.m_corners[i]))
		{
			box1InsideBox2 = false;
		}
		else
			corner1Inside2 = true;
	}
	if (box1InsideBox2)
		return 0;
	box = box1;
	for (int i = 0; i < 8; i++)
	{
		if (!box.isInside(box2.m_corners[i]))
		{
			box2InsideBox1 = false;
		}
		else
			corner2Inside1 = true;
	}
	if (box2InsideBox1)
		return 1;
	//case 2 : boxes intersect

	if (corner1Inside2 || corner2Inside1)
		return 2;
	//case 2 : maybe the boxes intersect along an edge, temp test comparing distance between centers and diagonals
	glm::dvec3 center1 = (box1.m_corners[0] + box1.m_corners[1] + box1.m_corners[2] + box1.m_corners[3] + box1.m_corners[4] + box1.m_corners[5] + box1.m_corners[6] + box1.m_corners[7]) * 0.125;
	glm::dvec3 center2 = (box2.m_corners[0] + box2.m_corners[1] + box2.m_corners[2] + box2.m_corners[3] + box2.m_corners[4] + box2.m_corners[5] + box2.m_corners[6] + box2.m_corners[7]) * 0.125;
	if (glm::length(center1 - center2) < 0.5*(glm::length(box1.m_corners[0] - box1.m_corners[7])+glm::length(box2.m_corners[0]-box2.m_corners[7])))
		return 2;

	return 3;
}

OctreeVoxelNode::OctreeVoxelNode(const int& dataSize) {
	m_children.resize(8); // initialize array of 8 children
	for (int i = 0; i < 8; i++) {
		m_children[i] = nullptr;
	}
	m_data = std::vector<bool>(0,false); // initialize boolean vector
	m_dataSize = dataSize;
}

OctreeVoxelNode::~OctreeVoxelNode()
{};

void OctreeVoxelNode::insertValue(int depth, int x, int y, int z, int valuePosition, int maxDepth) {

	if (depth == maxDepth) { // if maximum depth is reached
		if ((int)m_data.size() == 0)
		{
			m_data = std::vector<bool>(m_dataSize, false);
		}
		m_data[valuePosition] = true; // insert value into boolean vector
		return;
	}
	int childIndex = getChildIndex(depth, x, y, z, maxDepth);
	if (childIndex > 7)
		return;
	if (!m_children[childIndex]) { // if child node does not exist
		m_children[childIndex] = new OctreeVoxelNode(m_dataSize); // create new child node
	}
	m_children[childIndex]->insertValue(depth + 1, x, y, z, valuePosition, maxDepth); // recursively insert value into child node
}

bool OctreeVoxelNode::readValue(int depth, int x, int y, int z, int valuePosition, int maxDepth)
{
	if (depth == maxDepth) {
		return m_data[valuePosition];
	}
	int childIndex = getChildIndex(depth, x, y, z, maxDepth);
	if (!m_children[childIndex]) { // if child node does not exist
		return false;
	}
	return m_children[childIndex]->readValue(depth + 1, x, y, z, valuePosition, maxDepth);
}

int OctreeVoxelNode::getChildIndex(int depth, int x, int y, int z, int maxDepth)
{
	int dx, dy, dz;
	
	//depth d : on regarde le dième bit de poids fort de x
	dx = (x & (1 << (maxDepth - depth))) >> (maxDepth - depth);
	dy = (y & (1 << (maxDepth - depth))) >> (maxDepth - depth);
	dz = (z & (1 << (maxDepth - depth))) >> (maxDepth - depth);
	return dx + (dy << 1) + (dz << 2);
}

OctreeVoxelGrid::OctreeVoxelGrid(const double voxelSize, const ClippingAssembly clippingAssembly, const int numberOfScans)
{
	m_clippingAssembly = clippingAssembly;
	m_numberOfScans = numberOfScans;
	m_octree = new OctreeVoxelNode(numberOfScans);
	glm::vec4 scale = m_clippingAssembly.clippingUnion[0]->params;
	m_maxSize=2*std::max(std::max(scale[0], scale[1]), scale[2]);
	m_voxelSize = m_maxSize;
	m_maxDepth = 0;
	while (m_voxelSize > voxelSize)
	{
		m_maxDepth++;
		m_voxelSize /= 2;
	}
}

OctreeVoxelGrid::~OctreeVoxelGrid()
{}

bool OctreeVoxelGrid::areVoxelCoordinatesValid(const int& x, const int& y, const int& z) const
{
	return ((x >= 0) && (y >= 0) && (z >= 0) && (x < std::pow(2,m_maxDepth)) && (y < std::pow(2, m_maxDepth)) && (z < std::pow(2, m_maxDepth)));
}

bool OctreeVoxelGrid::isVoxelOccupied(const int& x, const int& y, const int& z, const int& scanNumber) const
{
	return m_octree->readValue(0, x, y, z, scanNumber, m_maxDepth);
}

bool OctreeVoxelGrid::isEmpty(const int& x, const int& y, const int& z) const
{
	for (int i = 0; i < m_numberOfScans; i++)
	{
		if (isVoxelOccupied(x, y, z, i))
			return false;
	}
	return true;
}

void OctreeVoxelGrid::voxelCoordinatesOfPoint(const glm::dvec3& point, int& x, int& y, int& z) const
{
	x = (int)((point[0] + m_maxSize / 2) / m_voxelSize);
	y = (int)((point[1] + m_maxSize / 2) / m_voxelSize);
	z = (int)((point[2] + m_maxSize / 2) / m_voxelSize);
	return;
}

glm::dvec3 OctreeVoxelGrid::centerOfVoxel(const int& x, const int& y, const int& z) const
{
	return glm::dvec3(-m_maxSize / 2 + (x + 0.5) * m_voxelSize, -m_maxSize / 2 + (y + 0.5) * m_voxelSize, -m_maxSize / 2 + (z + 0.5) * m_voxelSize);
}

GeometricBox OctreeVoxelGrid::getBoxFromCoordinates(const int& x, const int& y, const int& z) const
{
	std::vector<glm::dvec3> boxCorners;
	boxCorners.push_back(glm::dvec3(-m_maxSize / 2 + x * m_voxelSize, -m_maxSize / 2 + y * m_voxelSize, -m_maxSize / 2 + z * m_voxelSize));
	boxCorners.push_back(glm::dvec3(-m_maxSize / 2 + (x + 1) * m_voxelSize, -m_maxSize / 2 + y * m_voxelSize, -m_maxSize / 2 + z * m_voxelSize));
	boxCorners.push_back(glm::dvec3(-m_maxSize / 2 + x * m_voxelSize, -m_maxSize / 2 + (y + 1) * m_voxelSize, -m_maxSize / 2 + z * m_voxelSize));
	boxCorners.push_back(glm::dvec3(-m_maxSize / 2 + x * m_voxelSize, -m_maxSize / 2 + y * m_voxelSize, -m_maxSize / 2 + (z + 1) * m_voxelSize));
	boxCorners.push_back(glm::dvec3(-m_maxSize / 2 + (x + 1) * m_voxelSize, -m_maxSize / 2 + (y + 1) * m_voxelSize, -m_maxSize / 2 + z * m_voxelSize));
	boxCorners.push_back(glm::dvec3(-m_maxSize / 2 + (x + 1) * m_voxelSize, -m_maxSize / 2 + y * m_voxelSize, -m_maxSize / 2 + (z + 1) * m_voxelSize));
	boxCorners.push_back(glm::dvec3(-m_maxSize / 2 + x * m_voxelSize, -m_maxSize / 2 + (y + 1) * m_voxelSize, -m_maxSize / 2 + (z + 1) * m_voxelSize));
	boxCorners.push_back(glm::dvec3(-m_maxSize / 2 + (x + 1) * m_voxelSize, -m_maxSize / 2 + (y + 1) * m_voxelSize, -m_maxSize / 2 + (z + 1) * m_voxelSize));
	GeometricBox gridBox(boxCorners);
	return gridBox;
}

void OctreeVoxelGrid::addPoint(const glm::dvec3& point, const int& scanNumber)
{
	int x, y, z;
	voxelCoordinatesOfPoint(point, x, y, z);
	if (!areVoxelCoordinatesValid(x,y,z))
	{
		Logger::log(LoggerMode::rayTracingLog) << "error : tried to add value outside of range" << Logger::endl;
		return;
	}
	m_octree->insertValue(0, x, y, z, scanNumber, m_maxDepth);
	return;
}

void OctreeVoxelGrid::addValue(const int& x, const int& y, const int& z, const int& scanNumber)
{
	if (!areVoxelCoordinatesValid(x, y, z))
	{
		Logger::log(LoggerMode::rayTracingLog) << "error : tried to add value outside of range" << Logger::endl;
		return;
	}
	m_octree->insertValue(0, x, y, z, scanNumber, m_maxDepth);
	return;
}

RectangularPlane::RectangularPlane(const glm::dvec3& normal, const std::vector<glm::dvec3>& corners, const glm::dvec3& centerPoint, const PlaneType& planetype)
{
	m_planeType = planetype;
	m_normal = normal;
	m_centerPoint = centerPoint;
	m_corners = std::vector<glm::dvec3>(0);
	std::vector<double> tempPlane;
	tempPlane.push_back(m_normal[0]);
	tempPlane.push_back(m_normal[1]);
	tempPlane.push_back(m_normal[2]);
	tempPlane.push_back(-glm::dot(m_normal, m_centerPoint));
	if ((int)corners.size() != 4)
		return;
	for (int i = 0; i < 4; i++)
	{
		m_corners.push_back(MeasureClass::projectPointToPlane(corners[i], tempPlane));
	}
	return;
}

void RectangularPlane::updateCorners(const std::vector<glm::dvec3>& corners)
{
	std::vector<double> tempPlane;
	tempPlane.push_back(m_normal[0]);
	tempPlane.push_back(m_normal[1]);
	tempPlane.push_back(m_normal[2]);
	tempPlane.push_back(-glm::dot(m_normal, m_centerPoint));
	if ((int)corners.size() != 4)
		return;
	m_corners = std::vector<glm::dvec3>(0);

	for (int i = 0; i < 4; i++)
	{
		m_corners.push_back(MeasureClass::projectPointToPlane(corners[i], tempPlane));
	}
	glm::dvec3 newCenter = 0.25 * (m_corners[0] + m_corners[1] + m_corners[2] + m_corners[3]);
	newCenter = MeasureClass::projectPointToPlane(newCenter, tempPlane);
	m_centerPoint = newCenter;
	return;
}

void RectangularPlane::updateNormal(const glm::dvec3& normal)
{
	m_normal = normal;
	std::vector<glm::dvec3> previousCorners = m_corners;
	std::vector<double> tempPlane;
	tempPlane.push_back(m_normal[0]);
	tempPlane.push_back(m_normal[1]);
	tempPlane.push_back(m_normal[2]);
	tempPlane.push_back(-glm::dot(m_normal, m_centerPoint));
	m_corners = std::vector<glm::dvec3>(0);

	for (int i = 0; i < 4; i++)
	{
		m_corners.push_back(MeasureClass::projectPointToPlane(previousCorners[i], tempPlane));
	}

	return;
}

void RectangularPlane::updateCenter(const glm::dvec3& centerPoint)
{
	m_centerPoint = centerPoint;
	std::vector<glm::dvec3> previousCorners = m_corners;
	std::vector<double> tempPlane;
	tempPlane.push_back(m_normal[0]);
	tempPlane.push_back(m_normal[1]);
	tempPlane.push_back(m_normal[2]);
	tempPlane.push_back(-glm::dot(m_normal, m_centerPoint));
	m_corners = std::vector<glm::dvec3>(0);

	for (int i = 0; i < 4; i++)
	{
		m_corners.push_back(MeasureClass::projectPointToPlane(previousCorners[i], tempPlane));
	}
	return;
}

TransformationModule RectangularPlane::createTransfo() const
{	
	glm::dvec3 boxDirectionX(m_corners[1] - m_corners[0]), boxDirectionY(m_corners[2] - m_corners[0]);
	boxDirectionX = boxDirectionX / glm::length(boxDirectionX);
	boxDirectionY = boxDirectionY / glm::length(boxDirectionY);
	//case where plane is horizontal
	if (m_planeType==PlaneType::horizontal)
	{
		//unused?
		double sign = 1.0;
		if (glm::dot(glm::cross(boxDirectionX, boxDirectionY), glm::dvec3(0.0, 0.0, 1.0)) < 0)
			sign = -1.0;
		TransformationModule mod;
		glm::dquat quat1 = glm::rotation(glm::dvec3(0.0, 1.0, 0.0), boxDirectionX);
		glm::dvec3 up = glm::normalize(glm::cross(boxDirectionX, boxDirectionY));
		glm::dvec3 newUp = quat1 * glm::dvec3(0.0, 0.0, 1.0);
		glm::dquat quat2 = glm::rotation(newUp, up);

		double sign1(1.0), sign2(1.0);
		if (boxDirectionX[0] < 0)
			sign1 = -1.0;
		if (boxDirectionY[1] < 0)
			sign2 = -1.0;
		double angle1 = acos(glm::dot(glm::dvec3(sign1, 0.0, 0.0), boxDirectionX));
		double angle2 = acos(glm::dot(glm::dvec3(0.0, sign2, 0.0), boxDirectionY));

		if (abs(angle1 - angle2) > 0.5)
		{
			angle2 = acos(glm::dot(glm::dvec3(0.0, -sign2, 0.0), boxDirectionY));
		}

		double angle = 0.5 * (angle1 + angle2);
		quat2 = glm::angleAxis(angle, glm::dvec3(0.0, 0.0, sign));
		mod.setRotation(quat2);
		mod.setPosition(m_centerPoint);
		glm::dvec3 scale = 0.5 * glm::dvec3(glm::length(m_corners[1] - m_corners[0]), glm::length(m_corners[2] - m_corners[0]), 0.001);
		mod.setScale(scale);
		return mod;
	}
	
	else if(m_planeType==PlaneType::vertical)
	{
		//case where plane is vertical
		bool first2cornersVertical(false);
		
		if (abs(glm::dot(boxDirectionX, glm::dvec3(0.0, 0.0, 1.0))) > abs(glm::dot(boxDirectionY, glm::dvec3(0.0, 0.0, 1.0))))
		{
			boxDirectionX = boxDirectionY;
			first2cornersVertical = true;
		}
		double verticalScale;
		if (first2cornersVertical)
			verticalScale = std::max(1.0, glm::length(m_corners[1] - m_corners[0]));
		else
			verticalScale = std::max(1.0, glm::length(m_corners[2] - m_corners[0]));
		if (boxDirectionX[0] < 0)
			boxDirectionX = -boxDirectionX;
		double angle = acos(glm::dot(boxDirectionX, glm::dvec3(1.0, 0.0, 0.0)));
		if (boxDirectionX[1] < 0)
			angle = -angle;
		TransformationModule mod;
		glm::dquat quat = glm::angleAxis(angle, glm::dvec3(0.0, 0.0, 1.0));
		mod.setRotation(quat);
		mod.setPosition(m_centerPoint);
		glm::dvec3 scale;
		if (first2cornersVertical)
		{
			scale = 0.5 * glm::dvec3(glm::length(m_corners[2] - m_corners[0]), 0.001, verticalScale);
		}
		else
			scale = 0.5 * glm::dvec3(glm::length(m_corners[1] - m_corners[0]), 0.001, verticalScale);
		mod.setScale(scale);
		return mod;
	}
	else
	{
		//plane is tilted
		
		//normal -> take vector v, rotate it around cross(normal,v) until it matches normal
		//what makes a good vector v ? 
		glm::dvec3 normal = m_normal;
		if (normal[2] < 0)
			normal = -normal;
		glm::dvec3 v = glm::dvec3(0.0, 0.0, 1.0);
		glm::dvec3 axis = glm::cross(normal, v);
		axis = axis / glm::length(axis);
		double angle = acos(glm::dot(v, normal));
		if (glm::dot(v, glm::cross(axis,normal)) > 0)
			angle = -angle;
		glm::dquat quat = glm::angleAxis(angle, axis);
		//as is, the normal is oriented correctly, but we probably should rotate around the normal to make the corners match

		glm::dvec3 w = m_corners[1] - m_corners[0];
		w = w / glm::length(w);
		glm::dvec3 newX = quat * glm::dvec3(1.0, 0.0, 0.0);
		angle = acos(glm::dot(newX, w));
		if (glm::dot(newX, glm::cross(normal,w)) > 0)
			angle = -angle;
		quat = glm::angleAxis(angle, normal)*quat;
		TransformationModule mod;
		mod.setRotation(quat);
		mod.setPosition(m_centerPoint);
		glm::dvec3 scale;
		scale = 0.5*glm::dvec3(glm::length(m_corners[1] - m_corners[0]), glm::length(m_corners[2] - m_corners[0]), 0.001);
		mod.setScale(scale);
		return mod;

	}
}

AbstractPlane::AbstractPlane(const glm::dvec3& normal, const glm::dvec3& point)
{
	m_normal = normal/glm::length(normal);
	m_point = point;
}

bool OctreeRayTracing::computePlaneIntersection(const std::vector<double>& plane1, const std::vector<double>& plane2, glm::dvec3& lineDirection, glm::dvec3& linePoint)
{
	glm::dvec3 normal1(glm::dvec3(plane1[0], plane1[1], plane1[2])), normal2(glm::dvec3(plane2[0], plane2[1], plane2[2]));
	if (abs(glm::dot(normal1, normal2)) > 0.995)
		return false;
	else
	{
		lineDirection = glm::cross(normal1, normal2);
		lineDirection /= glm::length(lineDirection);
		//get index of biggest coordinate
		int maxCoordLineIndex(0);
		if (abs(lineDirection[1]) > abs(lineDirection[0]))
		{
			if (abs(lineDirection[2]) > abs(lineDirection[1]))
				maxCoordLineIndex = 2;
			else
				maxCoordLineIndex = 1;
		}
		else if (abs(lineDirection[2]) > abs(lineDirection[0]))
			maxCoordLineIndex = 2;

		if (maxCoordLineIndex == 0)
		{
			double det = normal1[1] * normal2[2] - normal1[2] * normal2[1];
			if (abs(det) <= std::numeric_limits<double>::epsilon())
				return false;
			linePoint[0] = 0.0;
			linePoint[1] = -(plane1[3] * plane2[2] - plane1[2] * plane2[3]) / det;
			linePoint[2] = -(plane1[1] * plane2[3] - plane1[3] * plane2[1]) / det;
		}

		if (maxCoordLineIndex == 1)
		{
			double det = normal1[0] * normal2[2] - normal1[2] * normal2[0];
			if (abs(det) <= std::numeric_limits<double>::epsilon())
				return false;
			linePoint[0] = -(plane1[3] * plane2[2] - plane1[2] * plane2[3]) / det;
			linePoint[1] = 0.0;
			linePoint[2] = -(plane1[0] * plane2[3] - plane1[3] * plane2[0]) / det;
		}

		if (maxCoordLineIndex == 2)
		{
			double det = normal1[0] * normal2[1] - normal1[1] * normal2[0];
			if (abs(det) <= std::numeric_limits<double>::epsilon())
				return false;
			linePoint[0] = -(plane1[3] * plane2[1] - plane1[1] * plane2[3]) / det;
			linePoint[1] = -(plane1[0] * plane2[3] - plane1[3] * plane2[0]) / det;
			linePoint[2] = 0.0;
		}
		return true;
	}
}

void OctreeRayTracing::connectPlanes(RectangularPlane& rect1, RectangularPlane& rect2)
{
	if (abs(glm::dot(rect1.m_normal, rect2.m_normal)) > 0.995)
		return;
	
	glm::dvec3 lineDirection(1.0, 0.0, 0.0);
	glm::dvec3 pointOnLine(0.0, 0.0, 0.0);
	std::vector<double> plane1(4), plane2(4);
	for (int i = 0; i < 3; i++)
	{
		plane1[i] = rect1.m_normal[i];
		plane2[i] = rect2.m_normal[i];
	}
	plane1[3] = -glm::dot(rect1.m_normal, rect1.m_centerPoint);
	plane2[3] = -glm::dot(rect2.m_normal, rect2.m_centerPoint);
	computePlaneIntersection(plane1, plane2, lineDirection, pointOnLine);
	
	//we have the line, now we find for each plane the second closest corner to that line, and extend along the edge (first/second) point until second point lands on the line

	glm::dvec3 plane1dirX = rect1.m_corners[1] - rect1.m_corners[0];
	plane1dirX /= glm::length(plane1dirX);
	glm::dvec3 plane1dirY = rect1.m_corners[2] - rect1.m_corners[0];
	plane1dirY /= glm::length(plane1dirY);
	glm::dvec3 plane2dirX = rect2.m_corners[1] - rect2.m_corners[0];
	plane2dirX /= glm::length(plane2dirX);
	glm::dvec3 plane2dirY = rect2.m_corners[2] - rect2.m_corners[0];
	plane2dirY /= glm::length(plane2dirY);

	//compute extension towards line for each corner
	std::vector<double> extension1dirX(0);
	extension1dirX.push_back(computeExtensionTowardsLine(rect1.m_corners[0], plane1dirX, pointOnLine, lineDirection));
	extension1dirX.push_back(computeExtensionTowardsLine(rect1.m_corners[1], plane1dirX, pointOnLine, lineDirection));
	extension1dirX.push_back(computeExtensionTowardsLine(rect1.m_corners[2], plane1dirX, pointOnLine, lineDirection));
	extension1dirX.push_back(computeExtensionTowardsLine(rect1.m_corners[3], plane1dirX, pointOnLine, lineDirection));

	std::vector<double> extension1dirY(0);
	extension1dirY.push_back(computeExtensionTowardsLine(rect1.m_corners[0], plane1dirY, pointOnLine, lineDirection));
	extension1dirY.push_back(computeExtensionTowardsLine(rect1.m_corners[1], plane1dirY, pointOnLine, lineDirection));
	extension1dirY.push_back(computeExtensionTowardsLine(rect1.m_corners[2], plane1dirY, pointOnLine, lineDirection));
	extension1dirY.push_back(computeExtensionTowardsLine(rect1.m_corners[3], plane1dirY, pointOnLine, lineDirection));
	
	//same thing for the other plane
	std::vector<double> extension2dirX(0);
	extension2dirX.push_back(computeExtensionTowardsLine(rect2.m_corners[0], plane2dirX, pointOnLine, lineDirection));
	extension2dirX.push_back(computeExtensionTowardsLine(rect2.m_corners[1], plane2dirX, pointOnLine, lineDirection));
	extension2dirX.push_back(computeExtensionTowardsLine(rect2.m_corners[2], plane2dirX, pointOnLine, lineDirection));
	extension2dirX.push_back(computeExtensionTowardsLine(rect2.m_corners[3], plane2dirX, pointOnLine, lineDirection));

	std::vector<double> extension2dirY(0);
	extension2dirY.push_back(computeExtensionTowardsLine(rect2.m_corners[0], plane2dirY, pointOnLine, lineDirection));
	extension2dirY.push_back(computeExtensionTowardsLine(rect2.m_corners[1], plane2dirY, pointOnLine, lineDirection));
	extension2dirY.push_back(computeExtensionTowardsLine(rect2.m_corners[2], plane2dirY, pointOnLine, lineDirection));
	extension2dirY.push_back(computeExtensionTowardsLine(rect2.m_corners[3], plane2dirY, pointOnLine, lineDirection));

	//for each plane, we need to find the smallest second-smallest extension thing
	//first check the sign:
	int posSignX(0);
	if (extension1dirX[0] > 0)
		posSignX++;

	if (extension1dirX[1] > 0)
		posSignX++;

	if (extension1dirX[2] > 0)
		posSignX++;

	if (extension1dirX[3] > 0)
		posSignX++;

	int smallestX = 0;
	int secondSmallestX = 0;
	bool noExtX(false);
	double extAmountX(0);
	switch (posSignX)
	{
	case 0:
	{
		//find the 2 smallest
		
		for (int i = 1; i < 4; i++)
			if (extension1dirX[i] > extension1dirX[smallestX])
				smallestX = i;
		if (smallestX == 0)
			secondSmallestX++;
		for (int i = 1; i < 4; i++)
			if ((extension1dirX[i] > extension1dirX[secondSmallestX])&&(i!=smallestX))
				secondSmallestX = i;
		extAmountX = extension1dirX[secondSmallestX];
		break;
	}
	case 1:
	{
		//find the smallest negative
		if (extension1dirX[0] > 0)
			secondSmallestX = 1;
		for (int i = 0; i < 4; i++)
		{
			if (extension1dirX[i] > 0)
				smallestX = i;
			else if (extension1dirX[i] > extension1dirX[secondSmallestX])
				secondSmallestX = i;
		}
		extAmountX = extension1dirX[secondSmallestX];

		break;
	}
	case 2:
	{
		//no extension
		extAmountX = 0;
		noExtX = true;
	}
	case 3:
	{
		//find the smallest positive
		if (extension1dirX[0] < 0)
			secondSmallestX = 1;
		for (int i = 0; i < 4; i++)
		{
			if (extension1dirX[i] < 0)
				smallestX = i;
			else if (extension1dirX[i] < extension1dirX[secondSmallestX])
				secondSmallestX = i;
		}
		extAmountX =extension1dirX[secondSmallestX];

		break;
	}
	case 4:
	{
		//find the 2 smallest

		for (int i = 1; i < 4; i++)
			if (extension1dirX[i] < extension1dirX[smallestX])
				smallestX = i;
		if (smallestX == 0)
			secondSmallestX++;
		for (int i = 1; i < 4; i++)
			if ((extension1dirX[i] < extension1dirX[secondSmallestX]) && (i != smallestX))
				secondSmallestX = i;
		extAmountX = extension1dirX[secondSmallestX];

		break;
	}
	}
	//do again for Y direction

	int posSignY(0);
	if (extension1dirY[0] > 0)
		posSignY++;

	if (extension1dirY[1] > 0)
		posSignY++;

	if (extension1dirY[2] > 0)
		posSignY++;

	if (extension1dirY[3] > 0)
		posSignY++;

	int smallestY = 0;
	int secondSmallestY = 0;
	bool noExtY(false);
	double extAmountY(0);
	switch (posSignY)
	{
	case 0:
	{
		//find the 2 smallest

		for (int i = 1; i < 4; i++)
			if (extension1dirY[i] > extension1dirY[smallestY])
				smallestY = i;
		if (smallestY == 0)
			secondSmallestY++;
		for (int i = 1; i < 4; i++)
			if ((extension1dirY[i] > extension1dirY[secondSmallestY]) && (i != smallestY))
				secondSmallestY = i;
		extAmountY = extension1dirY[secondSmallestY];
		break;
	}
	case 1:
	{
		//find the smallest negative
		if (extension1dirY[0] > 0)
			secondSmallestY = 1;
		for (int i = 0; i < 4; i++)
		{
			if (extension1dirY[i] > 0)
				smallestY = i;
			else if (extension1dirY[i] > extension1dirY[secondSmallestY])
				secondSmallestY = i;
		}
		extAmountY = extension1dirY[secondSmallestY];

		break;
	}
	case 2:
	{
		//no extension
		extAmountY = 0;
		noExtY = true;
	}
	case 3:
	{
		//find the smallest positive
		if (extension1dirY[0] < 0)
			secondSmallestY = 1;
		for (int i = 0; i < 4; i++)
		{
			if (extension1dirY[i] < 0)
				smallestY = i;
			else if (extension1dirY[i] < extension1dirY[secondSmallestY])
				secondSmallestY = i;
		}
		extAmountY = extension1dirY[secondSmallestY];

		break;
	}
	case 4:
	{
		//find the 2 smallest

		for (int i = 1; i < 4; i++)
			if (extension1dirY[i] < extension1dirY[smallestY])
				smallestY = i;
		if (smallestY == 0)
			secondSmallestY++;
		for (int i = 1; i < 4; i++)
			if ((extension1dirY[i] < extension1dirY[secondSmallestY]) && (i != smallestY))
				secondSmallestY = i;
		extAmountY = extension1dirY[secondSmallestY];

		break;
	}
	}

	if (!noExtX && !noExtY)
	{
		if (abs(extAmountX) < abs(extAmountY))
		{
			//extend along direction X
			std::vector<glm::dvec3> newCorners = rect1.m_corners;
			newCorners[smallestX] += (extAmountX * plane1dirX);
			newCorners[secondSmallestX] += extAmountX * plane1dirX;
			rect1.updateCorners(newCorners);
		}
		else
		{
			//extend along direction Y
			std::vector<glm::dvec3> newCorners = rect1.m_corners;
			newCorners[smallestY] += (extAmountY * plane1dirY);
			newCorners[secondSmallestY] += extAmountY * plane1dirY;
			rect1.updateCorners(newCorners);
			
		}
	}

	//now do it again for the second plane

	posSignX = 0;
	if (extension2dirX[0] > 0)
		posSignX++;

	if (extension2dirX[1] > 0)
		posSignX++;

	if (extension2dirX[2] > 0)
		posSignX++;

	if (extension2dirX[3] > 0)
		posSignX++;

	smallestX = 0;
	secondSmallestX = 0;
	noExtX = false;
	extAmountX = 0;
	switch (posSignX)
	{
	case 0:
	{
		//find the 2 smallest
		
		for (int i = 1; i < 4; i++)
			if (extension2dirX[i] > extension2dirX[smallestX])
				smallestX = i;
		if (smallestX == 0)
			secondSmallestX++;
		for (int i = 0; i < 4; i++)
			if ((extension2dirX[i] > extension2dirX[secondSmallestX])&&(i!=smallestX))
				secondSmallestX = i;
		extAmountX = extension2dirX[secondSmallestX];
		break;
	}
	case 1:
	{
		//find the smallest negative
		if (extension2dirX[0] > 0)
			secondSmallestX = 1;
		for (int i = 0; i < 4; i++)
		{
			if (extension2dirX[i] > 0)
				smallestX = i;
			else if (extension2dirX[i] > extension2dirX[secondSmallestX])
				secondSmallestX = i;
		}
		extAmountX = extension2dirX[secondSmallestX];

		break;
	}
	case 2:
	{
		//no extension
		extAmountX = 0;
		noExtX = true;
	}
	case 3:
	{
		//find the smallest positive
		if (extension2dirX[0] < 0)
			secondSmallestX = 1;
		for (int i = 0; i < 4; i++)
		{
			if (extension2dirX[i] < 0)
				smallestX = i;
			else if (extension2dirX[i] < extension2dirX[secondSmallestX])
				secondSmallestX = i;
		}
		extAmountX =extension2dirX[secondSmallestX];

		break;
	}
	case 4:
	{
		//find the 2 smallest

		for (int i = 1; i < 4; i++)
			if (extension2dirX[i] < extension2dirX[smallestX])
				smallestX = i;
		if (smallestX == 0)
			secondSmallestX++;
		for (int i = 0; i < 4; i++)
			if ((extension2dirX[i] < extension2dirX[secondSmallestX]) && (i != smallestX))
				secondSmallestX = i;
		extAmountX = extension2dirX[secondSmallestX];

		break;
	}
	}
	//do again for Y direction

	posSignY = 0;
	if (extension2dirY[0] > 0)
		posSignY++;

	if (extension2dirY[1] > 0)
		posSignY++;

	if (extension2dirY[2] > 0)
		posSignY++;

	if (extension2dirY[3] > 0)
		posSignY++;

	smallestY = 0;
	secondSmallestY = 0;
	noExtY = false;
	extAmountY = 0;
	switch (posSignY)
	{
	case 0:
	{
		//find the 2 smallest

		for (int i = 1; i < 4; i++)
			if (extension2dirY[i] > extension2dirY[smallestY])
				smallestY = i;
		if (smallestY == 0)
			secondSmallestY++;
		for (int i = 0; i < 4; i++)
			if ((extension2dirY[i] > extension2dirY[secondSmallestY]) && (i != smallestY))
				secondSmallestY = i;
		extAmountY = extension2dirY[secondSmallestY];
		break;
	}
	case 1:
	{
		//find the smallest negative
		if (extension2dirY[0] > 0)
			secondSmallestY = 1;
		for (int i = 0; i < 4; i++)
		{
			if (extension2dirY[i] > 0)
				smallestY = i;
			else if (extension2dirY[i] > extension2dirY[secondSmallestY])
				secondSmallestY = i;
		}
		extAmountY = extension2dirY[secondSmallestY];

		break;
	}
	case 2:
	{
		//no extension
		extAmountY = 0;
		noExtY = true;
	}
	case 3:
	{
		//find the smallest positive
		if (extension2dirY[0] < 0)
			secondSmallestY = 1;
		for (int i = 0; i < 4; i++)
		{
			if (extension2dirY[i] < 0)
				smallestY = i;
			else if (extension2dirY[i] < extension2dirY[secondSmallestY])
				secondSmallestY = i;
		}
		extAmountY = extension2dirY[secondSmallestY];

		break;
	}
	case 4:
	{
		//find the 2 smallest

		for (int i = 1; i < 4; i++)
			if (extension2dirY[i] < extension2dirY[smallestY])
				smallestY = i;
		if (smallestY == 0)
			secondSmallestY++;
		for (int i = 0; i < 4; i++)
			if ((extension2dirY[i] < extension2dirY[secondSmallestY]) && (i != smallestY))
				secondSmallestY = i;
		extAmountY = extension2dirY[secondSmallestY];

		break;
	}
	}

	if (!noExtX && !noExtY)
	{
		if (abs(extAmountX) < abs(extAmountY))
		{
			//extend along direction X
			std::vector<glm::dvec3> newCorners = rect2.m_corners;
			newCorners[smallestX] += (extAmountX * plane2dirX);
			newCorners[secondSmallestX] += extAmountX * plane2dirX;
			rect2.updateCorners(newCorners);
		}
		else
		{
			//extend along direction Y
			std::vector<glm::dvec3> newCorners = rect2.m_corners;
			newCorners[smallestY] += (extAmountY * plane2dirY);
			newCorners[secondSmallestY] += extAmountY * plane2dirY;
			rect2.updateCorners(newCorners);
		}
	}
}

double OctreeRayTracing::computeExtensionTowardsLine(const glm::dvec3& origin, const glm::dvec3& extDirection, const glm::dvec3& linePoint, const glm::dvec3 lineDirection)
{
	//cross(O+t*uX-lineP,lineD)=0
	//cross (A+t u, d)=0
	// (A[y]+tu[y])*d[z]-(A[z]+tu[z])*d[y])=0
	// (A[z]+tu[z])*d[x]-(A[x]+tu[x])*d[z])=0
	// (A[x]+tu[x])*d[y]-(A[y]+tu[y])*d[z])=0

	//t(u[y]*d[z]-u[z]*d[y]) = A[z]*d[y]-A[y]*d[z]
	//

	//returns t such that origin+t*extDirection is a point on the line. assumes everything is coplanar

	//start by testing if things are coplanar, for debug
	std::vector<glm::dvec3> testPoints(0);
	testPoints.push_back(linePoint);
	testPoints.push_back(linePoint + lineDirection);
	testPoints.push_back(linePoint + extDirection);
	std::vector<double> testPlane(0);
	fitPlane(testPoints, testPlane);
	glm::dvec3 originProj = MeasureClass::projectPointToPlane(origin, testPlane);
	if (glm::length(origin - originProj) > 0.05)
	{
		//problem, stuff is not coplanar
		double length = glm::length(origin - originProj);
		bool toDelete(false);
	}
	
	glm::dvec3 A = origin - linePoint;
	int divCase = 0;
	double div = extDirection[1] * lineDirection[2] - extDirection[2] * lineDirection[1];
	double tempDiv= extDirection[2] * lineDirection[0] - extDirection[0] * lineDirection[2];
	if (abs(div) < abs(tempDiv))
	{
		divCase = 1;
		div = tempDiv;
	}
	tempDiv = extDirection[0] * lineDirection[1] - extDirection[1] * lineDirection[0];
	if (abs(div) < abs(tempDiv))
	{
		divCase = 2;
		div = tempDiv;
	}

	double t = 0;
	switch (divCase)
	{
	case 0:
	{
		t = A[2] * lineDirection[1] - A[1] * lineDirection[2];
		t /= div;
		break;
	}
	case 1:
	{
		t = A[0] * lineDirection[2] - A[2] * lineDirection[0];
		t /= div;
		break;
	}
	case 2:
	{
		t = A[1] * lineDirection[0] - A[0] * lineDirection[1];
		t /= div;
		break;
	}
	}

	return t;
}