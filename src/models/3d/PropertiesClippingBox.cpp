#include "gui/widgets/PropertiesClippingBox.h"

#define _USE_MATH_DEFINES
#include <math.h>

PropertiesClippingBox::PropertiesClippingBox(QWidget *parent, const float& guiScale, const ApplicationType& type)
    : QWidget(parent)
    , ui(new Ui::toolbar_clippinggroup())
{
    ui->setupUi(this);
    resetUI();

    connect(ui->lineEdit_posX, &QLineEdit::editingFinished, this, [this]() { updateProperty(ui->lineEdit_posX, m_cbox.center.x, false); });
    connect(ui->lineEdit_posY, &QLineEdit::editingFinished, this, [this]() { updateProperty(ui->lineEdit_posY, m_cbox.center.y, false); });
    connect(ui->lineEdit_posZ, &QLineEdit::editingFinished, this, [this]() { updateProperty(ui->lineEdit_posZ, m_cbox.center.z, false); });
    connect(ui->lineEdit_rotX, &QLineEdit::editingFinished, this, [this]() { updateProperty(ui->lineEdit_rotX, m_cbox.rotX, true); });
    connect(ui->lineEdit_rotY, &QLineEdit::editingFinished, this, [this]() { updateProperty(ui->lineEdit_rotY, m_cbox.rotY, true); });
    connect(ui->lineEdit_rotZ, &QLineEdit::editingFinished, this, [this]() { updateProperty(ui->lineEdit_rotZ, m_cbox.rotZ, true); });
    connect(ui->lineEdit_sizeX, &QLineEdit::editingFinished, this, [this]() { updateProperty(ui->lineEdit_sizeX, m_cbox.sizeX, false); });
    connect(ui->lineEdit_sizeY, &QLineEdit::editingFinished, this, [this]() { updateProperty(ui->lineEdit_sizeY, m_cbox.sizeY, false); });
    connect(ui->lineEdit_sizeZ, &QLineEdit::editingFinished, this, [this]() { updateProperty(ui->lineEdit_sizeZ, m_cbox.sizeZ, false); });

	switch (type)
	{
		case ApplicationType::SCANSAP:
			ui->okButton->hide();
			ui->cancelButton->hide();
			connect(ui->checkBox_show, &QCheckBox::stateChanged, this, [this](int state) {
				if (state == 0) emit onActivateClipping(false);
				if (state == 2) emit onActivateClipping(true);
			});
			break; 
		case ApplicationType::CONVERTER:
			ui->checkBox_show->hide();
			connect(ui->okButton, &QAbstractButton::clicked, this, [this]() { this->hide(); emit onClippingBoxValidated(m_cbox); });
			connect(ui->cancelButton, &QAbstractButton::clicked, this, [this]() { this->hide(); });
			break;
	}
}

PropertiesClippingBox::~PropertiesClippingBox()
{
    emit onActivateClipping(false);
};

void PropertiesClippingBox::closeEvent(QCloseEvent*)
{
    ui->checkBox_show->setChecked(false);
    emit onActivateClipping(false);
}

void PropertiesClippingBox::setClippingBoxValue(const ClippingBox& clipBox)
{
    m_cbox = clipBox;
    resetUI();
}

void PropertiesClippingBox::resetUI()
{
    ui->lineEdit_posX->setText(QString::number(m_cbox.center.x));
    ui->lineEdit_posY->setText(QString::number(m_cbox.center.y));
    ui->lineEdit_posZ->setText(QString::number(m_cbox.center.z));
    ui->lineEdit_rotX->setText(QString::number(m_cbox.rotX));
    ui->lineEdit_rotY->setText(QString::number(m_cbox.rotY));
    ui->lineEdit_rotZ->setText(QString::number(m_cbox.rotZ));
    ui->lineEdit_sizeX->setText(QString::number(m_cbox.sizeX));
    ui->lineEdit_sizeY->setText(QString::number(m_cbox.sizeY));
    ui->lineEdit_sizeZ->setText(QString::number(m_cbox.sizeZ));
}

void PropertiesClippingBox::updateProperty(QLineEdit* editInput, float& prop, bool convertToRadians)
{
    float newValue = editInput->text().toFloat();

    if (convertToRadians)
        newValue = newValue * (float)M_PI / 180.f;


    if (newValue != prop)
    {
        prop = newValue;

        emit onClippingBoxChanged(m_cbox);
    }
}