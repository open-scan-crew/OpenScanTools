#include "gui/Dialog/DialogImageHD.h"


DialogImageHD::DialogImageHD(IDataDispatcher& dataDispatcher, QWidget* parent)
    : ADialog(dataDispatcher, parent)
{
    m_ui.setupUi(this);
    translateUI();

    connect(m_ui.okButton, &QPushButton::released, this, &DialogImageHD::generateImage);
    connect(m_ui.cancelButton, &QPushButton::released, this, &DialogImageHD::cancel);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::deletePointsDialogDisplay);
    adjustSize();
}

DialogImageHD::~DialogImageHD()
{
    m_dataDispatcher.unregisterObserver(this);
}

void DialogImageHD::informData(IGuiData* data)
{
    switch (data->getType())
    {
    case guiDType::deletePointsDialogDisplay:
    {
        break;
    }
    break;
    }
}

void DialogImageHD::generateImage()
{

}

void DialogImageHD::cancel()
{

}

void DialogImageHD::translateUI()
{

}
