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

#ifndef STEPPIECE_H
#define STEPPIECE_H

#include <unordered_map>

#include "Entity.h"
#include "InfoPiece.h"

class StepPiece {

private:
    Entity* root;
    unsigned int depth;
    unsigned int nbEntities;
    double volume;
    double xAxisDistance, yAxisDistance, zAxisDistance;

    std::unordered_map<std::string, unsigned int> occurrences;

    double score;

public:
    StepPiece(Entity* _root);
    StepPiece(Entity* _root, unsigned int _depth, unsigned int _nbEntities, const std::unordered_map<std::string, unsigned int>& _occurrences);
    StepPiece(Entity* _root, const InfoPiece& infoPiece);

    ~StepPiece() = default;

    // Getters
    inline Entity* getRoot() const { return root; }
    inline unsigned int getDepth() const { return depth; }
    inline unsigned int getNbEntities() const { return nbEntities; }
    inline double getVolumeMBB() const {return volume; }
    inline double getXAxisDistance() const {return xAxisDistance; }
    inline double getYAxisDistance() const {return yAxisDistance; }
    inline double getZAxisDistance() const {return zAxisDistance; }
    inline std::unordered_map<std::string, unsigned int> getOccurrences() const { return occurrences; }

    inline double getScore() { return score; }
    inline void setScore(double score) { this->score = score; }


    InfoPiece getInfoClassification(const Entity& entityPiece);
};

#endif //STEPPIECE_H