#ifndef TOOLBAR_IMPORTSCANTRA_H
#define TOOLBAR_IMPORTSCANTRA_H

#include <QtWidgets/QWidget>
#include "ui_toolbar_importScantra.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ToolBarImportScantra : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarImportScantra(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

	void informData(IGuiData *data) override;

private:
    ~ToolBarImportScantra();
	void onProjectLoad(IGuiData* data);

public slots:
	void slotImportScanTra();

private:
	Ui::ToolBarImportScantra m_ui;
    IDataDispatcher &m_dataDispatcher;
	QString m_openPath;
};

#endif // TOOLBAR_IMPORTOBJECTS_H

