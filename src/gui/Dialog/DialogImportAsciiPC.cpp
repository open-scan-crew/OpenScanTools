#include "gui\Dialog\DialogImportAsciiPC.h"
#include "gui/texts/PointCloudTexts.hpp"
#include "gui/texts/FileSystemTexts.hpp"
#include "gui/texts/ErrorMessagesTexts.hpp"
#include "io/FileUtils.h"

#include <QtWidgets/qmessagebox.h>
#include "qcombobox.h"

#include <regex>
#include <array>
#include <fstream>

static std::vector<std::pair<QString, Import::AsciiValueRole>> comboBoxContent = {
	{TEXT_ASCII_IGNORE, Import::AsciiValueRole::Ignore},
	{"X", Import::AsciiValueRole::X},
	{"Y", Import::AsciiValueRole::Y},
	{"Z", Import::AsciiValueRole::Z},
	{TEXT_ASCII_RED, Import::AsciiValueRole::R},
	{TEXT_ASCII_GREEN, Import::AsciiValueRole::G},
	{TEXT_ASCII_BLUE, Import::AsciiValueRole::B},
	{TEXT_ASCII_INTENSITY, Import::AsciiValueRole::I},
	{TEXT_ASCII_RED_FLOAT, Import::AsciiValueRole::Rf},
	{TEXT_ASCII_GREEN_FLOAT, Import::AsciiValueRole::Gf},
	{TEXT_ASCII_BLUE_FLOAT, Import::AsciiValueRole::Bf}
};

DialogImportAsciiPC::DialogImportAsciiPC(QWidget* parent)
	: QDialog(parent)
	, m_fileIndex(0)
	, m_isFinished(false)
{
	m_ui.setupUi(this);

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::MSWindowsFixedSizeDialogHint;
	flags ^= Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);


	QObject::connect(m_ui.applyButton, &QPushButton::released, this, [this]() { this->onOk(false); });
	QObject::connect(m_ui.applyAllButton, &QPushButton::released, this, [this]() { this->onOk(true); });
	QObject::connect(m_ui.cancelButton, &QPushButton::released, this, &DialogImportAsciiPC::onCancel);

	QObject::connect(m_ui.separatorLineEdit, &QLineEdit::textChanged, this, &DialogImportAsciiPC::updateTable);
	QObject::connect(m_ui.whitespacePushButton, &QPushButton::released, this, [this]() {this->changeSeparator(' '); });
	QObject::connect(m_ui.semicolonPushButton, &QPushButton::released, this, [this]() {this->changeSeparator(';'); });
	QObject::connect(m_ui.commaPushButton, &QPushButton::released, this, [this]() {this->changeSeparator(','); });

	QObject::connect(m_ui.skipLineSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &DialogImportAsciiPC::updateTable);
	
	QObject::connect(m_ui.useCommaDecimalChara, &QCheckBox::released, this, &DialogImportAsciiPC::updateTable);

}

DialogImportAsciiPC::~DialogImportAsciiPC()
{}

bool DialogImportAsciiPC::setInfoAsciiPC(std::vector<std::filesystem::path> filePaths)
{
	for (std::filesystem::path path : filePaths)
	{
		FileType type = ExtensionDictionnary.at(path.extension().string());
		if (type == FileType::PTS)
			m_filesValuesRoles.push_back({ path,  Import::AsciiInfo() });
	}

	m_fileIndex = 0;

	if (m_filesValuesRoles.empty())
		return false;

	updateTable();
	return true;
}

bool DialogImportAsciiPC::isFinished()
{
	return m_isFinished;
}

std::map<std::filesystem::path, Import::AsciiInfo> DialogImportAsciiPC::getFileImportInfo()
{
	std::map<std::filesystem::path, Import::AsciiInfo> mapAsciiValuesRoles;
	for (auto el : m_filesValuesRoles)
	{
		mapAsciiValuesRoles[el.first] = el.second;
	}
	return mapAsciiValuesRoles;
}

bool DialogImportAsciiPC::updateTable()
{
	if (m_fileIndex >= m_filesValuesRoles.size())
		return false;

	std::filesystem::path filePath = m_filesValuesRoles[m_fileIndex].first;

	m_ui.filePathLineEdit->setText(QString::fromStdWString(filePath.wstring()));

	m_ui.columnPtsTableWidget->clear();
	std::ifstream fstream(filePath, std::ios::in);

	std::vector<std::string> infoLines;
	int maxLines = 10;
	int maxCol = 0;

	char sep = getSeparator();

	m_ui.asciiCodeLabel->setText(QString::fromStdString("ASCII Code : %1").arg(QString::fromStdString(std::to_string((int)sep))));
	
	if (fstream.good())
	{
		try {

			for (int i = 0; i < m_ui.skipLineSpinBox->value(); i++)
			{
				std::string dmp;
				std::getline(fstream, dmp);
			}

			for (int i = 0; i < maxLines; i++)
			{
				std::string line;
				std::getline(fstream, line);
				infoLines.push_back(line);
				bool something = false;
				int nbCol = 0;
				for (char c : line)
				{
					if (c == sep)
					{
						if(something == true)
							nbCol++;
						something = false;
					}
					else
						something = true;

				}
				if (something)
					nbCol++;

				if (nbCol > maxCol)
					maxCol = nbCol;
			}
		}
		catch (std::exception e)
		{
			assert(false);
			QMessageBox modal(QMessageBox::Icon::Warning, TEXT_ERROR, TEXT_ERROR_OPEN_FILE.arg(QString::fromStdWString(filePath.wstring())), QMessageBox::StandardButton::Ok);
			modal.exec();
			m_fileIndex++;
			return updateTable();
		}
	}
	else
	{
		assert(false);
		QMessageBox modal(QMessageBox::Icon::Warning, TEXT_ERROR, TEXT_ERROR_OPEN_FILE.arg(QString::fromStdWString(filePath.wstring())), QMessageBox::StandardButton::Ok);
		modal.exec();
		m_fileIndex++;
		return updateTable();
	}

	if (infoLines.empty())
	{
		assert(false);
		QMessageBox modal(QMessageBox::Icon::Warning, TEXT_ERROR, TEXT_ERROR_EMPTY_FILE.arg(QString::fromStdWString(filePath.wstring())), QMessageBox::StandardButton::Ok);
		modal.exec();
		m_fileIndex++;
		return updateTable();
	}

	int row = 1;

	//header
	m_ui.columnPtsTableWidget->setRowCount(1);
	m_ui.columnPtsTableWidget->setColumnCount(maxCol);
	for (int c = 0; c < m_ui.columnPtsTableWidget->columnCount(); c++)
		m_ui.columnPtsTableWidget->setCellWidget(0, c, createChoiceBox(m_ui.columnPtsTableWidget, c));


	std::regex autorizedChar("[0-9,.-]");

	for (std::string line : infoLines)
	{
		m_ui.columnPtsTableWidget->setRowCount(row + 1);
		
		std::string temp = "";
		int col = 0;

		bool error = false;
		for (char c : line)
		{
			if (c == sep)
			{
				if (temp.empty())
					continue;
				
				QTableWidgetItem* newItem = new QTableWidgetItem(QString::fromStdString(temp));
				if (error)
					newItem->setBackground(Qt::red);

				m_ui.columnPtsTableWidget->setItem(row, col, newItem);

				temp.clear();
				col++;
				error = false;
			}
			else
			{
				temp += c;

				std::string singleChar = "";
				singleChar += c;
				if (!std::regex_match(singleChar, autorizedChar))
					error = true;

				if (c == '.' && m_ui.useCommaDecimalChara->isChecked() || c == ',' && !m_ui.useCommaDecimalChara->isChecked())
					error = true;
			}
		}

		QTableWidgetItem* newItem = new QTableWidgetItem(QString::fromStdString(temp));
		if (error)
			newItem->setBackground(Qt::red);

		m_ui.columnPtsTableWidget->setItem(row, col, newItem);

		temp.clear();
		row++;
	}

	m_ui.columnPtsTableWidget->resizeColumnsToContents();

	return true;
}

char DialogImportAsciiPC::getSeparator()
{
	char sep = ' ';
	if (!m_ui.separatorLineEdit->text().isEmpty())
		sep = m_ui.separatorLineEdit->text().toStdString().at(0);
	return sep;
}

QComboBox* DialogImportAsciiPC::createChoiceBox(QWidget* parent, int index)
{

	QComboBox* combo = new QComboBox(parent);
	for (auto content : comboBoxContent)
		combo->addItem(content.first, QVariant((uint32_t)content.second));

	QObject::connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this,index]() {updateComboBox(index); });

	if (index + 1 >= comboBoxContent.size())
		combo->setCurrentIndex(0);
	else
		combo->setCurrentIndex(index+1);
	return combo;
}

void DialogImportAsciiPC::updateComboBox(int changedColInd)
{
	QComboBox* changedComboBox = static_cast<QComboBox*>(m_ui.columnPtsTableWidget->cellWidget(0, changedColInd));
	if (changedComboBox == nullptr)
		return;

	for (int colInd = 0; colInd < m_ui.columnPtsTableWidget->columnCount(); colInd++)
	{
		if (colInd == changedColInd)
			continue;
		QComboBox* comboB = static_cast<QComboBox*>(m_ui.columnPtsTableWidget->cellWidget(0, colInd));
		if (comboB == nullptr)
			continue;

		if (comboB->currentIndex() == changedComboBox->currentIndex())
			comboB->setCurrentIndex(0);
	}
}

void DialogImportAsciiPC::changeSeparator(char sep)
{
	std::string newText = "";
	newText += sep;

	m_ui.separatorLineEdit->setText(QString::fromStdString(newText));
	updateTable();
}

void DialogImportAsciiPC::onOk(bool all)
{
	Import::AsciiInfo info;
	std::vector<Import::AsciiValueRole> values;
	std::array<bool, 3> containXYZ = { false, false, false };
	for (int colInd = 0; colInd < m_ui.columnPtsTableWidget->columnCount(); colInd++)
	{
		Import::AsciiValueRole role = Import::AsciiValueRole(static_cast<QComboBox*>(m_ui.columnPtsTableWidget->cellWidget(0, colInd))->currentData().toInt());
		if (role == Import::AsciiValueRole::X)
			containXYZ[0] = true;
		if (role == Import::AsciiValueRole::Y)
			containXYZ[1] = true;
		if (role == Import::AsciiValueRole::Z)
			containXYZ[2] = true;
		values.push_back(role);
	}

	if (!containXYZ[0] || !containXYZ[1] || !containXYZ[2])
	{
		QMessageBox modal(QMessageBox::Icon::Warning, TEXT_ERROR, TEXT_ERROR_NO_XYZ_SET, QMessageBox::StandardButton::Ok);
		modal.exec();
		return;
	}

	info.columnsRole = values;
	info.sep = getSeparator();
	info.useCommaAsDecimal = m_ui.useCommaDecimalChara->isChecked();

	if (all)
	{
		for (int i = m_fileIndex; i < m_filesValuesRoles.size(); i++)
			m_filesValuesRoles[i] = { m_filesValuesRoles[i].first, info };
		m_isFinished = true;
		QDialog::accept();
	}
	else
	{
		m_filesValuesRoles[m_fileIndex] = {m_filesValuesRoles[m_fileIndex].first, info };
		m_fileIndex++;

		if (m_fileIndex < m_filesValuesRoles.size())
			updateTable();
		else
		{
			m_isFinished = true;
			QDialog::accept();
		}
	}

}

void DialogImportAsciiPC::onCancel()
{
	m_isFinished = false;
	QDialog::accept();
}
