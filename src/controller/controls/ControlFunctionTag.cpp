#include "controller/controls/ControlFunctionTag.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"
#include "controller/functionSystem/FunctionManager.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataTag.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/GuiData/GuiDataTree.h"
#include "utils/OpenScanToolsModelEssentials_impl.hpp"
#include "utils/Logger.h"
#include "gui/Texts.hpp"
#include "magic_enum/magic_enum.hpp"

namespace control::function::tag
{
    /*
    ** ActivateCreate
    */

    ActivateCreate::ActivateCreate()
    {
    }

    ActivateCreate::~ActivateCreate()
    {
    }

    void ActivateCreate::doFunction(Controller& controller)
    {
        controller.getFunctionManager().launchFunction(controller, ContextType::tagCreation);
        CONTROLLOG << "control::function::tag::ActivateCreate" << LOGENDL;
    }

    bool ActivateCreate::canUndo() const
    {
        return (false);
    }

    void ActivateCreate::undoFunction(Controller& controller)
    {
    }

    ControlType ActivateCreate::getType() const
    {
        return (ControlType::activateCreateFunctionTag);
    }

    /*
    ** ActivateMove
    */

    ActivateMove::ActivateMove()
    {
    }

    ActivateMove::~ActivateMove()
    {
    }

    void ActivateMove::doFunction(Controller& controller)
    {
        controller.getFunctionManager().launchFunction(controller, ContextType::tagMove);
        CONTROLLOG << "control::function::tag::ActivateMove" << LOGENDL;
    }

    bool ActivateMove::canUndo() const
    {
        return (false);
    }

    void ActivateMove::undoFunction(Controller& controller)
    {
    }

    ControlType ActivateMove::getType() const
    {
        return (ControlType::activateMoveFunctionTag);
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
        controller.getFunctionManager().launchFunction(controller, ContextType::tagDuplication);
        CONTROLLOG << "control::function::tag::ActivateDuplicate" << LOGENDL;
    }

    bool ActivateDuplicate::canUndo() const
    {
        return (false);
    }

    void ActivateDuplicate::undoFunction(Controller& controller)
    {
    }

    ControlType ActivateDuplicate::getType() const
    {
        return (ControlType::activateDuplicateFunctionTag);
    }

    /*
    ** setCurrentTemplate
    */

    SetCurrentTagTemplate::SetCurrentTagTemplate(SafePtr<sma::TagTemplate> temp)
    {
        m_newTemplate = temp;
    }

    SetCurrentTagTemplate::~SetCurrentTagTemplate()
    {
    }

    void SetCurrentTagTemplate::doFunction(Controller& controller)
    {
        controller.getContext().setCurrentTemplate(m_newTemplate);

        ReadPtr<sma::TagTemplate> rTemp = m_newTemplate.cget();
        std::wstring tempName = L"NO DATA";
        if (rTemp)
            tempName = rTemp->getName();
        CONTROLLOG << "control::function::tag::SetCurrentTemplate " << tempName << LOGENDL;
    }

    bool SetCurrentTagTemplate::canUndo() const
    {
        return (false);
    }

    void SetCurrentTagTemplate::undoFunction(Controller& controller)
    {
    }

    ControlType SetCurrentTagTemplate::getType() const
    {
        return (ControlType::setCurrentTagTemplate);
    }

    SetCurrentTagTemplate::SetCurrentTagTemplate() { }

    /*
    ** SetDefaultIcon
    */

    SetDefaultIcon::SetDefaultIcon(scs::MarkerIcon newIcon)
    {
        _newIcon = newIcon;
    }

    SetDefaultIcon::~SetDefaultIcon()
    {
    }

    void SetDefaultIcon::doFunction(Controller& controller)
    {
        CONTROLLOG << "control::function::tag::SetDefaultIcon " << magic_enum::enum_name(_newIcon) << LOGENDL;
        controller.getContext().setActiveIcon(_newIcon);
        controller.updateInfo(new GuiDataTagDefaultIcon(_newIcon));
    }

    bool SetDefaultIcon::canUndo() const
    {
        return (false);
    }

    void SetDefaultIcon::undoFunction(Controller& controller)
    { }

    ControlType SetDefaultIcon::getType() const
    {
        return (ControlType::setDefaultIconFunctionTag);
    }

    SetDefaultIcon::SetDefaultIcon() { }

    /*
    ** DuplicateTag
    */

    DuplicateTag::DuplicateTag(SafePtr<TagNode> srcTag, SafePtr<TagNode> dstTag)
    {
        m_srcTag = srcTag;
        m_dstTag = dstTag;
        m_canUndo = false;
    }

    DuplicateTag::~DuplicateTag()
    {
    }

    void DuplicateTag::doFunction(Controller& controller)
    {
        CONTROLLOG << "control::function::tag::DuplicateTag do " << LOGENDL;
        OpenScanToolsGraphManager& graphManager = controller.getOpenScanToolsGraphManager();

        ReadPtr<TagNode> srcTag = m_srcTag.cget();
        WritePtr<TagNode> dstTag = m_dstTag.get();
        if (!srcTag || !dstTag)
            return;

        m_oldData = *&dstTag;
        m_oldClippingData = *&dstTag;
        m_oldTagData = *&dstTag;

        dstTag->copyUIData(*&srcTag, false);
        dstTag->copyClippingData(*&srcTag);
        dstTag->copyTagData(*&srcTag);

        // Actualize the tag for the viewport
        //controller.changeSelection({ m_dstTag });
        
        m_canUndo = true;
    }

    bool DuplicateTag::canUndo() const
    {
        return true;
    }

    void DuplicateTag::undoFunction(Controller& controller)
    {
        controller.changeSelection({});

        WritePtr<TagNode> dstTag = m_dstTag.get();

        dstTag->copyUIData(m_oldData, false);
        dstTag->copyClippingData(m_oldClippingData);
        dstTag->copyTagData(m_oldTagData);

        //controller.actualizeOnId(m_dstTag);

        CONTROLLOG << "control::function::tag::CreateDuplicateTagTag undo " << LOGENDL;
    }

    ControlType DuplicateTag::getType() const
    {
        return (ControlType::duplicateTag);
    }
}