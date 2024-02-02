#include "DialogPointCloudObjectCreation.h"

//#include "gui/Texts.hpp"
#include "io/FileUtils.h"
#include "controller/controls/ControlApplication.h"
#include "controller/controls/ControlPCObject.h"
#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataClipping.h"
#include "controller/messages/PointCloudObjectCreationParametersMessage.h"

#include <QtWidgets/QFileDialog>
#include <QtCore/QStandardPaths>


DialogPointCloudObjectCreation::DialogPointCloudObjectCreation(IDataDispatcher& dataDispatcher, QWidget *parent)
    : ADialog(dataDispatcher, parent)
{
    ui.setupUi(this);
    setModal(true);
    ui.label_density_unit->setVisible(false);
    ui.lineEdit_density->setVisible(false);
    ui.label_density->setVisible(false);

    connect(ui.pushButton_import, &QPushButton::clicked, this, &DialogPointCloudObjectCreation::startExport);
    connect(ui.pushButton_cancel, &QPushButton::clicked, this, &DialogPointCloudObjectCreation::cancelExport);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::pcoCreationParametersDisplay);

    ui.lineEdit_file->setRegExp(QRegExpEdit::AlphaNumericWithSeparator);

}

DialogPointCloudObjectCreation::~DialogPointCloudObjectCreation()
{
    m_dataDispatcher.unregisterObserver(this);
}

void DialogPointCloudObjectCreation::informData(IGuiData *data)
{
    switch (data->getType())
    {
    case guiDType::pcoCreationParametersDisplay:
        m_parameters = (static_cast<GuiDataPointCloudObjectDialogDisplay*>(data))->m_parameters;
        ui.lineEdit_file->setText(QString::fromStdWString(m_parameters.fileName));
        ui.lineEdit_density->setText(QString::number(m_parameters.pointDensity));
        show();
        break;  
    }
}

void DialogPointCloudObjectCreation::startExport()
{
    m_parameters.fileName = ui.lineEdit_file->text().toStdWString();
    m_parameters.pointDensity = ui.lineEdit_density->getValue();
 
    PointCloudObjectCreationParametersMessage* message(new PointCloudObjectCreationParametersMessage(m_parameters));
    m_dataDispatcher.sendControl(new control::function::ForwardMessage(message));

    hide();
}

void DialogPointCloudObjectCreation::closeEvent(QCloseEvent* event)
{
    cancelExport();
}

void DialogPointCloudObjectCreation::cancelExport()
{
    m_dataDispatcher.sendControl(new control::function::Abort());
    hide();
}