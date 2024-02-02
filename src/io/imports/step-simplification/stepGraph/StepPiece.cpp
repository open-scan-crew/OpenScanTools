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

#include "io/imports/step-simplification/stepGraph/StepPiece.h"

StepPiece::StepPiece(Entity* _root)
    : root(_root)
{
    InfoPiece infoPiece = getInfoClassification(*_root);
    depth = infoPiece.getDepth();
    nbEntities = infoPiece.getNbEntities();
    auto mbbVolumeCalculation =  MBBVolumeCalculation(_root, nbEntities);
    volume = mbbVolumeCalculation.getVolumeMBB();
    xAxisDistance = mbbVolumeCalculation.getXAxisDistance();
    yAxisDistance = mbbVolumeCalculation.getYAxisDistance();
    zAxisDistance = mbbVolumeCalculation.getZAxisDistance();
    occurrences = infoPiece.getOccurrences();
    score = -1;
}

StepPiece::StepPiece(Entity *_root, unsigned int _depth, unsigned int _nbEntities, const std::unordered_map<std::string, unsigned int>& _occurrences)
    : root(_root), depth(_depth), nbEntities(_nbEntities), occurrences(_occurrences), score(-1)
{}

StepPiece::StepPiece(Entity* _root, const InfoPiece& infoPiece)
    : root(_root)
{
    depth = infoPiece.getDepth();
    nbEntities = infoPiece.getNbEntities();
    occurrences = infoPiece.getOccurrences();
    score = -1;
}

/**
 * The methode retrieving the information of the StepPiece
 * @param entityPiece The root entity of a piece
 * @return The InfoPiece object helping to fill in the attributes of the current StepPiece
 */
InfoPiece StepPiece::getInfoClassification(const Entity &entityPiece) {
    auto infoPiece = InfoPiece(0, 1, entityPiece.getName());
    for (auto child : entityPiece.getChildren()) {
        if (child != NULL) {
            InfoPiece childInfoPiece = getInfoClassification(*child);
            infoPiece += childInfoPiece;
        }
    }
    infoPiece.incrDepth();

    return infoPiece;
}