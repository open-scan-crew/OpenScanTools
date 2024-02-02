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

#ifndef UPDATEPIECECHILDREN_H
#define UPDATEPIECECHILDREN_H

#include <unordered_set>

#include "../stepGraph/StepGraph.h"

/**
 * Delete piece's entities
 * @arg stepGraph
 * @arg entitiesToVisit List of entities to visit
 * @arg entitiesToDelete List of entities to delete
 */
class DeletePieceChildren {

private:
    StepGraph* stepGraph;
    std::unordered_set<Entity*> entitiesToDelete;
    Entity* entityStart;

public:
    DeletePieceChildren(StepGraph* _stepGraph, Entity* _entityStart);

    inline const std::unordered_set<Entity*> getEntitiesToDelete() const { return entitiesToDelete; }

    void findEntitiesToDelete();
    void deleteEntities();

private:
    bool allParentsInDelList(const Entity& entity) const;
};

#endif // UPDATEPIECECHILDREN_H