#ifndef A_DUPLICATION_H
#define A_DUPLICATION_H

#include "models/3d/DuplicationTypes.h"
#include <glm/glm.hpp>

class TransformationModule;
class Controller;


class ADuplication
{
public:
    ADuplication(DuplicationMode mode);
    ~ADuplication();

protected:
    void setObjectParameters(Controller& controller, TransformationModule& transfo, const glm::dvec3& position, const glm::dvec3& scale);

protected:
    DuplicationMode m_mode;
};

#endif
