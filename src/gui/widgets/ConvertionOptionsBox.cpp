#include "gui/widgets/ConvertionOptionsBox.h"
#include "io/FileUtils.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/widgets/E57TreeWidget.h"
#include "gui/texts/TlsTexts.hpp"
#include "gui/texts/FileTypeTexts.hpp"
#include "gui/Texts.hpp"
#include "gui/texts/ContextTexts.hpp"
#include "controller/controls/ControlProject.h"
#include "controller/controls/ControlFunction.h"


#include "QtWidgets/qfiledialog.h"

ConvertionOptionsBox::ConvertionOptionsBox(IDataDispatcher& dataDispatcher, QWidget* parent)
	: ADialog(dataDispatcher, parent)
	, m_mask(0)
	, m_isBrowsingFolder(true)
	, m_fileInspector(nullptr)
{
	m_ui.setupUi(this);
	setWindowTitle(TEXT_CONVERTION_TITLE);
	
	connect(m_ui.okButton, &QPushButton::released, this, &ConvertionOptionsBox::sendConvertionInfo);
	connect(m_ui.cancelButton, &QPushButton::released, this, &ConvertionOptionsBox::cancelConvertion);
	m_dataDispatcher.registerObserverOnKey(this, { guiDType::conversionOptionsDisplay });
	m_dataDispatcher.registerObserverOnKey(this, guiDType::conversionFilePaths);

	m_ui.formatComboBox->clear();
	for (const auto& iterator : s_OutputFormat)
		m_ui.formatComboBox->addItem(iterator.first);
	m_ui.formatGroupBox->hide();

	m_ui.precisionComboBox->clear();
    m_ui.precisionComboBox->addItem(TEXT_TLS_PRECISION_1_MILLIMETER, QVariant((int)tls::PrecisionType::TL_OCTREE_1MM));
    m_ui.precisionComboBox->addItem(TEXT_TLS_PRECISION_100_MICROMETER, QVariant((int)tls::PrecisionType::TL_OCTREE_100UM));
    m_ui.precisionComboBox->addItem(TEXT_TLS_PRECISION_10_MICROMETER, QVariant((int)tls::PrecisionType::TL_OCTREE_10UM));
	m_ui.precisionComboBox->setCurrentIndex(1);
	m_ui.precisionGroupBox->hide();

	connect(m_ui.browseButton, &QPushButton::released, this, &ConvertionOptionsBox::selectOutDir);
	m_ui.browseGroupBox->hide();

	m_ui.list_fileNames->hide();
	m_ui.csvCheckBox->hide();
	m_ui.importCheckBox->hide();
	m_ui.forceCheckBox->hide();
	m_ui.multiscanRenamingLabel->hide();

    m_ui.groupBox_truncate->hide();

	m_ui.lineEdit_truncateX->setType(NumericType::DISTANCE);
	m_ui.lineEdit_truncateY->setType(NumericType::DISTANCE);
	m_ui.lineEdit_truncateZ->setType(NumericType::DISTANCE);

	m_ui.gridLayout->setSizeConstraint(QLayout::SetFixedSize);
	this->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Ignored);

	E57TreeWidget* e57Inspector = new E57TreeWidget(this);
	m_fileInspector = e57Inspector;
	m_ui.mainLayout->addWidget(e57Inspector, 0, 1, 1, 2);
	connect(m_ui.list_fileNames, &QListWidget::itemClicked, [e57Inspector](QListWidgetItem* item) {
		e57Inspector->inspect(item->text());
		e57Inspector->show();
	});
	e57Inspector->hide();

	adjustSize();
}

ConvertionOptionsBox::~ConvertionOptionsBox()
{}

void ConvertionOptionsBox::closeEvent(QCloseEvent *event)
{
	cancelConvertion();
}

void ConvertionOptionsBox::updateUI()
{
	m_ui.lineEdit_truncateX->setValue(m_storedTranslation.x);
	m_ui.lineEdit_truncateY->setValue(m_storedTranslation.y);
	m_ui.lineEdit_truncateZ->setValue(m_storedTranslation.z);
}

void ConvertionOptionsBox::informData(IGuiData *keyValue)
{
	switch (keyValue->getType()) {

		case guiDType::conversionOptionsDisplay: 
		{
			GuiDataConversionOptionsDisplay* info = dynamic_cast<GuiDataConversionOptionsDisplay*>(keyValue);
			if (!info)
				return;
			m_mask = info->m_type;
			m_ui.formatGroupBox->setVisible(m_mask & BoxOptions::FORMAT);
			m_ui.precisionGroupBox->setVisible(m_mask & BoxOptions::PRECISION);

			//NOTE (Aurélien) outdir or outfile but not both
			if (info->m_type & BoxOptions::OUTDIR) 
			{
				m_isBrowsingFolder = true;
				m_ui.browseGroupBox->show();
			}
			else if (info->m_type & BoxOptions::OUTFILE)
			{
				m_isBrowsingFolder = false;
				m_ui.browseGroupBox->show();
			}

			m_ui.csvCheckBox->setVisible(m_mask & BoxOptions::CSV);

			m_ui.importCheckBox->setVisible(m_mask & BoxOptions::IMPORT);

			m_ui.groupBox_truncate->setVisible(m_mask & BoxOptions::TRUNCATE_COORDINATES);

			m_ui.checkBoxColor->setVisible(m_mask & BoxOptions::ASK_COLOR_PRESENT);

			m_ui.multiscanRenamingLabel->setVisible(m_mask & BoxOptions::AUTO_RENAMING_MULTISCAN);

			m_ui.forceCheckBox->setVisible(m_mask & BoxOptions::FORCE_FILE_OVERWRITE);
			if (m_mask & BoxOptions::TRUNCATE_COORDINATES) {
				m_storedTranslation = info->m_translation;
				updateUI();
			}	

			if (abs(m_storedTranslation.x) >= BIG_COORDINATES_THRESHOLD
				|| abs(m_storedTranslation.y) >= BIG_COORDINATES_THRESHOLD
				|| abs(m_storedTranslation.z) >= BIG_COORDINATES_THRESHOLD)
			{
				m_ui.groupBox_truncate->setChecked(true);
				QString labelText = "<P><b><i><font color='#ff0000' font_size=4>";
				labelText += TEXT_WARNING;
				labelText.append("</font></i></b></P></br>");
				labelText += TEXT_BIG_COORDINATES_DETECTED;
				m_ui.warningCoordinatesLabel->setText(labelText);
			}
			else
				m_ui.warningCoordinatesLabel->setText(TEXT_TRANSLATE_COORDINATES);

			show();
			break;
		}
		case guiDType::conversionFilePaths:
		{
			auto data = static_cast<GuiDataConversionFilePaths*>(keyValue);

			m_ui.list_fileNames->clear();
			static_cast<E57TreeWidget*>(m_fileInspector)->clear();

			std::map<FileType, std::vector < std::filesystem::path>> filepathsPerType;


			for (std::filesystem::path path : data->m_paths)
			{
				if (ExtensionDictionnary.find(path.extension().string()) != ExtensionDictionnary.end())
				{
					FileType type = ExtensionDictionnary.at(path.extension().string());
					if (filepathsPerType.find(type) == filepathsPerType.end())
						filepathsPerType[type] = std::vector<std::filesystem::path>();
					filepathsPerType[type].push_back(path);
				}
			}

			if (filepathsPerType.find(FileType::E57) == filepathsPerType.end())
			{
				m_fileInspector->hide();
				m_ui.list_fileNames->hide();
			}
			else
			{
				for (std::filesystem::path path : filepathsPerType.at(FileType::E57))
					m_ui.list_fileNames->addItem(QString::fromStdWString(path.wstring()));
				m_fileInspector->show();
				m_ui.list_fileNames->show();
			}

			adjustSize();
		}
	}
}

void ConvertionOptionsBox::sendConvertionInfo()
{
    ConvertProperties prop = {};
	prop.mask = m_mask;
    prop.filePrecision = m_ui.precisionComboBox->currentData().toInt();
	prop.fileFormat = m_ui.formatComboBox->currentIndex();
	prop.isCSV = m_ui.csvCheckBox->isChecked();
	prop.isImport = m_ui.importCheckBox->isChecked();
	prop.output = std::filesystem::path(m_ui.outDir->text().toStdString());
    prop.readFlsColor = m_ui.checkBoxColor->isChecked();
	prop.overwriteExisting = m_ui.forceCheckBox->isChecked();

    if (m_ui.groupBox_truncate->isChecked())
    {
        prop.truncate.x = m_ui.lineEdit_truncateX->getValue();
        prop.truncate.y = m_ui.lineEdit_truncateY->getValue();
        prop.truncate.z = m_ui.lineEdit_truncateZ->getValue();
    }

	if (m_isBrowsingFolder && !prop.output.has_extension() || !m_isBrowsingFolder && prop.output.has_extension())
	{
		m_dataDispatcher.sendControl(new control::project::ConvertScan(prop));
		hide();
	}
}

void  ConvertionOptionsBox::cancelConvertion()
{
	m_dataDispatcher.sendControl(new control::function::Abort);
	hide();
}

void ConvertionOptionsBox::selectOutDir()
{
	QString output;
	if(m_isBrowsingFolder)
		output = QFileDialog::getExistingDirectory(this, TEXT_SELECT_DIRECTORY, QDir::currentPath(), QFileDialog::ShowDirsOnly);
	else
		output = QFileDialog::getSaveFileName(this, TEXT_SAVE_FILENAME, QDir::currentPath(), TEXT_FILE_TYPE_TLS, nullptr);

	m_ui.outDir->setText(output);
}
