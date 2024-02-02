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

#ifndef ENTITY_P_L_ASS_H
#define ENTITY_P_L_ASS_H

#include "Entity.h"
#include "StepGraph.h"

#include <list>
/**
 * Entity PRESENTATION_LAYER_ASSIGMENT
 */

class Entity_P_L_Ass : public Entity {

public:
    Entity_P_L_Ass(unsigned int _id, const std::string& _name, const std::string& _arguments);

    bool canUpdateReference(Entity* entityChild, std::list<Entity*> &entitiesToDelete) override;
    void updateChildReference(Entity* entityChild, StepGraph& graph) override;
};

#endif // ENTITY_P_L_ASS_H