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

#ifndef INFOPIECE_H
#define INFOPIECE_H

#include <unordered_map>
#include <string>

/**
 * Information of a piece
 * @arg depth Depth of the sub-graph of the piece
 * @arg nbEntities Number of entities in the piece
 * @arg occurrences Number of occurrences fo each type of entity in the piece
 */
class InfoPiece {

private:
    unsigned int depth;
    unsigned int nbEntities;
    std::unordered_map<std::string, unsigned int> occurrences;

public:
    InfoPiece();
    InfoPiece(unsigned int _depth,
              unsigned int _nbEntities,
              const std::string& entityName);
    InfoPiece(unsigned int _depth,
              unsigned int _nbEntities,
              const std::unordered_map<std::string, unsigned int>& _occurrences);

    inline unsigned int getDepth() const { return depth; }
    inline unsigned int getNbEntities() const { return nbEntities; }
    inline std::unordered_map<std::string, unsigned int> getOccurrences() const { return occurrences; }

    inline void incrDepth() { ++depth; }

    bool operator==(const InfoPiece& infoPiece) const;
    inline bool operator!=(const InfoPiece& infoPiece) const { return !(*this == infoPiece); }
    void operator+=(const InfoPiece& infoPiece);
};

#endif // INFOPIECE_H