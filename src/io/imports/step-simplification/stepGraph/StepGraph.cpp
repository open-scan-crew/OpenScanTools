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

#include "io/imports/step-simplification/stepGraph/StepGraph.h"

#include <iostream>

StepGraph::StepGraph() : nodes(), stepPieces(), history() {}

StepGraph::~StepGraph() {
    for (auto entity : nodes) delete entity.second;
}

/**
 * Methode adding a new entity to the graph
 * @param node The node to add
 */
void StepGraph::addNode(Entity *node) {
    nodes[node->getId()] = node;
}

/**
 * Methode adding a special entity to the list containing the parts of the model (the root entity of a step part)
 * @param manifold The manifold entity to add into the list
 */
void StepGraph::addStepPiece(StepPiece *manifold) {
    stepPieces.push_back(manifold);
}

/**
 * This method is called by StepFile to create the StepPiece objects containing theirs respective properties
 * @return The list containing the StepPieces objects associated a step file
 */
void StepGraph::createStepPieces(const std::list<Entity*>& nodesOfStepParts) {
    for (auto root : nodesOfStepParts) {
        stepPieces.push_back(new StepPiece(root));
        nbPieces++;
    }
}

/**
 * Method of entity deletion
 * @param entity The entity to delete
 */
 void StepGraph::deleteEntity(const Entity &entity) {
     unsigned int entityId = entity.getId();

     Entity *pEntity = nodes[entityId];
     nodes.erase(entityId);
 }

 /**
  * Comparison operator override
  * @param stepGraph
  * @return True if the compared StepGraph is the same, False indeed
  */
 bool StepGraph::operator==(const StepGraph &stepGraph) {
     if (nodes.size() != stepGraph.nodes.size()) return false;

     auto it1 = nodes.begin();
     auto it2 = stepGraph.nodes.begin();

     auto itEnd1 = nodes.end();

     while (it1 != itEnd1) {
         if (it1->first != it2->first) return false;
         if (*it1->second != *it2->second) return false;
         it1++;
         it2++;
     }

     return true;
 }

void StepGraph::addDeletionToHistory(const InfoDeletionPiece &infoDelPiece) {
    history.push(infoDelPiece);
    nbPieces--;
 }

 /*
  * SHOULD CREATE A NEW CLASS DEDICATED TO RESTORATION OF PARTS
  */

void StepGraph::restoreLastDeletedPiece() {
    // Retrieve the suppression data for the last part
    InfoDeletionPiece infoPieceToRestore = history.top();

    for (auto deletedEntity : infoPieceToRestore.getDeletedParentsEntities()) {
        for (auto parentToUpdate : deletedEntity->getParents()) {
            auto canUpdate = true;
            for (auto deletedEntity : infoPieceToRestore.getDeletedParentsEntities()) {
                if (deletedEntity->getId() == parentToUpdate->getId()) {
                    canUpdate = false; break;
                }
            }
            if (canUpdate) {
                // Updating parents
                auto info = infoPieceToRestore.getPreviousArgsParents();
                Entity *child = nullptr;
                for (auto deletedParent : infoPieceToRestore.getDeletedParentsEntities()) {
                    for (auto parent : deletedParent->getParents()) {
                        if (parent == parentToUpdate) child = deletedParent;
                    }
                }
                if (child != nullptr) {
                    parentToUpdate->setArguments(info.find(parentToUpdate->getId())->second);
                    parentToUpdate->addChild(child);
                } else std::cout << "Impossible to find the entity passed in argument." << std::endl;
            }
        }
    }

    auto entitiesToRestore = std::list<Entity*>();
    findEntityToRestore(infoPieceToRestore.getEntityRoot(), entitiesToRestore);
    entitiesToRestore.push_front(infoPieceToRestore.getEntityRoot());
    for (auto entity : entitiesToRestore) addNode(entity);
    for (auto entityParent : infoPieceToRestore.getDeletedParentsEntities()) addNode(entityParent);
    for (auto parentToUpdate : infoPieceToRestore.getPreviousArgsParents()) getNodes()[parentToUpdate.first]->setArguments(parentToUpdate.second);
    for (auto parentToLink : infoPieceToRestore.getEntityRoot()->getParents()) {
        if (parentToLink->getChildren().find(infoPieceToRestore.getEntityRoot()) == parentToLink->getChildren().end()) parentToLink->addChild(infoPieceToRestore.getEntityRoot());
    }
    history.pop();
    nbPieces++;
}

void StepGraph::restoreInitialModel() {
    while (!history.empty()) {
        restoreLastDeletedPiece();
    }
}

void StepGraph::findEntityToRestore(Entity* ent, std::list<Entity*> &l) {
    // We need to visit children to recursively
    std::unordered_set<Entity *> children = ent->getChildren();
    for (auto child : children) {
        // Get recursively the struct returned by the associated child
        findEntityToRestore(child, l);
        l.push_back(child);
    }
}