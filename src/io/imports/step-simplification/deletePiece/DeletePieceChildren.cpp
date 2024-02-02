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

#include "io/imports/step-simplification/deletePiece/DeletePieceChildren.h"

DeletePieceChildren::DeletePieceChildren(StepGraph* _stepGraph, Entity* _entityStart)
    : stepGraph(_stepGraph), entitiesToDelete(), entityStart(_entityStart)
{}

/**
 * Complete the list entitiesToDelete
 */
void DeletePieceChildren::findEntitiesToDelete() {
    std::unordered_set<Entity*> entitiesToVisit = std::unordered_set<Entity*>();

    for (auto entity : entityStart->getChildren()) entitiesToVisit.insert(entity);

    bool newSuppression = true;
    std::unordered_set<Entity*> visitOfTurn;

    while (newSuppression) {
        newSuppression = false;
        visitOfTurn = entitiesToVisit;

        for (auto entity : visitOfTurn) {
            if (entity != NULL) {
                if (allParentsInDelList(*entity)) {
                    newSuppression = true;
                    entitiesToVisit.erase(entity);
                    for (auto child : entity->getChildren()) entitiesToVisit.insert(child);
                    entitiesToDelete.insert(entity);
                }
            }
        }
    }
}

/**
 * Delete entities in the list entitiesToDelete
 */
void DeletePieceChildren::deleteEntities() {
    for (auto entity : entitiesToDelete) stepGraph->deleteEntity(*entity);
}

/**
 * Check if the parents of the entity are in entitiesToDelete
 * @param entity
 * @return true if all parents of entity are in entitiesToDelete
 */
bool DeletePieceChildren::allParentsInDelList(const Entity &entity) const {
    for (auto parent : entity.getParents())
        if (entitiesToDelete.find(parent) == entitiesToDelete.end() && parent != entityStart) return false;
    return true;
}
