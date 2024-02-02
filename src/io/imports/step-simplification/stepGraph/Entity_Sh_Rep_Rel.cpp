#include "io/imports/step-simplification/stepGraph/Entity_Sh_Rep_Rel.h"

Entity_Sh_Rep_Rel::Entity_Sh_Rep_Rel(unsigned int _id, const std::string& _name, const std::string& _arguments)
        : Entity(_id, _name, _arguments)
{}

bool Entity_Sh_Rep_Rel::canUpdateReference(Entity *entityChild, std::list<Entity*> &entitiesToDelete) {
    if (getNbChildren() != 2) return false;
    entitiesToDelete.push_back(this);
    return true;
}

void Entity_Sh_Rep_Rel::updateChildReference(Entity *entityChild, StepGraph& graph) {
    graph.deleteEntity(*this) ;
}