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

#include "io/imports/step-simplification/stepGraph/Entity.h"

#include <sstream>

Entity::Entity(unsigned int _id, std::string  _name, std::string  _arguments) :
    id(_id), name(std::move(_name)), arguments(std::move(_arguments))
{}

void Entity::addParent(Entity* entity) {
    parents.insert(entity);
}

void Entity::addChild(Entity* entity) {
    children.insert(entity);
}

/**
 * Method which must be override in specialized classes
 * @return false because by default an entity cannot be updated
 */
bool Entity::canUpdateReference(Entity* entityChild, std::list<Entity *> &entitiesToDelete) {
    return false;
}

/**
 * Method which must be override in specialized classes
 * @throw runtime::error because by default an entity cannot be updated
 */
void Entity::updateChildReference(Entity* entityChild, StepGraph& graph) {
    throw std::runtime_error("This entity does not support update child reference");
}

/**
 * Delete the reference of entityChild
 * @param entityChild
 */
std::string Entity::updateReference(Entity* entityChild) {
    std::string idEntityString = std::to_string(entityChild->getId());
    std::string argsBeforeUpdate = arguments;
    argsBeforeUpdate.erase(std::remove(argsBeforeUpdate.begin(), argsBeforeUpdate.end(), '\r'), argsBeforeUpdate.end()); // We remove line breaks in the arguments

    std::regex startList ("[(][ ]?#" + idEntityString + "[ ]?,"  );
    std::regex middleList(",[ ]?#"   + idEntityString + "[ ]?,"  );
    std::regex endList   (",[ ]?#"   + idEntityString + "[ ]?[)]");

    std::smatch res;
    if      (std::regex_search(argsBeforeUpdate, res, middleList)) deleteChildReference(entityChild, middleList, ",");
    else if (std::regex_search(argsBeforeUpdate, res, startList )) deleteChildReference(entityChild, startList , "(");
    else if (std::regex_search(argsBeforeUpdate, res, endList   )) deleteChildReference(entityChild, endList   , ")");
    /*else {
        std::stringstream error;
        error << "Update of reference #" << idEntityString << " impossible. Can't find the reference among the arguments of #" << this->getId() << ".";
        throw std::runtime_error(error.str());
    }*/

    return argsBeforeUpdate;
}

/**
 * Apply the deletion in the arguments and the children list
 * @param entityChild Entity to delete in the children list
 * @param regex String to delete in the arguments
 * @param newString String to replace to keep the correct entity arguments
 */
void Entity::deleteChildReference(Entity* entityChild, std::regex regex, std::string newString) {
    arguments = std::regex_replace(getArguments(), regex, newString);
    children.erase(entityChild);
}

/**
 * Write an entity in the STEP format
 */
std::ostream& operator<<(std::ostream& out, const Entity& entity) {
    out << "#" << entity.id << " = " << entity.name << "(" << entity.arguments << ");" << std::endl;
    return out;
}

/*
 * Override comparison operator
 */
bool Entity::operator==(const Entity &entity) {
    if (id != entity.id) return false;
    if (name != entity.name) return false;
    if (arguments != entity.arguments) return false;
    return true;
}


