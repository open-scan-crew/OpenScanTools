#ifndef DIALOG_STATISTICAL_OUTLIER_FILTER_H
#define DIALOG_STATISTICAL_OUTLIER_FILTER_H

#include "gui/Dialog/ADialog.h"
#include "ui_DialogStatisticalOutlierFilter.h"

#include "controller/messages/StatisticalOutlierFilterMessage.h"

class DialogStatisticalOutlierFilter : public ADialog
{
    Q_OBJECT
public:
    DialogStatisticalOutlierFilter(IDataDispatcher& dataDispatcher, QWidget* parent);
    ~DialogStatisticalOutlierFilter();

    void informData(IGuiData* data) override;

private slots:
    void startFiltering();
    void cancelFiltering();

private:
    void translateUI();
    void refreshUI();

    Ui::DialogStatisticalOutlierFilter m_ui;
    OutlierFilterMode m_mode = OutlierFilterMode::Separate;
    std::wstring m_outputFolder;
    QString m_openPath;
};

#endif
