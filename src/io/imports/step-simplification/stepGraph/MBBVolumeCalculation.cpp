/* Copyright (C) 2020 Antoine Garnier <antoine.garnier1@etu.univ-nantes.fr>
 * Copyright (C) 2020 Evan Gisdal <evan.gisdal@etu.univ-nantes.fr>
 * Copyright (C) 2020 Antoine Humbert <antoine.humbert@etu.univ-nantes.fr>
 *
 * This file is part of a transversal project made by students from the computer science department
 * of the Graduate School of Engineering of the University of Nantes. It was realized as an attempt
 * to solve the problem of the simplification of 3D models with the STEP format.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "io/imports/step-simplification/stepGraph/MBBVolumeCalculation.h"

#include <sstream>

MBBVolumeCalculation::MBBVolumeCalculation(Entity* _root, unsigned int nbOfNodes)
    : minX(std::numeric_limits<double>::max()), maxX(std::numeric_limits<double>::lowest()),
      minY(std::numeric_limits<double>::max()), maxY(std::numeric_limits<double>::lowest()),
      minZ(std::numeric_limits<double>::max()), maxZ(std::numeric_limits<double>::lowest()),
      root(_root)
{
    // If the coordinates can't be a reliable indicator for the volume of the piece (as the case of the cylinders)
    if(!getCoordinates(root, nbOfNodes)) {
        // We adapt the coordinates on the axes so that they are representative of the geometry of the part in space
        CylinderCoordinates c;
        calculateCylinderVolumeMBB(root, c);
        setCylinderCoordinates(c);
    }
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) tokens.push_back(token);
    return tokens;
}

void MBBVolumeCalculation::updateCoordinates(double x, double y, double z) {
    if (x > maxX) maxX = x;
    if (x < minX) minX = x;
    if (y > maxY) maxY = y;
    if (y < minY) minY = y;
    if (z > maxZ) maxZ = z;
    if (z < minZ) minZ = z;
}

/*double round( double val ) {
    if( val < 0 ) return std::ceil(val - 0.001);
    return floor(val + 0.001);
}*/

/**
 * Method adapting the volume calculation for a cylindrical shape
 * The coordinates of the axes are recursively are filled in as we go along
 * @param currentEntity The entity at the level from which recovery is recursively performed
 */
void MBBVolumeCalculation::calculateCylinderVolumeMBB(Entity *currentEntity, CylinderCoordinates& cylinder) {
    std::smatch matchInfo;
    auto pattern = std::regex(R"(.*\((.*)\).*)");

    // We need to visit children to recursively get coordinates in the part
    std::unordered_set<Entity *> children = currentEntity->getChildren();
    for (auto child : children) {
        calculateCylinderVolumeMBB(child, cylinder);
        if (child->getName() == "CARTESIAN_POINT") {
            if (std::regex_match(child->getArguments(), matchInfo, pattern)) {
                std::vector<std::string> values = split(matchInfo[1], ',');
                if (values.size() == 3) {
                    auto itXCoordinates = cylinder.xCoordinates.find(round(std::stod(values[0]) * 1000.0) / 1000.0);
                    auto itYCoordinates = cylinder.yCoordinates.find(round(std::stod(values[1]) * 1000.0) / 1000.0);
                    auto itZCoordinates = cylinder.zCoordinates.find(round(std::stod(values[2]) * 1000.0) / 1000.0);
                    if (itXCoordinates != cylinder.xCoordinates.end())
						itXCoordinates->second++;
                    else
						cylinder.xCoordinates.insert({round(std::stod(values[0]) * 1000.0) / 1000.0, 1});
                    if (itYCoordinates != cylinder.yCoordinates.end()) 
						itYCoordinates->second++;
                    else
						cylinder.yCoordinates.insert({round(std::stod(values[1]) * 1000.0) / 1000.0, 1});
                    if (itZCoordinates != cylinder.zCoordinates.end())
						itZCoordinates->second++;
                    else 
						cylinder.zCoordinates.insert({round(std::stod(values[2]) * 1000.0) / 1000.0, 1});
                }
            }
        }
    }
}

/**
 * Method getting recursively the minimal and maximal coordinates of the StepPieces
 * @param currentEntity The entity at the level from which recovery is recursively performed
 */
bool MBBVolumeCalculation::getCoordinates(Entity *currentEntity, unsigned int nbOfNodes) {
    std::smatch matchInfo;
    auto pattern = std::regex(R"(.*\((.*)\).*)");

    // We need to visit children to recursively get both minimum and maximum coordinates in the part
    std::unordered_set<Entity *> children = currentEntity->getChildren();
    for (auto child : children) {
        if (!getCoordinates(child, nbOfNodes)) 
            return false;
        if (child->getName() == "CARTESIAN_POINT") {
            if (std::regex_match(child->getArguments(), matchInfo, pattern)) {
                std::vector<std::string> coordinates = split(matchInfo[1], ',');
                if (coordinates.size() == 3) updateCoordinates(
                        round(std::stod(coordinates[0]) * 1000.0) / 1000.0,
                        round(std::stod(coordinates[1]) * 1000.0) / 1000.0,
                        round(std::stod(coordinates[2]) * 1000.0) / 1000.0 );
            }
        }
        // If a cylindrical shape has been detected, and that the piece is relatively few complex, we have to adapt the process for this particular case
        else if (child->getName() == "CYLINDRICAL_SURFACE" && nbOfNodes < 400) 
            return false;
    }

    return true;
}

/**
 * Methode returning the volume of the piece associated with the current object
 * @return The volume if all the min/max coordinates are found, 0 instead
 */
double MBBVolumeCalculation::getVolumeMBB() {
    if ((maxX == std::numeric_limits<double>::lowest()) || (maxY == std::numeric_limits<double>::lowest()) || (maxZ == std::numeric_limits<double>::lowest()) ||
        (minX == std::numeric_limits<double>::max()) || (minY == std::numeric_limits<double>::max()) || (minZ == std::numeric_limits<double>::max())) 
        return 0;
    double minimumSpace = 0.01;
    double volume = (maxX - minX) != 0. ? (maxX - minX) : minimumSpace;
    volume *= (maxY - minY) != 0. ? (maxY - minY) : minimumSpace;
    volume *= (maxZ - minZ) != 0. ? (maxZ - minZ) : minimumSpace;

    return volume;
}

std::ostream &operator<<(std::ostream &out, const MBBVolumeCalculation &mbbVolumeCalculation) {
    out <<  "minX: " << std::to_string(mbbVolumeCalculation.minX) << ", maxX: " << std::to_string(mbbVolumeCalculation.maxX)
        <<  ",minY: " << std::to_string(mbbVolumeCalculation.minY) << ", maxY: " << std::to_string(mbbVolumeCalculation.maxY)
        <<  ",minZ: " << std::to_string(mbbVolumeCalculation.minZ) << ", maxZ: " << std::to_string(mbbVolumeCalculation.maxZ);
    return out;
}

/**
 * Method adapting the coordinates on the axes so that they are representative of the geometry of the cylinder in space
 * @param cylinder The object containing the different coordinates of the axis
 */
void MBBVolumeCalculation::setCylinderCoordinates(CylinderCoordinates &cylinder) {
    // Deletion of minority among cartesian points (because they are not reliable)
    unsigned int reliableThreshold = 7; // The threshold at which we consider the number of points (at a particular location in space) large enough to consider the coordinate reliable.
    for (auto xCoordinate : cylinder.xCoordinates)
        if (xCoordinate.second < reliableThreshold) {
            cylinder.xCoordinates.erase(xCoordinate.first);
            break;
        }
    for (auto yCoordinate : cylinder.yCoordinates)
        if (yCoordinate.second < reliableThreshold) {
            cylinder.yCoordinates.erase(yCoordinate.first);
            break;
        }
    for (auto zCoordinate : cylinder.zCoordinates)
        if (zCoordinate.second < reliableThreshold) {
            cylinder.zCoordinates.erase(zCoordinate.first);
            break;
        }

    // Then, we have to get the maximal coordinate value of each axes
    std::map<double, unsigned int>::iterator last_elemX, last_elemY, last_elemZ;
    /*for (auto iter = cylinder.xCoordinates.begin(); iter != cylinder.xCoordinates.end(); iter++) 
		last_elemX = iter;
    for (auto iter = cylinder.yCoordinates.begin(); iter != cylinder.yCoordinates.end(); iter++) 
		last_elemY = iter;
    for (auto iter = cylinder.zCoordinates.begin(); iter != cylinder.zCoordinates.end(); iter++) 
		last_elemZ = iter;*/
	if(!cylinder.xCoordinates.empty())
		last_elemX = --cylinder.xCoordinates.end();
	else
		return;

	if (!cylinder.yCoordinates.empty())
		last_elemY = --cylinder.yCoordinates.end();
	else
		return;

	if (!cylinder.zCoordinates.empty())
		last_elemZ = --cylinder.zCoordinates.end();
	else
		return;

    double diameter;
	double newMinX, newMaxX, newMinY, newMaxY, newMinZ, newMaxZ;
    // Finding the main axis (The one with the most coordinates)
    if (cylinder.xCoordinates.size() >= cylinder.yCoordinates.size() &&
        cylinder.xCoordinates.size() >= cylinder.zCoordinates.size()) {
        diameter = std::abs(cylinder.xCoordinates.begin()->first -
                            last_elemX->first);   // This axis is then used as a reference to define the diameter value.
		newMinX = cylinder.xCoordinates.begin()->first;
		newMaxX = last_elemX->first;              // Setting the value for the main axis
        // Finding the axis associated with the height of the cylinder (The second one with the most coordinates)
        if (cylinder.yCoordinates.size() >= cylinder.zCoordinates.size()) {
            // Assigning new coordinates from the diameter (main axis) for the axis with the least number of different coordinates
			newMinZ = cylinder.zCoordinates.begin()->first - (diameter / 2);
			newMaxZ = last_elemZ->first + (diameter / 2);
			newMinY = cylinder.yCoordinates.begin()->first;
			newMaxY = last_elemY->first; // Setting the value for the height axis
        } else {
            // Assigning new coordinates from the diameter (main axis) for the axis with the least number of different coordinates
			newMinY = cylinder.yCoordinates.begin()->first - (diameter / 2);
			newMaxY = last_elemY->first + (diameter / 2);
			newMinZ = cylinder.zCoordinates.begin()->first;
			newMaxZ = last_elemZ->first; // Setting the value for the height axis
        }
    } else if (cylinder.yCoordinates.size() >= cylinder.zCoordinates.size()) {
        diameter = std::abs(cylinder.yCoordinates.begin()->first - last_elemY->first);
		newMinY = cylinder.yCoordinates.begin()->first;
		newMaxY = last_elemY->first;
        if (cylinder.xCoordinates.size() >= cylinder.zCoordinates.size()) {
			newMinZ = cylinder.zCoordinates.begin()->first - (diameter / 2);
			newMaxZ = last_elemZ->first + (diameter / 2);
			newMinX = cylinder.xCoordinates.begin()->first;
			newMaxX = last_elemX->first;
        } else {
			newMinX = cylinder.xCoordinates.begin()->first - (diameter / 2);
			newMaxX = last_elemX->first + (diameter / 2);
			newMinZ = cylinder.zCoordinates.begin()->first;
			newMaxZ = last_elemZ->first;
        }
    } else {
        diameter = std::abs(cylinder.zCoordinates.begin()->first - last_elemZ->first);
		newMinZ = cylinder.zCoordinates.begin()->first;
		newMaxZ = last_elemZ->first;
        if (cylinder.xCoordinates.size() >= cylinder.yCoordinates.size()) {
			newMinY = cylinder.yCoordinates.begin()->first - (diameter / 2);
			newMaxY = last_elemY->first + (diameter / 2);
			newMinX = cylinder.xCoordinates.begin()->first;
			newMaxX = last_elemX->first;
        } else {
			newMinX = cylinder.xCoordinates.begin()->first - (diameter / 2);
			newMaxX = last_elemX->first + (diameter / 2);
			newMinY = cylinder.yCoordinates.begin()->first;
			newMaxY = last_elemY->first;
        }
    }

	updateCoordinates(newMinX, newMinY, newMinZ);
	updateCoordinates(newMaxX, newMaxY, newMaxZ);
}