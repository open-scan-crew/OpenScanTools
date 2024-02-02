#ifndef INFODELETIONPIECE_H
#define INFODELETIONPIECE_H

#include <list>
#include <map>

#include "../stepGraph/Entity.h"

class InfoDeletionPiece {

private :
    Entity * entRoot;
    std::map<unsigned int, std::string> prevArgsParents;
    std::list<Entity *> deletedParentEntities;

public:
    explicit InfoDeletionPiece(Entity *, std::map<unsigned int, std::string>, std::list<Entity*>);
    inline std::map<unsigned int, std::string> getPreviousArgsParents() { return prevArgsParents; }
    inline Entity* getEntityRoot() { return entRoot; }
    inline std::list<Entity *> getDeletedParentsEntities() { return deletedParentEntities; }

};

#endif // INFODELETIONPIECE_H
