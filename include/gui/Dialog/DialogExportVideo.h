#ifndef DIALOG_EXPORT_VIDEO_H
#define DIALOG_EXPORT_VIDEO_H

#include "ui_DialogExportVideo.h"
#include "gui/Dialog/ADialog.h"
#include "io/exports/ExportParameters.hpp"

#include <unordered_set>

#include "models/Types.hpp"

class DialogExportVideo : public ADialog
{
    Q_OBJECT

public:
    DialogExportVideo(IDataDispatcher& dataDispatcher, QWidget *parent, float guiScale);
    ~DialogExportVideo();


    void informData(IGuiData *data) override;
    void closeEvent(QCloseEvent* event);

private:
    void onAnimationModeSelection();

    void onViewpoint1Click();
    void onViewpoint2Click();

    void onSelectOutFolder();
	void onSelectOutFile();

    void startGeneration();
    void cancelGeneration();

private:
    Ui::DialogExportVideo m_ui;
    QString m_openPath;

    int m_viewpointToEdit = -1;
	VideoExportParameters m_parameters;

};

#endif