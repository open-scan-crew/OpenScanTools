#ifndef DELETE_TYPE_DIALOG_H_
#define DELETE_TYPE_DIALOG_H_

#include <QtWidgets/QListView>
#include <QtGui/QStandardItemModel>
#include <QDialog>
#include "utils/Logger.h"
#include "ui_DialogDeleteScanTypeSelection.h"
#include "models/data/Data.h"

#include "gui/IDataDispatcher.h"

#include <unordered_set>

#define GUILOG Logger::log(LoggerMode::GuiLog)

class AGraphNode;

/*! Classe qui gère le Dialog de demande d'information sur la méthode de suppression de Scan à l'utilisateur

	Utilisé à chaque fois que l'utilisateur appelle la suppression d'un Cluster contenant un Scan ou d'un Scan*/
class DialogDeleteScanTypeSelect : public QDialog
{
	Q_OBJECT

public:
	explicit DialogDeleteScanTypeSelect(IDataDispatcher& dataDispatcher,const std::unordered_map<SafePtr<AGraphNode>, std::pair<QString, QString>>& importantData,const std::unordered_set<SafePtr<AGraphNode>>& otherData, QWidget *parent = 0);
	~DialogDeleteScanTypeSelect();

	bool getWaitUser();

public slots:

	void HardDelete();
	void SoftDelete();
	void Cancel();

private:
	std::unordered_map<SafePtr<AGraphNode>, std::pair<QString, QString>> m_importantData;
	std::unordered_set<SafePtr<AGraphNode>> m_setImportantData;
	std::unordered_set<SafePtr<AGraphNode>> m_otherData;
	IDataDispatcher& m_dataDispatcher;
	bool m_waitUser;

	Ui::DeleteScanDialog m_ui;
};

#endif // !DELETE_TYPE_DIALOG_H_