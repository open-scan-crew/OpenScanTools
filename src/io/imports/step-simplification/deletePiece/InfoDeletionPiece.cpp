#include "io/imports/step-simplification/deletePiece/InfoDeletionPiece.h"

InfoDeletionPiece::InfoDeletionPiece(Entity * r, std::map<unsigned int, std::string> prevArgs, std::list<Entity*> _deletedParentEntities) {
    entRoot = r;
    prevArgsParents = prevArgs;
    deletedParentEntities = _deletedParentEntities;
}
