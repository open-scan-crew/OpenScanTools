#ifndef DIALOG_RECENT_PROJECT_H_
#define DIALOG_RECENT_PROJECT_H_

#include <QtWidgets/QDialog>
#include <QtGui/QStandardItemModel>
#include "gui/Dialog/ADialog.h"
#include "ui_DialogRecentProjects.h"

#include <filesystem>

class DialogRecentProjects : public ADialog
{
	Q_OBJECT

public:
	explicit DialogRecentProjects(IDataDispatcher &dataDispacher, QWidget *parent = 0);
	~DialogRecentProjects();

	// from IPanel
	void informData(IGuiData *keyValue) override;

	void receiveProjectList(IGuiData *data);

public slots:
	void projectSelect(const QModelIndex& index);
	void FinishDialog();
private:
	Ui::DialogRecentProjects m_ui;
	std::vector<std::filesystem::path> m_pathsToProjects;

	QMetaObject::Connection ListConnect;
};

#endif // !DIALOG_RECENT_PROJECT_H_