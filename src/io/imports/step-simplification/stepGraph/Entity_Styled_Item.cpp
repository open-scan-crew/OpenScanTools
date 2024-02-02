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

#include "io/imports/step-simplification/stepGraph/Entity_Styled_Item.h"

Entity_Styled_Item::Entity_Styled_Item(unsigned int _id, const std::string& _name, const std::string& _arguments)
    : Entity(_id, _name, _arguments)
{}

bool Entity_Styled_Item::canUpdateReference(Entity *entityChild, std::list<Entity*> &entitiesToDelete) {
    if (getNbChildren() != 2) return false;
    for (auto parent : getParents()) {
        if (!parent->canUpdateReference(this, entitiesToDelete)) return false;
    }
    entitiesToDelete.push_back(this);
    return true;
}

void Entity_Styled_Item::updateChildReference(Entity *entityChild, StepGraph& graph) {
    for (auto parent : getParents()) parent->updateChildReference(this, graph);
    graph.deleteEntity(*this) ;
}
