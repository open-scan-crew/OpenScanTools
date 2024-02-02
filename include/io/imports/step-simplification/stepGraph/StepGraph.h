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

#ifndef STEPGRAPH_H
#define STEPGRAPH_H

#include <map>
#include <list>
#include <stack>

#include "../deletePiece/InfoDeletionPiece.h"
#include "StepPiece.h"

/**
 * Dependency graph of the step file
 *
 * @param nodes       The entities of the STEP model dependency graph
 * @param stepPieces  List of pieces in the graph
 */
class StepGraph {

private:
    std::map<unsigned int, Entity*>  nodes;
    std::list<StepPiece*> stepPieces;
    std::stack<InfoDeletionPiece> history;
    unsigned int nbPieces;

public:
    StepGraph();
    ~StepGraph();

    inline std::map<unsigned int, Entity*>& getNodes() { return nodes; }
    inline const std::list<StepPiece*>& getStepPieces() { return stepPieces; }
    inline size_t getNbEntities() const { return nodes.size(); }
    inline size_t getInitialNumberOfPieces() const { return stepPieces.size(); }
    inline size_t getNumberOfDeletedPieces() const { return getInitialNumberOfPieces() - getNumberOfPiece(); }
    inline unsigned int getNumberOfPiece() const { return nbPieces; }

    void createStepPieces(const std::list<Entity*>& stepPieces);

    void addNode(Entity*);
    void addStepPiece(StepPiece*);

    void addDeletionToHistory(const InfoDeletionPiece &);
    void restoreLastDeletedPiece();
    void restoreInitialModel();
    void findEntityToRestore(Entity *, std::list<Entity*> &);

    void deleteEntity(const Entity& entity);

    bool operator==(const StepGraph& stepGraph);
    inline bool operator!=(const StepGraph& stepGraph) { return !(*this == stepGraph); }
};

#endif // STEPGRAPH_H