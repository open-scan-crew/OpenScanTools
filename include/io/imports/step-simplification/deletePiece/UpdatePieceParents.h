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

#ifndef UPDATEPIECEPARENTS_H
#define UPDATEPIECEPARENTS_H

#include <list>
#include <cassert>
#include <map>

#include "../stepGraph/StepGraph.h"

/**
 * Update piece's parents
 * @arg stepGraph
 * @arg updateList List of update of the parents
 */
class UpdatePieceParents {

private:
    StepGraph* stepGraph;
    Entity* entityOrigin;

public:
    explicit UpdatePieceParents(StepGraph* _stepGraph, Entity* _entityOrigin);

    bool canUpdate(std::list<Entity *> &) const;
    std::map<unsigned int, std::string> launchUpdateParents(Entity* root, const std::list<Entity*>& parentsToDelete) const;
    static bool isEntityParentToDelete(Entity *e, const std::list<Entity*>& entitiesParentsToDelete) ;
};

#endif // UPDATEPIECEPARENTS_H