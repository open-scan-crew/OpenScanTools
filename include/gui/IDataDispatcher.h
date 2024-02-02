#ifndef I_DATA_DISPATCHER_H
#define I_DATA_DISPATCHER_H

#include "gui/GuiData/IGuiData.h"
#include "gui/IPanel.h"

class AControl;
class IControlListener;

/*! \class IDataDispatcher
 * \brief Interface to controll all the data imited from the Controller
 *
 * The job of this interface is double:
 *  - Send the data imited by the controller to the Observers registered
 *  - Send back the controls to the controller.
 */
class IDataDispatcher
{
public:

	virtual void updateInformation(IGuiData *value, IPanel* owner = nullptr) = 0;

    virtual void unregisterObserver(IPanel* panel) = 0;
    virtual void registerObserverOnKey(IPanel* panel, guiDType type) = 0;
    virtual void unregisterObserverOnKey(IPanel* panel, guiDType type) = 0;

    virtual void InitializeControlListener(IControlListener *listener) = 0;
    virtual void sendControl(AControl *control) = 0;
};

#endif // !I_DATA_DISPATCHER_H