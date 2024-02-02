#ifndef DIALOG_POINT_CLOUD_OBJECT_CREATION_H
#define DIALOG_POINT_CLOUD_OBJECT_CREATION_H

#include "ui_DialogPointCloudObjectCreation.h"
#include "gui/Dialog/ADialog.h"
#include "io/exports/ExportParameters.hpp"

class DialogPointCloudObjectCreation : public ADialog
{
    Q_OBJECT

public:
    DialogPointCloudObjectCreation(IDataDispatcher& dataDispatcher, QWidget *parent = Q_NULLPTR);
    ~DialogPointCloudObjectCreation();

    void informData(IGuiData *data) override;
    void closeEvent(QCloseEvent* event);

private:
    void cancelExport();
    void startExport();

private:
    Ui::DialogPointCloudObjectCreation ui;
    PointCloudObjectParameters m_parameters;
};

#endif