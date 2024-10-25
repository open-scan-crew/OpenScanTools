#include "controller/controls/ControlFunctionPiping.h"
#include "controller/Controller.h"

#include <unordered_set>

namespace control::function::piping
{
    /** ConnectToPiping **/

    ConnectToPiping::ConnectToPiping(SafePtr<PipingNode> piping, const std::unordered_set<SafePtr<AGraphNode>>& elements)
        : m_piping(piping)
        , m_elements(elements)
    {}

    ConnectToPiping::~ConnectToPiping()
    {}

    void ConnectToPiping::doFunction(Controller& controller)
    {
    }

    bool ConnectToPiping::canUndo() const
    {
        return false;
    }

    void ConnectToPiping::undoFunction(Controller& controller)
    {
    }

    ControlType ConnectToPiping::getType() const
    {
        return ControlType();
    }
}