#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtCore/QStandardPaths>
#include "gui/Dialog/TemplateEditorDialog.h"
#include "gui/widgets/TempFieldNode.h"
#include "controller/controls/ControlTemplateEdit.h"
#include "gui/GuiData/GuiDataTemplate.h"

#include "gui/Texts.hpp"
#include "gui/texts/MarkerTexts.hpp"
#include "utils/Logger.h"

#include "magic_enum/magic_enum.hpp"

#include <cctype>
#include <QHeaderView>
#include <QSortFilterProxyModel>

TemplateEditorDialog::TemplateEditorDialog(IDataDispatcher &dataDispacher, QWidget *parent)
	: ADialog(dataDispacher, parent)
	, ui(new Ui::TemplateEditDialog)
	, m_sortedColumn(0)
	, m_sortOrder(Qt::AscendingOrder)
	, m_fieldTypeToString({ {sma::tFieldType::date, TEXT_TEMPLATEEDITOR_DATE },
							{sma::tFieldType::list, TEXT_TEMPLATEEDITOR_LIST },
							{sma::tFieldType::number, TEXT_TEMPLATEEDITOR_NUMBER },
							{sma::tFieldType::string, TEXT_TEMPLATEEDITOR_STRING }, })
{
	ui->setupUi(this);
	GUI_LOG << "create TemplateEditorDialog" << LOGENDL;

	m_defaultValueManagers.insert(std::pair<sma::tFieldType, std::pair<defaultValueWidgetGenerator, defaultValueGatherer>>(sma::tFieldType::string,
		std::pair<defaultValueWidgetGenerator, defaultValueGatherer>(&TemplateEditorDialog::generateStringWidget, &TemplateEditorDialog::gatherStringWidget)));
	m_defaultValueManagers.insert(std::pair<sma::tFieldType, std::pair<defaultValueWidgetGenerator, defaultValueGatherer>>(sma::tFieldType::multiLine,
		std::pair<defaultValueWidgetGenerator, defaultValueGatherer>(&TemplateEditorDialog::generateMultiLineWidget, &TemplateEditorDialog::gatherMultiLineWidget)));
	m_defaultValueManagers.insert(std::pair<sma::tFieldType, std::pair<defaultValueWidgetGenerator, defaultValueGatherer>>(sma::tFieldType::date,
		std::pair<defaultValueWidgetGenerator, defaultValueGatherer>(&TemplateEditorDialog::generateDateWidget, &TemplateEditorDialog::gatherDateWidget)));
	m_defaultValueManagers.insert(std::pair<sma::tFieldType, std::pair<defaultValueWidgetGenerator, defaultValueGatherer>>(sma::tFieldType::number,
		std::pair<defaultValueWidgetGenerator, defaultValueGatherer>(&TemplateEditorDialog::generateNumberWidget, &TemplateEditorDialog::gatherNumberWidget)));
	m_defaultValueManagers.insert(std::pair<sma::tFieldType, std::pair<defaultValueWidgetGenerator, defaultValueGatherer>>(sma::tFieldType::hyperlink,
		std::pair<defaultValueWidgetGenerator, defaultValueGatherer>(&TemplateEditorDialog::generateHyperlinkWidget, &TemplateEditorDialog::gatherHyperlinkWidget)));
	m_defaultValueManagers.insert(std::pair<sma::tFieldType, std::pair<defaultValueWidgetGenerator, defaultValueGatherer>>(sma::tFieldType::list,
		std::pair<defaultValueWidgetGenerator, defaultValueGatherer>(&TemplateEditorDialog::generateListWidget, &TemplateEditorDialog::gatherListWidget)));
	m_defaultValueManagers.insert(std::pair<sma::tFieldType, std::pair<defaultValueWidgetGenerator, defaultValueGatherer>>(sma::tFieldType::none,
		std::pair<defaultValueWidgetGenerator, defaultValueGatherer>(&TemplateEditorDialog::generateNoneWidget, &TemplateEditorDialog::gatherNoneWidget)));

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendTagTemplate);

	ui->TemplateWidgetTable->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
	ui->TemplateWidgetTable->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
	ui->TemplateWidgetTable->setHorizontalHeaderItem(0, new QTableWidgetItem(TEXT_NAME));
	ui->TemplateWidgetTable->setHorizontalHeaderItem(1, new QTableWidgetItem(TEXT_TYPE));
	ui->TemplateWidgetTable->setHorizontalHeaderItem(2, new QTableWidgetItem(TEXT_REFERENCE));
	ui->TemplateWidgetTable->setHorizontalHeaderItem(3, new QTableWidgetItem(TEXT_DEFAULT_VALUE));
	ui->TemplateWidgetTable->horizontalHeader()->setVisible(true);
	ui->TemplateWidgetTable->verticalHeader()->setVisible(false);
	ui->TemplateWidgetTable->setShowGrid(false);

	m_typesVec.clear();
	m_typesVec.push_back(sma::tFieldType::string);
	m_typesVec.push_back(sma::tFieldType::number);
	m_typesVec.push_back(sma::tFieldType::list);
	m_typesVec.push_back(sma::tFieldType::date);
	//m_typesVec.push_back(sma::tFieldType::hyperlink);

	QObject::connect(ui->CancelBtn, &QPushButton::clicked, [this]() {this->hide(); });
	QObject::connect(ui->FinishBtn, &QPushButton::clicked, [this]() {this->hide(); });
	QObject::connect(ui->TemplateNameInfield, SIGNAL(editingFinished()), this, SLOT(renameTemplate()));
	
	QObject::connect(ui->TemplateWidgetTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &TemplateEditorDialog::setFilter);
	//ui->TemplateWidgetTable->setModel(new QSortFilterProxyModel());
}

TemplateEditorDialog::~TemplateEditorDialog()
{
	m_dataDispatcher.unregisterObserver(this);
}

void TemplateEditorDialog::informData(IGuiData *data)
{
	if (data->getType() == guiDType::sendTagTemplate)
		receiveTagTemplate(data);
}

void TemplateEditorDialog::receiveTagTemplate(IGuiData * data)
{
	GuiDataSendTagTemplate *lData = static_cast<GuiDataSendTagTemplate*>(data);
	ui->TemplateNameInfield->blockSignals(true);

	ui->TemplateWidgetTable->clear();
	m_template = lData->m_temp;

	ReadPtr<sma::TagTemplate> rTemp = m_template.cget();
	if (!rTemp)
		return;

	ui->TemplateNameInfield->setText(QString::fromStdWString(rTemp->getName()));
	int i = 0;

	std::vector<sma::tField> fields = rTemp->getFieldsCopy();
	ui->TemplateWidgetTable->setColumnCount(5);
	ui->TemplateWidgetTable->setRowCount((int)fields.size() + 1);

	QStringList m_TableHeader;
	m_TableHeader << TEXT_NAME << TEXT_TYPE << TEXT_REFERENCE << TEXT_DEFAULT_VALUE << tr("");
	ui->TemplateWidgetTable->setHorizontalHeaderLabels(m_TableHeader);
	

	//ui->TemplateWidgetTable->setStyleSheet("QTableView {selection-background-color: green;}");

	ui->TemplateWidgetTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	m_listLink.clear();
	//GUI_LOG << "GENERATE LISTS" << LOGENDL;
	for (SafePtr<UserList> list : lData->m_lists)
		m_listLink.push_back(list);

	//GUI_LOG << "GENERATE FIELDS" << LOGENDL;
	for (sma::tField& field : fields)
	{
		//GUI_LOG << "generate field " << *it << LOGENDL;

		QLineEdit *nameInfield = new QLineEdit(ui->TemplateWidgetTable);
		nameInfield->setMaxLength(20);
		QComboBox *comboType = new QComboBox(ui->TemplateWidgetTable);
		QComboBox *comboRef = new QComboBox(ui->TemplateWidgetTable);
		QPushButton *deletebtn = new QPushButton(TEXT_MARKER_DEFINITION_DELETE, ui->TemplateWidgetTable);

		nameInfield->setText(QString::fromStdWString(field.m_name));
		int typeIndex = 0;
		int savedType = 0;
		for (const sma::tFieldType& fieldType : m_typesVec)
		{
			comboType->addItem(m_fieldTypeToString.at(fieldType));
			if (fieldType == field.m_type)
				savedType = typeIndex;
			++typeIndex;
		}
		comboType->blockSignals(true);
		comboType->setCurrentIndex(savedType);
		comboType->blockSignals(false);

		comboRef->blockSignals(true);
		if (field.m_type != sma::tFieldType::list)
			comboRef->setDisabled(true);
		else
		{
			int savedRef = 0;
			int refIndex = 0;

			for (SafePtr<UserList> list : m_listLink)
			{
				ReadPtr<UserList> rList = list.cget();
				if (!rList)
					continue;

				comboRef->addItem(QString::fromStdWString(rList->getName()));
				if (field.m_fieldReference == list)
					savedRef = refIndex;
				++refIndex;
			}
			comboRef->setCurrentIndex(savedRef);
		}
		comboRef->blockSignals(false);

		QWidget *widg = (this->*(m_defaultValueManagers[field.m_type].first))(field, i);

		ui->TemplateWidgetTable->setCellWidget(i, 0, nameInfield);
		ui->TemplateWidgetTable->setCellWidget(i, 1, comboType);
		ui->TemplateWidgetTable->setCellWidget(i, 2, comboRef);
		if (widg != nullptr)
			ui->TemplateWidgetTable->setCellWidget(i, 3, widg);
		ui->TemplateWidgetTable->setCellWidget(i, 4, deletebtn);

		QObject::connect(nameInfield, &QLineEdit::editingFinished, this, [this, i]() { modifyName(i); });
		QObject::connect(nameInfield, &QLineEdit::textChanged, this, [this, i]() { verifNameModif(i); });
		QObject::connect(deletebtn, &QPushButton::released, this, [this, i]() { deleteField(i); });
		QObject::connect(comboType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, i]() { modifyType(i); });
		QObject::connect(comboRef, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, i]() { modifyRef(i); });

		++i;
	}

	createCreationField();
	ui->TemplateWidgetTable->show();
	ui->TemplateNameInfield->blockSignals(false);

	GUI_LOG << "receive template " << rTemp->getName() << LOGENDL;
}

void TemplateEditorDialog::createCreationField()
{
	QLineEdit *fakeInfield = new QLineEdit("", ui->TemplateWidgetTable);

	fakeInfield->blockSignals(true);
	fakeInfield->setPlaceholderText(TEXT_TITLE_NEW_FIELD);
	fakeInfield->setMaxLength(20);
	fakeInfield->blockSignals(false);

	ui->TemplateWidgetTable->setCellWidget(ui->TemplateWidgetTable->rowCount() - 1, 0, fakeInfield);

	connect(fakeInfield, SIGNAL(editingFinished()), this, SLOT(TryToAddNewLine()));
}

void TemplateEditorDialog::renameTemplate()
{
	ReadPtr<sma::TagTemplate> rTemp = m_template.cget();
	if (!rTemp)
		return;

	GUI_LOG << "renameTemplate" << LOGENDL;
	if (ui->TemplateNameInfield->text() != "" && ui->TemplateNameInfield->text().toStdWString() != rTemp->getName())
		m_dataDispatcher.sendControl(new control::tagTemplate::RenameTagTemplate(m_template, ui->TemplateNameInfield->text().toStdWString()));
	else
	{
		ui->TemplateNameInfield->blockSignals(true);
		ui->TemplateNameInfield->setText(QString::fromStdWString(rTemp->getName()));
		ui->TemplateNameInfield->blockSignals(false);
	}
}

void TemplateEditorDialog::verifNameModif(int line)
{
	ui->TemplateWidgetTable->setSortingEnabled(false);

	std::vector<sma::tField> fields;

	{
		ReadPtr<sma::TagTemplate> rTemp = m_template.cget();
		if (!rTemp)
			return;

		fields = rTemp->getFieldsCopy();
	}

	int saveLine = line;

	for (auto it = fields.begin(); it != fields.end(); it++)
	{
		if (line == 0)
		{
			QWidget *widg = ui->TemplateWidgetTable->cellWidget(saveLine, 0);
			QLineEdit *nameInfield = static_cast<QLineEdit*>(widg);

			if (nameInfield->text().size() > 20)
				nameInfield->setText(QString::fromStdWString((*it).m_name));
		}
		--line;
	}
	ui->TemplateWidgetTable->setSortingEnabled(true); 
}

void TemplateEditorDialog::modifyName(int line)
{
	ui->TemplateWidgetTable->sortItems(0);
	ui->TemplateWidgetTable->setSortingEnabled(false);
	GUI_LOG << "modifyName" << LOGENDL;

	std::vector<sma::tField> fields;
	{
		ReadPtr<sma::TagTemplate> rTemp = m_template.cget();
		if (!rTemp)
			return;
		fields = rTemp->getFieldsCopy();
	}

	int saveLine = line;

	for (auto it = fields.begin(); it != fields.end(); it++)
	{
		if (line == 0)
		{
			QWidget *widg = ui->TemplateWidgetTable->cellWidget(saveLine, 0);
			QLineEdit *nameInfield = static_cast<QLineEdit*>(widg);

			m_dataDispatcher.sendControl(new control::tagTemplate::TemplateRenameField(m_template, (*it).m_id, nameInfield->text().toStdWString()));
		}
		--line;
	}
	ui->TemplateWidgetTable->setSortingEnabled(true); 
}

void TemplateEditorDialog::modifyType(int line)
{
	ui->TemplateWidgetTable->setSortingEnabled(false);
	GUI_LOG << "modifyType" << LOGENDL; 
	
	std::vector<sma::tField> fields;
	{
		ReadPtr<sma::TagTemplate> rTemp = m_template.cget();
		if (!rTemp)
			return;
		fields = rTemp->getFieldsCopy();
	}

	int saveLine = line;

	for (auto it = fields.begin(); it != fields.end(); it++)
	{
		if (line == 0)
		{
			QWidget *widg = ui->TemplateWidgetTable->cellWidget(saveLine, 1);
			QComboBox *typeCombo = static_cast<QComboBox*>(widg);
			sma::tFieldType type = sma::tFieldType::none;
			if (typeCombo->currentIndex() >= 0 && typeCombo->currentIndex() < m_typesVec.size())
				type = m_typesVec[typeCombo->currentIndex()];
			m_dataDispatcher.sendControl(new control::tagTemplate::TemplateChangeTypeField(m_template, (*it).m_id, type));
		}
		--line;
	}
	ui->TemplateWidgetTable->setSortingEnabled(true);
}

void TemplateEditorDialog::modifyRef(int line)
{
	ui->TemplateWidgetTable->setSortingEnabled(false);
	GUI_LOG << "modifyRef" << LOGENDL;

	std::vector<sma::tField> fields;
	{
		ReadPtr<sma::TagTemplate> rTemp = m_template.cget();
		if (!rTemp)
			return;
		fields = rTemp->getFieldsCopy();
	}

	int saveLine = line;

	for (auto it = fields.begin(); it != fields.end(); it++)
	{
		if (line == 0)
		{
			QWidget *widg = ui->TemplateWidgetTable->cellWidget(saveLine, 2);
			QComboBox *refCombo = static_cast<QComboBox*>(widg);
			xg::Guid refSelected = xg::Guid("0");
			if (refCombo->currentIndex() >= 0 && refCombo->currentIndex() < m_listLink.size())
				m_dataDispatcher.sendControl(new control::tagTemplate::TemplateChangeRefField(m_template, (*it).m_id, m_listLink[refCombo->currentIndex()]));
		}
		--line;
	}
	ui->TemplateWidgetTable->setSortingEnabled(true);
}

void TemplateEditorDialog::modifyDefaultValue(int line)
{
	ui->TemplateWidgetTable->setSortingEnabled(false);
	GUI_LOG << "modifyDefaultValue" << LOGENDL;

	std::vector<sma::tField> fields;
	{
		ReadPtr<sma::TagTemplate> rTemp = m_template.cget();
		if (!rTemp)
			return;
		fields = rTemp->getFieldsCopy();
	}

	int saveLine = line;
	sma::tFieldType type = sma::tFieldType::none;

	for (auto it = fields.begin(); it != fields.end(); it++)
	{
		if (line == 0)
		{
			std::wstring result = (this->*(m_defaultValueManagers[(*it).m_type].second))(saveLine);
			m_dataDispatcher.sendControl(new control::tagTemplate::TemplateChangeDefaultValue(m_template, (*it).m_id, result));
			return;
		}
		--line;
	}
	//type;
	ui->TemplateWidgetTable->setSortingEnabled(true);
}

void TemplateEditorDialog::deleteField(int line)
{
	ui->TemplateWidgetTable->setSortingEnabled(false);
	GUI_LOG << "deleteField" << LOGENDL;
	QMessageBox::StandardButton reply = QMessageBox::question(this, TEXT_TITLE_FIELD_REMOVAL_BOX, TEXT_MESSAGE_FIELD_REMOVAL_BOX,
		QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);
	if (reply == QMessageBox::Yes)
	{
		std::vector<sma::tField> fields;
		{
			ReadPtr<sma::TagTemplate> rTemp = m_template.cget();
			if (!rTemp)
				return;
			fields = rTemp->getFieldsCopy();
		}

		for (auto it = fields.begin(); it != fields.end(); it++)
		{
			if (line == 0)
				m_dataDispatcher.sendControl(new control::tagTemplate::TemplateDeleteField(m_template, (*it).m_id));
			--line;
		}
	}
	ui->TemplateWidgetTable->setSortingEnabled(true);
}

void TemplateEditorDialog::TryToAddNewLine()
{
	ui->TemplateWidgetTable->setSortingEnabled(false);
	GUI_LOG << "add new line" << LOGENDL;
	QWidget *newWidget = ui->TemplateWidgetTable->cellWidget(ui->TemplateWidgetTable->rowCount() - 1, 0);
	QLineEdit *lEdit = static_cast<QLineEdit*>(newWidget);

	if (TEXT_TITLE_NEW_FIELD != lEdit->text() && lEdit->text() != QString(""))
		m_dataDispatcher.sendControl(new control::tagTemplate::TemplateCreateField(m_template, lEdit->text().toStdWString()));

	lEdit->blockSignals(true);
	lEdit->setText(TEXT_TITLE_NEW_FIELD);
	lEdit->blockSignals(false);
	ui->TemplateWidgetTable->setSortingEnabled(true);
}

void TemplateEditorDialog::controlNumberDefaultField(int line)
{
	ui->TemplateWidgetTable->setSortingEnabled(false); 
	
	std::vector<sma::tField> fields;
	{
		ReadPtr<sma::TagTemplate> rTemp = m_template.cget();
		if (!rTemp)
			return;
		fields = rTemp->getFieldsCopy();
	}

	int saveLine = line;

	for (auto it = fields.begin(); it != fields.end(); it++)
	{
		if (line == 0)
		{
			QWidget *widg = ui->TemplateWidgetTable->cellWidget(saveLine, 3);
			QLineEdit *NumberInfield = static_cast<QLineEdit*>(widg);
			std::string finalstr = "";
			std::string str = NumberInfield->text().toStdString();
			for (std::string::iterator itStr = str.begin(); itStr != str.end(); itStr++)
				if (std::isdigit(*itStr) > 0)
					finalstr += (*itStr);
			NumberInfield->blockSignals(true);
			NumberInfield->setText(QString::fromStdString(finalstr));
			NumberInfield->blockSignals(false);
		}
		--line;
	}
	ui->TemplateWidgetTable->setSortingEnabled(true);
}

void TemplateEditorDialog::CancelDialog()
{

}

void TemplateEditorDialog::FinishDialog()
{
	close();
	delete(this);
}

QWidget * TemplateEditorDialog::generateStringWidget(sma::tField& field, int line)
{
	ui->TemplateWidgetTable->setSortingEnabled(false);
	QLineEdit *lineEdit;

	lineEdit = new QLineEdit(ui->TemplateWidgetTable);
	lineEdit->blockSignals(true);
	lineEdit->setText(QString::fromStdWString(field.m_defaultValue));
	lineEdit->blockSignals(false);
	QObject::connect(lineEdit, &QLineEdit::editingFinished, this, [this, line]() { modifyDefaultValue(line); });
	ui->TemplateWidgetTable->setSortingEnabled(true);
	return (lineEdit);
}

QWidget * TemplateEditorDialog::generateMultiLineWidget(sma::tField& field, int line)
{
	return (nullptr);
}

QWidget * TemplateEditorDialog::generateDateWidget(sma::tField& field, int line)
{
	return (nullptr);
	//QPushButton *dateBtn;//décès
}

QWidget * TemplateEditorDialog::generateNumberWidget(sma::tField& field, int line)
{
	ui->TemplateWidgetTable->setSortingEnabled(false);
	QLineEdit *lineEdit;

	lineEdit = new QLineEdit(ui->TemplateWidgetTable);
	lineEdit->blockSignals(true);
	lineEdit->setText(QString::fromStdWString(field.m_defaultValue));
	lineEdit->blockSignals(false);
	QObject::connect(lineEdit, &QLineEdit::editingFinished, this, [this, line]() { modifyDefaultValue(line); });
	QObject::connect(lineEdit, &QLineEdit::textChanged, this, [this, line]() { controlNumberDefaultField(line); });
	ui->TemplateWidgetTable->setSortingEnabled(true); 
	return (lineEdit);
}

QWidget * TemplateEditorDialog::generateHyperlinkWidget(sma::tField& field, int line)
{
	ui->TemplateWidgetTable->setSortingEnabled(false);
	QLineEdit *lineEdit;

	lineEdit = new QLineEdit(ui->TemplateWidgetTable);
	lineEdit->blockSignals(true);
	lineEdit->setText(QString::fromStdWString(field.m_defaultValue));
	lineEdit->blockSignals(false);
	QObject::connect(lineEdit, &QLineEdit::editingFinished, this, [this, line]() { modifyDefaultValue(line); });
	ui->TemplateWidgetTable->setSortingEnabled(true); 
	return (lineEdit);
}

QWidget * TemplateEditorDialog::generateListWidget(sma::tField& field, int line)
{
	ui->TemplateWidgetTable->setSortingEnabled(false);
	QComboBox *defaultListElem;

	defaultListElem = new QComboBox(ui->TemplateWidgetTable);
	defaultListElem->blockSignals(true);

	ReadPtr<UserList> rList = field.m_fieldReference.cget();
	if (rList)
	{
		int savedRef = -1;
		int refIndex = 0;

		for (std::wstring value : rList->clist())
		{
			defaultListElem->addItem(QString::fromStdWString(value));
			if (field.m_defaultValue == value)
				savedRef = refIndex;
			++refIndex;
		}
		defaultListElem->setCurrentIndex(savedRef);
	}

	defaultListElem->blockSignals(false);
	QObject::connect(defaultListElem, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, line]() { modifyDefaultValue(line); });
	ui->TemplateWidgetTable->setSortingEnabled(false); 
	return (defaultListElem);
}

QWidget * TemplateEditorDialog::generateNoneWidget(sma::tField& field, int line)
{
	return (nullptr);
}

std::wstring TemplateEditorDialog::gatherStringWidget(int line)
{
	ui->TemplateWidgetTable->setSortingEnabled(true); 

	std::wstring str = L"";

	std::vector<sma::tField> fields;
	{
		ReadPtr<sma::TagTemplate> rTemp = m_template.cget();
		if (!rTemp)
			return str;
		fields = rTemp->getFieldsCopy();
	}

	int saveLine = line;

	for (auto it = fields.begin(); it != fields.end(); it++)
	{
		if (line == 0)
		{
			QWidget *widg = ui->TemplateWidgetTable->cellWidget(saveLine, 3);
			QLineEdit *hyperInfield = static_cast<QLineEdit*>(widg);
			str = hyperInfield->text().toStdWString();
			return (str);
		}
		--line;
	}
	ui->TemplateWidgetTable->setSortingEnabled(false);
	return (str);
}

std::wstring TemplateEditorDialog::gatherMultiLineWidget(int line)
{
	return (L"");
}

std::wstring TemplateEditorDialog::gatherDateWidget(int line)
{
	return (L"");
}

std::wstring TemplateEditorDialog::gatherNumberWidget(int line)
{
	ui->TemplateWidgetTable->setSortingEnabled(false); 

	std::wstring str = L"";
	std::vector<sma::tField> fields;
	{
		ReadPtr<sma::TagTemplate> rTemp = m_template.cget();
		if (!rTemp)
			return str;
		fields = rTemp->getFieldsCopy();
	}

	int saveLine = line;

	for (auto it = fields.begin(); it != fields.end(); it++)
	{
		if (line == 0)
		{
			QWidget *widg = ui->TemplateWidgetTable->cellWidget(saveLine, 3);
			QLineEdit *hyperInfield = static_cast<QLineEdit*>(widg);
			str = hyperInfield->text().toStdWString();
		}
		--line;
	}
	ui->TemplateWidgetTable->setSortingEnabled(true);
	return (str);
}

std::wstring TemplateEditorDialog::gatherHyperlinkWidget(int line)
{
	ui->TemplateWidgetTable->setSortingEnabled(false); 

	std::wstring str = L"";

	std::vector<sma::tField> fields;
	{
		ReadPtr<sma::TagTemplate> rTemp = m_template.cget();
		if (!rTemp)
			return str;
		fields = rTemp->getFieldsCopy();
	}

	int saveLine = line;

	for (auto it = fields.begin(); it != fields.end(); it++)
	{
		if (line == 0)
		{
			QWidget *widg = ui->TemplateWidgetTable->cellWidget(saveLine, 3);
			QLineEdit *hyperInfield = static_cast<QLineEdit*>(widg);
			str = hyperInfield->text().toStdWString();
		}
		--line;
	}
	ui->TemplateWidgetTable->setSortingEnabled(true);
	return (str);
}

std::wstring TemplateEditorDialog::gatherListWidget(int line)
{
	ui->TemplateWidgetTable->setSortingEnabled(true); 

	std::wstring str = L"";

	std::vector<sma::tField> fields;
	{
		ReadPtr<sma::TagTemplate> rTemp = m_template.cget();
		if (!rTemp)
			return str;
		fields = rTemp->getFieldsCopy();
	}

	int saveLine = line;

	for (sma::tField field : fields)
	{
		if (line == 0)
		{
			QWidget *widg = ui->TemplateWidgetTable->cellWidget(saveLine, 3);
			QComboBox *typeCombo = static_cast<QComboBox*>(widg);
			ReadPtr<UserList> rList = field.m_fieldReference.cget();
			if (typeCombo->currentIndex() >= 0 && rList && typeCombo->currentIndex() < rList->clist().size() + 1)
				str = typeCombo->currentText().toStdWString();
			return (str);
		}
		--line;
	}
	ui->TemplateWidgetTable->setSortingEnabled(false);
	return (str);
}

std::wstring TemplateEditorDialog::gatherNoneWidget(int line)
{
	return (L"");
}

void TemplateEditorDialog::setFilter(const int& header)
{
	if (m_sortedColumn == header)
	{
		if (m_sortOrder == Qt::DescendingOrder)
			m_sortOrder = Qt::AscendingOrder;
		else
			m_sortOrder = Qt::DescendingOrder;
		ui->TemplateWidgetTable->sortByColumn(header, m_sortOrder);
	}
	else
		ui->TemplateWidgetTable->sortByColumn(header, m_sortOrder);
	m_sortedColumn = header;
}