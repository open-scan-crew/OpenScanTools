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

#ifndef MBBVOLUMECALCULATION_H
#define MBBVOLUMECALCULATION_H

#include <limits>
#include <map>

#include "Entity.h"

struct CylinderCoordinates {
    std::map<double, unsigned int> xCoordinates;
    std::map<double, unsigned int> yCoordinates;
    std::map<double, unsigned int> zCoordinates;
};

class MBBVolumeCalculation {

private:
    double minX, maxX;
    double minY, maxY;
    double minZ, maxZ;
    Entity *root;

private:
    bool getCoordinates(Entity*, unsigned int nbNodes);
    void updateCoordinates(double x, double y, double z);
    void calculateCylinderVolumeMBB(Entity*, CylinderCoordinates&);
    void setCylinderCoordinates(CylinderCoordinates&);

public:
    explicit MBBVolumeCalculation(Entity*, unsigned int nbNodes);
    double getVolumeMBB();
    inline double getXAxisDistance() const { return maxX - minX; }
    inline double getYAxisDistance() const { return maxY - minY; }
    inline double getZAxisDistance() const { return maxZ - minZ; }
    inline void resetCoordinates() { minX = std::numeric_limits<double>::max(); minY = std::numeric_limits<double>::max(); minZ = std::numeric_limits<double>::max(); maxX = std::numeric_limits<double>::lowest(); maxY = std::numeric_limits<double>::lowest(); maxZ = std::numeric_limits<double>::lowest(); }

    friend std::ostream& operator<<(std::ostream& out, const MBBVolumeCalculation& mbbVolumeCalculation);
};

#endif // MBBVOLUMECALCULATION_H
