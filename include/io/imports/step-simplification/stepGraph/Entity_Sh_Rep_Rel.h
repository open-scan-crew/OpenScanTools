#ifndef ENTITY_SH_REP_REL_H
#define ENTITY_SH_REP_REL_H

#include "Entity.h"
#include "StepGraph.h"

#include <list>

/**
 * Entity SHAPE_REPRESENTATION_RELATIONSHIP
 */
class Entity_Sh_Rep_Rel : public Entity {

public:
    Entity_Sh_Rep_Rel(unsigned int _id, const std::string& _name, const std::string& _arguments);

    bool canUpdateReference(Entity *entityChild, std::list<Entity*> &entitiesToDelete) override;
    void updateChildReference(Entity *entityChild, StepGraph& graph) override;
};

#endif // ENTITY_SH_REP_REL_H