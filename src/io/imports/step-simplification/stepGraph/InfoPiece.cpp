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

#include "io/imports/step-simplification/stepGraph/InfoPiece.h"

InfoPiece::InfoPiece()
    : depth(0), nbEntities(0), occurrences()
{}

InfoPiece::InfoPiece(unsigned int _depth, unsigned int _nbEntities, const std::string& entityName)
    : depth(_depth), nbEntities(_nbEntities)
{
    occurrences[entityName] = 1;
}

InfoPiece::InfoPiece(unsigned int _depth,
                     unsigned int _nbEntities,
                     const std::unordered_map<std::string, unsigned int>& _occurrences)
    : depth(_depth), nbEntities(_nbEntities), occurrences(_occurrences)
{}

/**
 * Concatenation of two objects InfoPiece
 */
void InfoPiece::operator+=(const InfoPiece &infoPiece) {
    if (infoPiece.depth > depth) depth = infoPiece.depth;
    nbEntities += infoPiece.nbEntities;

    for (auto occurrence : infoPiece.occurrences) {
        if (occurrences.count(occurrence.first) == 0)
            occurrences[occurrence.first] = occurrence.second;
        else occurrences[occurrence.first] += occurrence.second;
    }
}

bool InfoPiece::operator==(const InfoPiece &infoPiece) const {
    if (depth != infoPiece.depth) return false;
    if (nbEntities != infoPiece.nbEntities) return false;
    if (!(occurrences == infoPiece.occurrences)) return false;

    return true;
}
