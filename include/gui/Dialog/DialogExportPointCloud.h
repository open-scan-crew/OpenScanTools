#ifndef DIALOG_EXPORT_POINT_CLOUD_H
#define DIALOG_EXPORT_POINT_CLOUD_H

#include "ui_DialogExportPointCloud.h"
#include "gui/Dialog/ADialog.h"
#include "io/exports/ExportParameters.hpp"

#include <unordered_set>

class AClippingNode;

class DialogExportPointCloud : public ADialog
{
    Q_OBJECT

public:
    DialogExportPointCloud(IDataDispatcher& dataDispatcher, QWidget *parent);
    ~DialogExportPointCloud();

    void informData(IGuiData *data) override;
    void closeEvent(QCloseEvent* event);

public:
    void onSelectSource(int index);
    void onSelectMethod(int index);
    void onSelectFormat(int index);

    void onSelectOutFolder();
    void onSelectTempFolder();
    void onSelectOutFile();

    void startExport();
    void cancelExport();

private:
    void setPointCloudStatus(ObjectStatusFilter status);

    void translateUI();
    void refreshUI();
    void initComboBoxPointCloud();
    void refreshDialogTitle();
    void refreshClippingNames();
    void refreshSourceOption();
    void refreshMethodOption();
    void refreshShowFileName();
    void refreshFormatOptions();

private:
    Ui::DialogExportPointCloud m_ui;
    bool m_showClippingOptions;
    bool m_showGridOptions;
    bool m_showMergeOption;
    ExportPointCloudSource pc_source_;

    std::unordered_set<SafePtr<AClippingNode>> m_clippings;
    QString m_openPath;

    std::unordered_map<ExportClippingFilter, QString> m_exportSourceTexts;
    std::unordered_map<ExportPointCloudMerging, QString> m_exportMethodTexts;
};

#endif