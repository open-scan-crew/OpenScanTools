#ifndef DIALOG_IMPORT_STEP_SIMPLIFICATION_H
#define DIALOG_IMPORT_STEP_SIMPLIFICATION_H

#include "gui/Dialog/ADialog.h"
#include "io/MeshObjectTypes.h"
#include "ui_DialogImportStepSimplification.h"

enum class Selection;

class DialogImportStepSimplification : public ADialog
{
    Q_OBJECT

public:
	DialogImportStepSimplification(IDataDispatcher& dataDispatcher, QWidget *parent);
    ~DialogImportStepSimplification();

    void informData(IGuiData *data) override;

	void setImportInputData(const FileInputData& data);

private:
    void onCancel();
	void onSimplify();

	void onInputBrowser();
	void onOutputBrowser();

private:
	void updateKeepPercent(double keep);


private:
    Ui::DialogImportStepSimplification m_ui;
	FileInputData					   m_data;
	QString							   m_openPath;

	bool							   m_importAfter;
};

#endif
