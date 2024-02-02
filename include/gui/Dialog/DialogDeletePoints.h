#ifndef DIALOG_DELETE_POINTS_H
#define DIALOG_DELETE_POINTS_H

#include "ui_DialogDeletePoints.h"
#include "gui/Dialog/ADialog.h"
#include "io/exports/ExportParameters.hpp"

class AClippingNode;

class DialogDeletePoints : public ADialog
{
    Q_OBJECT

public:
    DialogDeletePoints(IDataDispatcher& dataDispatcher, QWidget *parent);
    ~DialogDeletePoints();

    void informData(IGuiData *data) override;

public:
    void closeEvent(QCloseEvent* event);
    void onSelectSource(int index);

    void startDeletion();
    void cancelDeletion();

private:
    void translateUI();
    void refreshUI();
    void refreshClippingNames();
    void initSourceOption();

private:
    Ui::DialogDeletePoints m_ui;
    ExportClippingFilter m_clippingFilter;

    std::vector<SafePtr<AClippingNode>> m_clippings;

    std::unordered_map<ExportClippingFilter, QString> m_exportSourceTexts;
};

#endif
