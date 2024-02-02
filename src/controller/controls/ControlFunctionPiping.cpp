#include "controller/controls/ControlFunctionPiping.h"
#include "controller/controls/ControlSpecial.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "gui/GuiData/GuiDataTree.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataTree.h"

#include <unordered_set>
#include "utils/Logger.h"

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