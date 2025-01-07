#include "gui/widgets/PropertyPointCloud.h"
#include "utils/QtLogStream.hpp"
#include "utils/math/trigo.h"
#include "controller/Controller.h"
#include "controller/controls/ControlScanEdition.h"
#include "controller/controls/ControlDataEdition.h"
#include "controller/controls/ControlObject3DEdition.h"
#include "models/graph/APointCloudNode.h"

#define TEXT_SCAN_PROPERTIES PropertyPointCloud::tr("Scan properties")
#define TEXT_PCO_PROPERTIES PropertyPointCloud::tr("Point Cloud Object properties")

PropertyPointCloud::PropertyPointCloud(const Controller& controller, QWidget* parent)
    : APropertyGeneral(controller.getDataDispatcher(), parent)
{
    m_ui.setupUi(this);

    m_ui.genericPropsHeadWidget->setControllerInfo(controller);
    m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());

    // Link action
    connect(m_ui.colorPicker, &QPushButton::clicked, this, &PropertyPointCloud::slotSetScanColor);
    connect(m_ui.checkBox_clippable, &QCheckBox::clicked, this, &PropertyPointCloud::slotClippableChanged);
    connect(m_ui.addAsKeyPoint, &QPushButton::clicked, this, &PropertyPointCloud::addAsAnimationViewPoint);

    // Link action
    connect(m_ui.posX, &QLineEdit::editingFinished, this, &PropertyPointCloud::onCenterXEdit);
    connect(m_ui.posY, &QLineEdit::editingFinished, this, &PropertyPointCloud::onCenterYEdit);
    connect(m_ui.posZ, &QLineEdit::editingFinished, this, &PropertyPointCloud::onCenterZEdit);

    connect(m_ui.rotX, &QLineEdit::editingFinished, this, &PropertyPointCloud::onOrientXEdit);
    connect(m_ui.rotY, &QLineEdit::editingFinished, this, &PropertyPointCloud::onOrientYEdit);
    connect(m_ui.rotZ, &QLineEdit::editingFinished, this, &PropertyPointCloud::onOrientZEdit);

    connect(m_ui.scaleX, &QLineEdit::editingFinished, this, &PropertyPointCloud::onScaleXEdit);
    connect(m_ui.scaleY, &QLineEdit::editingFinished, this, &PropertyPointCloud::onScaleYEdit);
    connect(m_ui.scaleZ, &QLineEdit::editingFinished, this, &PropertyPointCloud::onScaleZEdit);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::renderValueDisplay);

    m_ui.posX->setType(NumericType::DISTANCE);
    m_ui.posY->setType(NumericType::DISTANCE);
    m_ui.posZ->setType(NumericType::DISTANCE);

    m_ui.rotX->setUnit(UnitType::DEG);
    m_ui.rotY->setUnit(UnitType::DEG);
    m_ui.rotZ->setUnit(UnitType::DEG);

#ifndef ANIMATION
    m_ui.addAsKeyPoint->setVisible(false);
#endif 
}

PropertyPointCloud::~PropertyPointCloud()
{
    m_dataDispatcher.unregisterObserver(this);
}

void PropertyPointCloud::hideEvent(QHideEvent* event)
{

}

bool PropertyPointCloud::actualizeProperty(SafePtr<AGraphNode> object )
{
    if (object)
        m_storedScan = static_pointer_cast<APointCloudNode>(object);

    m_ui.genericPropsHeadWidget->setObject(m_storedScan);
    m_ui.genericPropsFeetWidget->setObject(m_storedScan);

    return update();
}

bool PropertyPointCloud::update()
{
    ReadPtr<APointCloudNode> pointCloud = m_storedScan.cget();
    if (!pointCloud)
        return false;

    m_ui.RGBCheckBox->setCheckState((pointCloud->getRGBAvailable() == true) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    m_ui.IntensityCheckBox->setCheckState((pointCloud->getIntensityAvailable() == true) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    m_ui.numberOfPoints->setText(QString::number(pointCloud->getNbPoint()));
    m_ui.scannerModel->setText(QString::fromStdWString(pointCloud->getSensorModel()));
    m_ui.scannerSerialNumber->setText(QString::fromStdWString(pointCloud->getSensorSerialNumber()));
    m_ui.acquisitionDate->setText(QString::fromStdWString(pointCloud->getStringAcquisitionTime()));
    m_ui.importDate->setText(QString::fromStdWString(pointCloud->getStringTimeCreated()));
    m_ui.checkBox_clippable->setChecked(pointCloud->getClippable());

    setObject3DParameters(*&pointCloud);

    QColor color = QColorFromColor32(pointCloud->getColor());
    QPalette palPicker = m_ui.colorPicker->palette();
    palPicker.setColor(QPalette::Button, color);
    palPicker.setColor(QPalette::ButtonText, !color);
    m_ui.colorPicker->setPalette(palPicker);

    // *** Différences d’affichage entre Scan et PCO ***
    bool trueScan = (pointCloud->getType() == ElementType::Scan);

    // NOTE - This title will later be used by the DockWidget that host this widget.
    setWindowTitle(trueScan ? TEXT_SCAN_PROPERTIES : TEXT_PCO_PROPERTIES);

    m_ui.posX->setReadOnly(trueScan);
    m_ui.posY->setReadOnly(trueScan);
    m_ui.posZ->setReadOnly(trueScan);

    m_ui.rotX->setReadOnly(trueScan);
    m_ui.rotY->setReadOnly(trueScan);
    m_ui.rotZ->setReadOnly(trueScan);

    m_ui.scaleX->setReadOnly(trueScan);
    m_ui.scaleY->setReadOnly(trueScan);
    m_ui.scaleZ->setReadOnly(trueScan);

    m_ui.scannerModel->setVisible(trueScan);
    m_ui.label_scanner_model->setVisible(trueScan);
    m_ui.scannerSerialNumber->setVisible(trueScan);
    m_ui.label_scanner_serial_number->setVisible(trueScan);
    m_ui.colorPicker->setVisible(trueScan);

    return true;
}

void PropertyPointCloud::setObject3DParameters(const TransformationModule& data)
{
    m_ui.posX->blockSignals(true);
    m_ui.posY->blockSignals(true);
    m_ui.posZ->blockSignals(true);
    m_ui.rotX->blockSignals(true);
    m_ui.rotY->blockSignals(true);
    m_ui.rotZ->blockSignals(true);
    m_ui.scaleX->blockSignals(true);
    m_ui.scaleY->blockSignals(true);
    m_ui.scaleZ->blockSignals(true);

    m_ui.posX->setValue(data.getCenter().x);
    m_ui.posY->setValue(data.getCenter().y);
    m_ui.posZ->setValue(data.getCenter().z);

    glm::dvec3 eulers = tls::math::quat_to_euler_zyx_deg(data.getOrientation());
    m_ui.rotX->setValue(eulers.x);
    m_ui.rotY->setValue(eulers.y);
    m_ui.rotZ->setValue(eulers.z);

    m_ui.scaleX->setValue(data.getScale().x);
    m_ui.scaleY->setValue(data.getScale().y);
    m_ui.scaleZ->setValue(data.getScale().z);

    m_ui.posX->blockSignals(false);
    m_ui.posY->blockSignals(false);
    m_ui.posZ->blockSignals(false);
    m_ui.rotX->blockSignals(false);
    m_ui.rotY->blockSignals(false);
    m_ui.rotZ->blockSignals(false);
    m_ui.scaleX->blockSignals(false);
    m_ui.scaleY->blockSignals(false);
    m_ui.scaleZ->blockSignals(false);
}

void PropertyPointCloud::slotSetScanColor()
{
    ReadPtr<APointCloudNode> scan = m_storedScan.cget();
    QColor color = QColorDialog::getColor(QColorFromColor32(scan->getColor()), this);

    if (color.isValid() == true)
    {
        if (color != scan->getColor())
        {
            m_dataDispatcher.sendControl(new control::dataEdition::SetColor(m_storedScan, Color32FromQColor(color)));
            QPalette palPicker = m_ui.colorPicker->palette();
            palPicker.setColor(QPalette::Button, color);
            palPicker.setColor(QPalette::ButtonText, !color);
            m_ui.colorPicker->setPalette(palPicker);
        }
    }
}

void PropertyPointCloud::slotClippableChanged(int value)
{
    m_dataDispatcher.sendControl(new control::scanEdition::SetClippable(m_storedScan, m_ui.checkBox_clippable->isChecked()));
}

void PropertyPointCloud::addAsAnimationViewPoint()
{
    Pos3D pos = { std::strtod(m_ui.posX->text().toStdString().c_str(), NULL), std::strtod(m_ui.posY->text().toStdString().c_str(), NULL), std::strtod(m_ui.posZ->text().toStdString().c_str(), NULL) };
    //m_dataDispatcher.sendControl(new control::animation::AddViewPoint(m_storedScan));
}

void PropertyPointCloud::changeCenter()
{
    glm::dvec3 position(m_ui.posX->getValue(), m_ui.posY->getValue(), m_ui.posZ->getValue());
    m_dataDispatcher.sendControl(new control::object3DEdition::SetCenter(m_storedScan, position));
}

void PropertyPointCloud::changeOrientation()
{
    glm::dvec3 orientation(m_ui.rotX->getValue(), m_ui.rotY->getValue(), m_ui.rotZ->getValue());
    m_dataDispatcher.sendControl(new control::object3DEdition::SetRotation(m_storedScan, tls::math::euler_deg_to_quat(orientation)));
}

void PropertyPointCloud::changeScale()
{
    glm::dvec3 scale(m_ui.scaleX->getValue(), m_ui.scaleY->getValue(), m_ui.scaleZ->getValue());
    m_dataDispatcher.sendControl(new control::object3DEdition::SetSize(m_storedScan, scale));
}

void PropertyPointCloud::onOrientXEdit()
{
    if (!m_ui.rotX->isModified())
        return;
    changeOrientation();
}

void PropertyPointCloud::onOrientYEdit()
{
    if (!m_ui.rotY->isModified())
        return;
    changeOrientation();
}

void PropertyPointCloud::onOrientZEdit()
{
    if (!m_ui.rotZ->isModified())
        return;
    changeOrientation();
}

void PropertyPointCloud::onCenterXEdit()
{
    if (!m_ui.posX->isModified())
        return;
    changeCenter();
}

void PropertyPointCloud::onCenterYEdit()
{
    if (!m_ui.posY->isModified())
        return;
    changeCenter();
}

void PropertyPointCloud::onCenterZEdit()
{
    if (!m_ui.posZ->isModified())
        return;
    changeCenter();
}

void PropertyPointCloud::onScaleXEdit()
{
    if (!m_ui.scaleX->isModified())
        return;
    changeScale();
}

void PropertyPointCloud::onScaleYEdit()
{
    if (!m_ui.scaleY->isModified())
        return;
    changeScale();
}

void PropertyPointCloud::onScaleZEdit()
{
    if (!m_ui.scaleZ->isModified())
        return;
    changeScale();
}
