#ifndef HYPERLINKADD_DIALOG_H_
#define HYPERLINKADD_DIALOG_H_

#include "gui/Dialog/ADialog.h"
#include "ui_AddHyperlinkDialog.h"
#include "models/data/Data.h"

#include <filesystem>

class AObjectNode;

class HyperlinkAddDialog : public ADialog
{
	Q_OBJECT

public:
	explicit HyperlinkAddDialog(IDataDispatcher& dataDispacher, QWidget *parent = 0);
	~HyperlinkAddDialog();

	void closeEvent(QCloseEvent* event) override;
	virtual void informData(IGuiData* keyValue) override;

signals:
	void onCreatedLink(std::wstring hyperlink, std::wstring name);


public slots:

	void enableURL();
	void enableFile();
	void clickfileSearch();
	void updateFilePath();

	void acceptCreation();
	void cancelCreation();

private:
	Ui::AddHyperlinkDialog *ui;
	std::filesystem::path m_projectFolderPath;
	std::filesystem::path m_absPath;
	QString m_openPath;
};

#endif // !HYPERLINKADD_DIALOG_H_