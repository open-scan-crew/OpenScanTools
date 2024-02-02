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

#ifndef ENTITY_H
#define ENTITY_H

#include <unordered_set>
#include <string>
#include <iostream>
#include <cassert>
#include <regex>
#include <utility>
#include <list>

class StepGraph;

/**
 * Node of the dependency graph representing an entity of the STEP file
 * Oriented graph with a binary relation : current-entity -> referenced-entity
 *                                                                                             #32 = EDGE_LOOP ('NONE', ( #42317, #82278 )) ;
 * @param id          The id of the entity corresponding to its ID number in the STEP file     Example : #32
 * @param name        The name of the entity corresponding to its type in the STEP file        Example : EDGE_LOOP
 * @param arguments   The string corresponding to the entity arguments in the STEP file        Example : 'NONE', ( #42317, #82278 )
 * @param parents     Set of parent entities of the current entity
 * @param children    Set of children entities of the current entity
 * @param
 */
class Entity {

private:
    // Attributes
    unsigned int                    id;
    std::string                     name;
    std::string                     arguments;
    std::unordered_set<Entity*>     parents;
    std::unordered_set<Entity*>     children;

    // Methode
    void deleteChildReference(Entity* entityChild, std::regex regex, std::string newString);

protected:
    std::string updateReference(Entity* entityChild);

public:
    Entity(unsigned int _id, std::string  _name, std::string  _arguments);
    Entity(const Entity& e) = default;
    virtual ~Entity() = default;

    // Getters
    inline unsigned int getId() const { return id; }
    inline const std::string& getName() const { return name; }
    inline const std::string& getArguments() const { return arguments; }
    inline const std::unordered_set<Entity*>& getParents() const { return parents; }
    inline const std::unordered_set<Entity*>& getChildren() const { return children; }
    inline size_t getNbParents() const { return parents.size(); }
    inline size_t getNbChildren() const { return children.size(); }

    // Setter
    inline void setArguments(std::string args) {arguments = std::move(args); }
    inline void setId(unsigned int _id) {id = _id; }

    // Methods for building dependencies between entities
    void addParent(Entity*);
    void addChild(Entity*);

    // Methods for update the references of entities when deletions are performed
    virtual bool canUpdateReference(Entity* entityChild, std::list<Entity *>& entitiesToDelete);
    virtual void updateChildReference(Entity* entityChild, StepGraph& graph);

    // Overload of operators
    bool operator==(const Entity& entity);
    inline bool operator!=(const Entity& entity) { return !(*this == entity); };
    friend std::ostream& operator<<(std::ostream& out, const Entity& entity);
};

#endif // ENTITY_H
