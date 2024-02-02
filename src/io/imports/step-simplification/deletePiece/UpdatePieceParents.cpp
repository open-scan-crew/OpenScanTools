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

#include "io/imports/step-simplification/deletePiece/UpdatePieceParents.h"

UpdatePieceParents::UpdatePieceParents(StepGraph* _stepGraph, Entity* _entityOrigin)
    : stepGraph(_stepGraph), entityOrigin(_entityOrigin)
    {}

/**
 * Check if the parents of entityOrigin can be updated
 * @return true if the update is impossible
 */
bool UpdatePieceParents::canUpdate(std::list<Entity *> &parentsToDelete) const {
    for (auto parent : entityOrigin->getParents()) {
        if (!parent->canUpdateReference(entityOrigin, parentsToDelete)) return false;
    }
    return true;
}

/**
 * Recursive method returning the list of parents to update with the old arguments associated with them
 * @param current The current entity allowing the recursion
 * @param parentsToDelete The list of unnecessary high level entities to remove for the piece deletion
 * @return The map with the identifier of the entity to update as key and the string containing its old parameters as value.
 */
std::map<unsigned int, std::string> UpdatePieceParents::launchUpdateParents(Entity *current, const std::list<Entity*>& parentsToDelete) const {
    auto previousArgsParents = std::map<unsigned int, std::string>();
    for (auto parent : current->getParents()) {
        // we need to ensure that the entity we are about to update does not belong to the list of parent entities to delete
        if (isEntityParentToDelete(parent, parentsToDelete)) {
            auto previousArgs = launchUpdateParents(parent, parentsToDelete);
            for (const auto& prev : previousArgs) previousArgsParents.insert({prev.first, prev.second});
        }
        else {
            previousArgsParents.insert({parent->getId(), parent->getArguments()});
            parent->updateChildReference(current, *stepGraph);
        }
    }
    return previousArgsParents;
}


/**
 * Checking if the entity we are about to update does not belong to the list of parent entities to delete
 * @param e The entity to compare
 * @param entitiesParentsToDelete The list of parents to delete
 * @return true if the entity belongs to the list, false instead
 */
bool UpdatePieceParents::isEntityParentToDelete(Entity *e, const std::list<Entity*>& entitiesParentsToDelete) {
    for (auto p : entitiesParentsToDelete) if (p->getId() == e->getId()) return true;
    return false;
}