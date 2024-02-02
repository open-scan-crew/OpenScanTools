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

#include "io/imports/step-simplification/deletePiece/DeletePiece.h"

DeletePiece::DeletePiece(Entity* _entityOfPieces, StepGraph* _stepGraph)
    : entityOfPiece(_entityOfPieces),
      stepGraph(_stepGraph),
      updateParents(_stepGraph, _entityOfPieces),
      deleteChildren(_stepGraph, _entityOfPieces)
{}

/**
 * Starts the deletion procedure and indicates if the deletion has been completed.
 * @return 0 if the part can't be deleted, 1 instead
 */
unsigned int DeletePiece::deletePiece() {
    auto parentsToDelete = std::list<Entity *>();

    // Checks the possibility of updating high level entities above the part entities
    if (!updateParents.canUpdate(parentsToDelete)) {
        std::cout << "Deletion of the piece #" << entityOfPiece->getId() << " impossible, " << std::endl;
        return 0;
    }

    // We need to save some data about deletion (to keep a history for the restoration)
    auto infoDelPiece = new InfoDeletionPiece(entityOfPiece, updateParents.launchUpdateParents(entityOfPiece, parentsToDelete), parentsToDelete);

    // Recovery of entities from the subgraph associated with a part
    deleteChildren.findEntitiesToDelete();
    // Deletion of children
    deleteChildren.deleteEntities();
    // Deletion of the root entity
    stepGraph->deleteEntity(*entityOfPiece);
    // Deletion of unnecessary parents
    for (auto p : parentsToDelete) stepGraph->deleteEntity(*p);

    // Adding the necessary data to the history
    stepGraph->addDeletionToHistory(*infoDelPiece);
    return 1;
}