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
    enum class OutlierPreset
    {
        Low,
        Mid,
        High
    };

    void translateUI();
    void refreshUI();
    void applyPreset(OutlierPreset preset);
    void syncUiFromValues();

    Ui::DialogStatisticalOutlierFilter m_ui;
    OutlierFilterMode m_mode = OutlierFilterMode::Separate;
    OutlierPreset m_preset = OutlierPreset::Mid;
    int m_kNeighbors = 20;
    double m_nSigma = 1.0;
    int m_samplingPercent = 2;
    double m_beta = 4.0;
    std::wstring m_outputFolder;
    QString m_openPath;
};

#endif
