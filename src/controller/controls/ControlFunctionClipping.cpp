#include "controller/controls/ControlFunctionClipping.h"
#include "controller/controls/ControlFunction.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/ControlListener.h"
#include "gui/GuiData/GuiDataTree.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "pointCloudEngine/TlScanOverseer.h"

#include "gui/Texts.hpp"
#include "utils/Logger.h"

#include "models/graph/GraphManager.hxx"
#include "models/graph/BoxNode.h"

namespace control::function::clipping
{
    /*
    ** ActivateCreateLocal
    */

    ActivateCreateLocal::ActivateCreateLocal()
    {
    }

    ActivateCreateLocal::~ActivateCreateLocal()
    {
    }

    void ActivateCreateLocal::doFunction(Controller& controller)
    {
        controller.getFunctionManager().launchFunction(controller, ContextType::clippingBoxCreation);
        CONTROLLOG << "control::function::clipping::ActivateCreateLocal" << LOGENDL;
    }

    bool ActivateCreateLocal::canUndo() const
    {
        return (false);
    }

    void ActivateCreateLocal::undoFunction(Controller& controller)
    {
    }

    ControlType ActivateCreateLocal::getType() const
    {
        return (ControlType::activateCreateLocalFunctionClipping);
    }

    /*
    ** ActivateCreateAttached
    */

    ActivateCreateAttached::ActivateCreateAttached()
    {
    }

    ActivateCreateAttached::~ActivateCreateAttached()
    {
    }

    void ActivateCreateAttached::doFunction(Controller& controller)
    {
        controller.getFunctionManager().launchFunction(controller, ContextType::clippingBoxAttached3Points);
        CONTROLLOG << "control::function::clipping::ActivateCreateAttached" << LOGENDL;
    }

    bool ActivateCreateAttached::canUndo() const
    {
        return (false);
    }

    void ActivateCreateAttached::undoFunction(Controller& controller)
    {
    }

    ControlType ActivateCreateAttached::getType() const
    {
        return (ControlType::activateCreateAttachedBox3Points);
    }

    /*
    ** ActivateCreateAttached2Points
    */

    ActivateCreateAttached2Points::ActivateCreateAttached2Points()
    {
    }

    ActivateCreateAttached2Points::~ActivateCreateAttached2Points()
    {
    }

    void ActivateCreateAttached2Points::doFunction(Controller& controller)
    {
        controller.getFunctionManager().launchFunction(controller, ContextType::clippingBoxAttached2Points);
        CONTROLLOG << "control::function::clipping::ActivateCreateAttached2Points" << LOGENDL;
    }

    bool ActivateCreateAttached2Points::canUndo() const
    {
        return false;
    }

    void ActivateCreateAttached2Points::undoFunction(Controller& controller)
    {
    }

    ControlType ActivateCreateAttached2Points::getType() const
    {
        return ControlType::activateCreateAttachedBox2Points;
    }

    /*
    ** CreateGlobal
    */

    CreateGlobal::CreateGlobal()
    {
    }

    CreateGlobal::~CreateGlobal()
    {
    }

    void CreateGlobal::doFunction(Controller& controller)
    {
        GraphManager& graphManager = controller.getGraphManager();

        controller.updateInfo(new GuiDataActivatedFunctions(ContextType::none));
        controller.getFunctionManager().abort(controller);

        // Compute the bounding box based on the current scan transformation
        TlScanOverseer::getInstance().setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(xg::Guid(), true, true));
        BoundingBoxD projectBoundingBox = TlScanOverseer::getInstance().getActiveBoundingBox();

        SafePtr<BoxNode> box = make_safe<BoxNode>(true);
        {
            WritePtr<BoxNode> wBox = box.get();
            if (!wBox)
                return;

            wBox->setDefaultData(controller);
            wBox->setSize(projectBoundingBox.size());
            wBox->setPosition(projectBoundingBox.center());
        }

        controller.getControlListener()->notifyUIControl(new control::function::AddNodes(box));

        CONTROLLOG << "control::function::clipping::CreateGlobal" << LOGENDL;
    }

    ControlType CreateGlobal::getType() const
    {
        return (ControlType::createGlobalFunctionClipping);
    }

    /*
    ** SetAlignementValue
    */

    SetAlignementValue::SetAlignementValue(const double& angleZ)
        : m_angleZ(angleZ)
    {}

    SetAlignementValue::~SetAlignementValue()
    {}

    void SetAlignementValue::doFunction(Controller& controller)
    {
        CONTROLLOG << "control::function::clipping::SetAlignementValue " << m_angleZ << LOGENDL;
        controller.getContext().setClippingAngleZValue(m_angleZ);
    }

    ControlType SetAlignementValue::getType() const
    {
        return (ControlType::setClippingAlignementValue);
    }

    /*
    ** SetDefaultSize
    */

    SetDefaultSize::SetDefaultSize(glm::vec3 size)
    {
        m_size = size;
    }

    SetDefaultSize::~SetDefaultSize()
    {
    }

    void SetDefaultSize::doFunction(Controller& controller)
    {
        controller.getContext().setClippingSize(m_size);
    }

    ControlType SetDefaultSize::getType() const
    {
        return (ControlType::setDefaultSizeFunctionClipping);
    }

    /*
    ** SetDefaultOffset
    */

    SetDefaultOffset::SetDefaultOffset(ClippingBoxOffset offset)
    {
        m_offset = offset;
    }

    SetDefaultOffset::~SetDefaultOffset()
    {
    }

    void SetDefaultOffset::doFunction(Controller& controller)
    {
        controller.getContext().setClippingOffset(m_offset);
    }

    ControlType SetDefaultOffset::getType() const
    {
        return (ControlType::setDefaultOffsetFunctionClipping);
    }

    /*
    ** ActivateDuplicate
    */

    ActivateDuplicate::ActivateDuplicate()
    {
    }

    ActivateDuplicate::~ActivateDuplicate()
    {
    }

    void ActivateDuplicate::doFunction(Controller& controller)
    {
        controller.getFunctionManager().launchFunction(controller, ContextType::boxDuplication);
        CONTROLLOG << "control::function::clipping::ActivateDuplicate" << LOGENDL;
    }

    ControlType ActivateDuplicate::getType() const
    {
        return (ControlType::activateDuplicateFunctionClipping);
    }

}