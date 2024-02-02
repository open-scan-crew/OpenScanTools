#include "controller/controls/ControlPicking.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/FullClickMessage.h"
#include "gui/GuiData/GuiDataMeasure.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "utils/OpenScanToolsModelEssentials_impl.hpp"
#include "utils/Logger.h"

#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"

namespace control::picking
{
    //
    // Click
    //

    Click::Click(const ClickInfo& info)
		: m_clickInfo(info)
    {}

    Click::~Click()
    {}

    void Click::doFunction(Controller& controller)
    {
        CONTROLLOG << "Click detected : " << m_clickInfo.picking << Logger::endl;
        // TODO(robin) - Vérifier que les points sont bien envoyés à 'PropertyClippingSettings' et 'PropertyUserOrientation'
        if (controller.getFunctionManager().isActiveContext() == ContextType::none)
        {
            if (m_clickInfo.ctrl)
            {
                OpenScanToolsGraphManager& graphManager = controller.getOpenScanToolsGraphManager();
                std::unordered_set<SafePtr<AGraphNode>> selected(graphManager.getSelectedNodes());

                {
                    ReadPtr<AGraphNode> node = m_clickInfo.hover.cget();
                    if (node) {
                        if (node->isSelected())
                            selected.erase(m_clickInfo.hover);
                        else
                            selected.insert(m_clickInfo.hover);
                    }
                }

                controller.changeSelection(selected);
            }
            else
            {
                if(m_clickInfo.hover)
                    controller.changeSelection({ m_clickInfo.hover });
            }

			controller.updateInfo(new GuiDataPoint(m_clickInfo.picking));
			controller.updateInfo(new GuiDataRenderTargetClick());
		}
        else
        {
            FullClickMessage message(m_clickInfo);
            controller.getFunctionManager().feedMessage(controller, &message);
        }
    }

    bool Click::canUndo() const
    {
        return (false);
    }

    void Click::undoFunction(Controller& controller)
    { }

    ControlType Click::getType() const
    {
        return (ControlType::clickPicking);
    }

    FindScanFromPick::FindScanFromPick()
    {
    }

    FindScanFromPick::~FindScanFromPick()
    {
    }

    void FindScanFromPick::doFunction(Controller& controller)
    {
        controller.getFunctionManager().launchFunction(controller, ContextType::findScan);
    }

    bool FindScanFromPick::canUndo() const
    {
        return false;
    }

    void FindScanFromPick::undoFunction(Controller& controller)
    {}

    ControlType FindScanFromPick::getType() const
    {
        return ControlType::findScanFromPicking;
    }

}