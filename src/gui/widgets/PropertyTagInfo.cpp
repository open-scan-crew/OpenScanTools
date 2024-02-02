#include "gui/widgets/PropertyTagInfo.h"
#include "utils/ProjectStringSets.hpp"
#include "utils/Config.h"
#include "controller/controls/ControlTagEdition.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include <cctype>

#include <qpushbutton.h>
#include <glm/glm.hpp>
#include "controller/controls/ControlAnimation.h"

#include <magic_enum/magic_enum.hpp>

FocusWatcher::FocusWatcher(QObject* parent) 
	: QObject(parent)
	, m_blocking(false)
{
	if (parent)
		parent->installEventFilter(this);
}

FocusWatcher::~FocusWatcher(){}

bool FocusWatcher::eventFilter(QObject *obj, QEvent *event)
{
	//std::cout << magic_enum::enum_name(event->type()) << std::endl;
	if (this->parent() != obj)
		return true;
	if (event->type() == QEvent::WindowDeactivate)
		m_blocking = true;
	else if (event->type() == QEvent::WindowActivate)
		m_blocking = false;
		/*if (event->type() == QEvent::FocusIn)
			emit focusChanged(true);*/
	else if (event->type() == QEvent::FocusOut && !m_blocking)
		emit focusOut();
	return false;
}


PropertyTagInfo::PropertyTagInfo(IDataDispatcher& dataDispatcher, QWidget *parent, float guiScale)
	: QWidget(parent)
	, ui(new Ui::property_tagInfo)
	, m_dataDispatcher(dataDispatcher)
{
	ui->setupUi(this);
	ActValues = ProjectStringSets::getStringSet("ACTION");
	CatValues = ProjectStringSets::getStringSet("CATEGORY");
	PrefixValues = ProjectStringSets::getStringSet("PREFIX");
	StatusValues = ProjectStringSets::getStringSet("STATUS");

	for (std::vector<std::string>::iterator it = ActValues.begin(); it != ActValues.end(); it++)
		ui->comboBoxAction->addItem(QString::fromStdString(*it));
	for (std::vector<std::string>::iterator it = CatValues.begin(); it != CatValues.end(); it++)
		ui->comboBoxCategory->addItem(QString::fromStdString(*it));
	for (std::vector<std::string>::iterator it = PrefixValues.begin(); it != PrefixValues.end(); it++)
		ui->comboPrefix->addItem(QString::fromStdString(*it));
	for (std::vector<std::string>::iterator it = StatusValues.begin(); it != StatusValues.end(); it++)
		ui->comboBoxStatus->addItem(QString::fromStdString(*it));

    // Instanciate the color picker
    m_colorPicker = new ColorPicker(this, guiScale);
    ui->gridLayout->addWidget(m_colorPicker, 5, 1);

    connect(m_colorPicker, &ColorPicker::pickedColor, this, &PropertyTagInfo::changeTagColor);

	ui->labelHyperlinkLink->setTextFormat(Qt::RichText);
	ui->labelHyperlinkLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
	ui->labelHyperlinkLink->setOpenExternalLinks(true);

	//connect(ui->lineEditName, SIGNAL(editingFinished()), this, SLOT(createEditNameControl()));
	connect(new FocusWatcher(ui->lineEditName), &FocusWatcher::focusOut, this, &PropertyTagInfo::createEditNameControl);
	connect(ui->comboBoxAction, SIGNAL(currentIndexChanged(int)), this, SLOT(changeActionCombo(int)));
	connect(ui->comboBoxCategory, SIGNAL(currentIndexChanged(int)), this, SLOT(changeCatCombo(int)));
	connect(ui->comboPrefix, SIGNAL(currentIndexChanged(int)), this, SLOT(changePrefixCombo(int)));
	connect(ui->comboBoxStatus, SIGNAL(currentIndexChanged(int)), this, SLOT(changeStatusCombo(int)));
	//connect(ui->lineEditIndex, SIGNAL(editingFinished()), this, SLOT(changeUserIndex()));
	connect(new FocusWatcher(ui->lineEditIndex), &FocusWatcher::focusOut, this, &PropertyTagInfo::changeUserIndex);
	//connect(ui->plainTextEditDescription, SIGNAL(textChanged()), this, SLOT(createEditDescControl()));
    connect(new FocusWatcher(ui->plainTextEditDescription), &FocusWatcher::focusOut, this, &PropertyTagInfo::createEditDescControl);
	connect(new FocusWatcher(ui->lineEditHyperlink), &FocusWatcher::focusOut, this, &PropertyTagInfo::changeHyperLink);
	//connect(ui->lineEditHyperlink, SIGNAL(editingFinished()), this, SLOT(changeHyperLink()));
	connect(ui->lineEditHyperlink, SIGNAL(textChanged(const QString&)), this, SLOT(changeHyperLinkIntern(const QString&)));



	connect(ui->addAskeyPointButton,&QPushButton::clicked, this, &PropertyTagInfo::addAsAnimationKeyPoint);

	m_dataDispatcher.registerObserverOnKey(this, { guiDType::TagDataProperties });
	m_dataDispatcher.registerObserverOnKey(this, { guiDType::projectLoaded });
}

PropertyTagInfo::~PropertyTagInfo()
{
	PANELLOG << "destructor" << LOGENDL;
	m_dataDispatcher.unregisterObserver(this);
}

void PropertyTagInfo::informData(IGuiData *data)
{
	ui->comboBoxAction->blockSignals(true);
	ui->comboBoxCategory->blockSignals(true);
	ui->comboBoxStatus->blockSignals(true);
	ui->comboPrefix->blockSignals(true);
	ui->lineEditAuthor->blockSignals(true);
	ui->lineEditHyperlink->blockSignals(true);
	ui->lineEditIndex->blockSignals(true);
	ui->lineEditName->blockSignals(true);
	ui->plainTextEditDescription->blockSignals(true);

	if (data->getType() == guiDType::TagDataProperties)
	{
		GuiDataTagProperties *TPdata = static_cast<GuiDataTagProperties*>(data);

		storedTag = smp::Tag(*TPdata->_tag);

		ui->lineEditName->setText(QString::fromStdString(storedTag.getName()));
		ui->lineEditIndex->setText(QString::number(storedTag.getUserIndex()));
		ui->plainTextEditDescription->setPlainText(QString::fromStdString(storedTag.getDescription()));
		ui->lineEditAuthor->setText(QString::fromStdString(storedTag.getAuthor()));
		ui->lineEditHyperlink->setText(QString::fromStdString(storedTag.getHyperlink()));
		ui->labelHyperlinkLink->setText("<a href=\"" + QString::fromStdString(storedTag.getHyperlink())
			+ "\">" + QString::fromStdString(TPdata->_tag->getHyperlink()) + "</a>");
		ui->labelDateCreated->setText(QString::fromStdString(storedTag.getStringTime()));

		if (ui->plainTextEditDescription->toPlainText().toStdString() != TPdata->_tag->getDescription())
			ui->plainTextEditDescription->setPlainText(QString::fromStdString(TPdata->_tag->getDescription()));
		
		calculateBestCursorPosition(TPdata->_tag->getCursorPosition(), TPdata->_tag->getDescription().size());
		QTextCursor cursor = ui->plainTextEditDescription->textCursor();
		cursor.setPosition(storedDescCursor);
		ui->plainTextEditDescription->setTextCursor(cursor);

		setComboIndex(ui->comboBoxCategory, CatValues, TPdata->_tag->getCategory());
		setComboIndex(ui->comboBoxAction, ActValues, TPdata->_tag->getAction());
		setComboIndex(ui->comboPrefix, PrefixValues, TPdata->_tag->getPrefix());
		setComboIndex(ui->comboBoxStatus, StatusValues, TPdata->_tag->getStatus());

		QString formater;
		formater.sprintf("%.3f", storedTag.getPosition().x);
		ui->labelPosX->setText(formater);
		formater.sprintf("%.3f", storedTag.getPosition().y);
		ui->labelPosY->setText(formater);
		formater.sprintf("%.3f", storedTag.getPosition().z);
		ui->labelPosZ->setText(formater);
        
        m_colorPicker->setColorChecked(storedTag.getColor());
	}
	else if (data->getType() == guiDType::projectLoaded)
		storedTag.setId(0);

	ui->plainTextEditDescription->blockSignals(false);
	ui->comboBoxAction->blockSignals(false);
	ui->comboBoxCategory->blockSignals(false);
	ui->comboBoxStatus->blockSignals(false);
	ui->comboPrefix->blockSignals(false);
	ui->lineEditAuthor->blockSignals(false);
	ui->lineEditHyperlink->blockSignals(false);
	ui->lineEditIndex->blockSignals(false);
	ui->lineEditName->blockSignals(false);
}

std::string PropertyTagInfo::getName() const
{
	return ("PropertyTagInfo");
}

void PropertyTagInfo::changeActionCombo(int id)
{
	if (storedTag.getId() != 0 && ActValues[id] != storedTag.getAction())
		m_dataDispatcher.sendControl(new control::tagEdition::SetAction(storedTag.getId(), ActValues[id]));
}

void PropertyTagInfo::changeCatCombo(int id)
{
	if (storedTag.getId() != 0 && CatValues[id] != storedTag.getCategory())
		m_dataDispatcher.sendControl(new control::tagEdition::SetCategory(storedTag.getId(), CatValues[id]));
}

void PropertyTagInfo::changePrefixCombo(int id)
{
	if (storedTag.getId() != 0 && PrefixValues[id] != storedTag.getPrefix())
		m_dataDispatcher.sendControl(new control::tagEdition::SetPrefix(storedTag.getId(), PrefixValues[id]));
}

void PropertyTagInfo::changeStatusCombo(int id)
{
	if (storedTag.getId() != 0 && StatusValues[id] != storedTag.getStatus())
		m_dataDispatcher.sendControl(new control::tagEdition::SetStatus(storedTag.getId(), StatusValues[id]));
}

void PropertyTagInfo::changeUserIndex()
{
	if (storedTag.getId() == 0)
		return;

	std::string newText = ui->lineEditIndex->text().toStdString();

	if (newText.empty() == false)
	{
		std::string::iterator it;
		for (it = newText.begin(); it != newText.end(); it++)
			if (std::isdigit(*it) == 0)
			{
				ui->lineEditIndex->setText(QString::number(storedTag.getUserIndex()));
				return;
			}
		uint num = atoi(newText.c_str());
		m_dataDispatcher.sendControl(new control::tagEdition::SetUserId(storedTag.getId(), num));
	}
	else
		ui->lineEditIndex->setText(QString::number(storedTag.getUserIndex()));
}

void PropertyTagInfo::createEditNameControl()
{
    PANELLOG << "Editing name" << LOGENDL;

	if (storedTag.getId() != 0 && ui->lineEditName->text().toStdString() != storedTag.getName())
		m_dataDispatcher.sendControl(new control::tagEdition::SetName(storedTag.getId(), ui->lineEditName->text().toStdString()));
}

void PropertyTagInfo::createEditDescControl()
{
	ui->plainTextEditDescription->blockSignals(true);
	storedDescCursor = ui->plainTextEditDescription->textCursor().position();
	if (storedTag.getId() != 0 && ui->plainTextEditDescription->toPlainText().toStdString() != storedTag.getDescription())
		m_dataDispatcher.sendControl(new control::tagEdition::SetDescription(storedTag.getId(), ui->plainTextEditDescription->toPlainText().toStdString()));
}

void PropertyTagInfo::changeTagColor(Color32 color)
{
	if (storedTag.getId() != 0 && (color == storedTag.getColor()) == false)
		m_dataDispatcher.sendControl(new control::tagEdition::SetColor(storedTag.getId(), color));
}

void PropertyTagInfo::setComboIndex(QComboBox * combo, std::vector<std::string>& values, const std::string& valueToSet)
{
	int i = 0;
	std::vector<std::string>::iterator it;

	combo->setCurrentIndex(0);
	for (it = values.begin(); it != values.end(); it++)
	{
		if ((*it) == valueToSet)
		{
			combo->setCurrentIndex(i);
			break;
		}
		++i;
	}
}

void PropertyTagInfo::changeHyperLinkIntern(const QString& str)
{
	ui->labelHyperlinkLink->setText(ui->lineEditHyperlink->text());
}

void PropertyTagInfo::changeHyperLink()
{
	if (storedTag.getId() != 0 && ui->lineEditHyperlink->text().toStdString() != storedTag.getHyperlink())
	{
		int i = 0;
		std::string str = ui->lineEditHyperlink->text().toStdString();
		std::string strnew = ui->lineEditHyperlink->text().toStdString();

		for (i = 0; i < str.size(); i++)
			if (str[i] == '\\')
				strnew[i] = '/';
		m_dataDispatcher.sendControl(new control::tagEdition::SetHyperlink(storedTag.getId(), strnew));
	}
}

void PropertyTagInfo::addAsAnimationKeyPoint()
{
	Pos3D pos = { std::strtod(ui->labelPosX->text().toStdString().c_str(), NULL), std::strtod(ui->labelPosY->text().toStdString().c_str(), NULL), std::strtod(ui->labelPosZ->text().toStdString().c_str(), NULL) };
	m_dataDispatcher.sendControl(new control::animation::AddKeyPoint(pos));
}