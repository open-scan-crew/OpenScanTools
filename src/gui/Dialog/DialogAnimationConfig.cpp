#include "gui/Dialog/DialogAnimationConfig.h"

#include "controller/controls/ControlAnimation.h"
#include "gui/Dialog/MessageSplashScreen.h"

#include <QtWidgets/qcombobox.h>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qpushbutton.h>
#include <QtGui/QWheelEvent>

namespace
{
QString guidToQString(const xg::Guid& id)
{
    return QString::fromStdString(id.str());
}

xg::Guid qStringToGuid(const QString& value)
{
    return xg::Guid(value.toStdString());
}

class NoWheelComboBox : public QComboBox
{
public:
    using QComboBox::QComboBox;

protected:
    void wheelEvent(QWheelEvent* event) override
    {
        event->ignore();
    }
};

class NoWheelDoubleSpinBox : public QDoubleSpinBox
{
public:
    using QDoubleSpinBox::QDoubleSpinBox;

protected:
    void wheelEvent(QWheelEvent* event) override
    {
        event->ignore();
    }
};
}

DialogAnimationConfig::DialogAnimationConfig(IDataDispatcher& dataDispatcher, QWidget* parent)
    : QDialog(parent)
    , m_dataDispatcher(dataDispatcher)
    , m_editId(xg::Guid("bad-guid-string"))
    , m_isEdition(false)
{
    m_ui.setupUi(this);
    setModal(false);

    const auto pushButtons = findChildren<QPushButton*>();
    for (QPushButton* button : pushButtons)
    {
        if (!button)
            continue;
        button->setAutoDefault(false);
        button->setDefault(false);
    }

    configureTable();

    connect(m_ui.pushButtonAddViewpoint, &QPushButton::clicked, this, &DialogAnimationConfig::onAddViewpoint);
    connect(m_ui.pushButtonMoveUp, &QPushButton::clicked, this, &DialogAnimationConfig::onMoveUp);
    connect(m_ui.pushButtonMoveDown, &QPushButton::clicked, this, &DialogAnimationConfig::onMoveDown);
    connect(m_ui.pushButtonDeleteViewpoint, &QPushButton::clicked, this, &DialogAnimationConfig::onDeleteViewpoint);
    connect(m_ui.pushButtonCleanAnimList, &QPushButton::clicked, this, &DialogAnimationConfig::onCleanList);

    connect(m_ui.okButton, &QPushButton::clicked, this, &DialogAnimationConfig::onOk);
    connect(m_ui.updateButton, &QPushButton::clicked, this, &DialogAnimationConfig::onUpdate);
    connect(m_ui.deleteButton, &QPushButton::clicked, this, &DialogAnimationConfig::onDelete);
    connect(m_ui.cancelButton, &QPushButton::clicked, this, &DialogAnimationConfig::onCancel);

    setupForNew();
    m_ui.lineEdit_animationName->setFocus();
}

DialogAnimationConfig::~DialogAnimationConfig()
{}

void DialogAnimationConfig::setKnownAnimations(const std::vector<ViewPointAnimationConfig>& animations)
{
    m_knownAnimations = animations;
}

void DialogAnimationConfig::setAvailableViewpoints(const std::vector<AnimationViewpointInfo>& viewpoints)
{
    m_availableViewpoints = viewpoints;
}

void DialogAnimationConfig::setupForNew()
{
    m_isEdition = false;
    m_editId = xg::Guid("bad-guid-string");
    m_ui.lineEdit_animationName->clear();
    m_ui.animationListWidgetTable->setRowCount(0);
    m_ui.positionAsTimeRadioButton->setChecked(true);
    m_ui.updateButton->setEnabled(false);
    m_ui.deleteButton->setEnabled(false);
    m_ui.lineEdit_animationName->setFocus();
}

void DialogAnimationConfig::setupForEdit(const ViewPointAnimationConfig& config)
{
    m_isEdition = true;
    m_editId = config.getId();
    setCurrentConfigToUi(config);
    m_ui.updateButton->setEnabled(true);
    m_ui.deleteButton->setEnabled(true);
    m_ui.lineEdit_animationName->setFocus();
}

void DialogAnimationConfig::configureTable()
{
    m_ui.animationListWidgetTable->setColumnCount(2);
    m_ui.animationListWidgetTable->setHorizontalHeaderLabels({ tr("Viewpoint name"), tr("Position") });
    m_ui.animationListWidgetTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_ui.animationListWidgetTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_ui.animationListWidgetTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ui.animationListWidgetTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ui.animationListWidgetTable->verticalHeader()->setSectionsClickable(true);
    connect(m_ui.animationListWidgetTable->verticalHeader(), &QHeaderView::sectionClicked, this, [this](int row) {
        m_ui.animationListWidgetTable->selectRow(row);
    });
}

int DialogAnimationConfig::selectedRow() const
{
    const QModelIndexList selection = m_ui.animationListWidgetTable->selectionModel()->selectedRows();
    if (selection.empty())
        return -1;
    return selection.first().row();
}

void DialogAnimationConfig::insertRowAt(int row)
{
    m_ui.animationListWidgetTable->insertRow(row);

    QComboBox* combo = new NoWheelComboBox(m_ui.animationListWidgetTable);
    combo->addItem(tr("Select viewpoint"), QString());
    for (const AnimationViewpointInfo& vp : m_availableViewpoints)
        combo->addItem(vp.name, guidToQString(vp.id));

    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, combo](int) {
        xg::Guid id = qStringToGuid(combo->currentData().toString());
        if (!id.isValid())
            return;

        const AnimationViewpointInfo info = findViewpointInfo(id);
        if (info.id.isValid() && info.projectionMode == ProjectionMode::Orthographic)
        {
            combo->setCurrentIndex(0);
            showSplashMessage(tr("Only viewpoints in perspective mode are allowed"));
            return;
        }

        checkRenderingConsistency(id);
    });

    m_ui.animationListWidgetTable->setCellWidget(row, 0, combo);

    QDoubleSpinBox* spin = new NoWheelDoubleSpinBox(m_ui.animationListWidgetTable);
    spin->setDecimals(3);
    spin->setRange(0.0, 1000000.0);
    spin->setSingleStep(0.1);
    m_ui.animationListWidgetTable->setCellWidget(row, 1, spin);
}

void DialogAnimationConfig::onAddViewpoint()
{
    if (m_ui.animationListWidgetTable->rowCount() > 0)
    {
        auto* lastCombo = qobject_cast<QComboBox*>(m_ui.animationListWidgetTable->cellWidget(m_ui.animationListWidgetTable->rowCount() - 1, 0));
        if (lastCombo && !qStringToGuid(lastCombo->currentData().toString()).isValid())
        {
            showSplashMessage(tr("You must select a viewpoint before adding a new line."));
            return;
        }
    }

    int row = selectedRow();
    if (row < 0)
        row = m_ui.animationListWidgetTable->rowCount() - 1;
    insertRowAt(row + 1);
    m_ui.animationListWidgetTable->selectRow(row + 1);
}

void DialogAnimationConfig::onMoveUp()
{
    const int row = selectedRow();
    if (row <= 0)
        return;

    auto* combo = qobject_cast<QComboBox*>(m_ui.animationListWidgetTable->cellWidget(row, 0));
    auto* pos = qobject_cast<QDoubleSpinBox*>(m_ui.animationListWidgetTable->cellWidget(row, 1));
    if (!combo || !pos)
        return;

    const QVariant movedViewpointData = combo->currentData();
    const double movedPosition = pos->value();

    m_ui.animationListWidgetTable->removeRow(row);
    const int targetRow = row - 1;
    insertRowAt(targetRow);

    auto* targetCombo = qobject_cast<QComboBox*>(m_ui.animationListWidgetTable->cellWidget(targetRow, 0));
    auto* targetPos = qobject_cast<QDoubleSpinBox*>(m_ui.animationListWidgetTable->cellWidget(targetRow, 1));
    if (!targetCombo || !targetPos)
        return;

    targetCombo->setCurrentIndex(targetCombo->findData(movedViewpointData));
    targetPos->setValue(movedPosition);

    m_ui.animationListWidgetTable->selectRow(targetRow);
}

void DialogAnimationConfig::onMoveDown()
{
    const int row = selectedRow();
    if (row < 0 || row >= m_ui.animationListWidgetTable->rowCount() - 1)
        return;

    auto* combo = qobject_cast<QComboBox*>(m_ui.animationListWidgetTable->cellWidget(row, 0));
    auto* pos = qobject_cast<QDoubleSpinBox*>(m_ui.animationListWidgetTable->cellWidget(row, 1));
    if (!combo || !pos)
        return;

    const QVariant movedViewpointData = combo->currentData();
    const double movedPosition = pos->value();

    m_ui.animationListWidgetTable->removeRow(row);
    const int targetRow = row + 1;
    insertRowAt(targetRow);

    auto* targetCombo = qobject_cast<QComboBox*>(m_ui.animationListWidgetTable->cellWidget(targetRow, 0));
    auto* targetPos = qobject_cast<QDoubleSpinBox*>(m_ui.animationListWidgetTable->cellWidget(targetRow, 1));
    if (!targetCombo || !targetPos)
        return;

    targetCombo->setCurrentIndex(targetCombo->findData(movedViewpointData));
    targetPos->setValue(movedPosition);

    m_ui.animationListWidgetTable->selectRow(targetRow);
}

void DialogAnimationConfig::onDeleteViewpoint()
{
    const int row = selectedRow();
    if (row < 0)
        return;

    m_ui.animationListWidgetTable->removeRow(row);
}

void DialogAnimationConfig::onCleanList()
{
    m_ui.animationListWidgetTable->setRowCount(0);
}

void DialogAnimationConfig::onOk()
{
    if (!validateBeforeSave(true))
        return;

    bool ok = false;
    ViewPointAnimationConfig config = buildConfigFromUi(&ok);
    if (!ok)
        return;

    m_dataDispatcher.sendControl(new control::animation::CreateEditViewPointAnimation(config));
    accept();
}

void DialogAnimationConfig::onUpdate()
{
    if (!m_isEdition)
        return;
    if (!validateBeforeSave(false))
        return;

    bool ok = false;
    ViewPointAnimationConfig config = buildConfigFromUi(&ok);
    if (!ok)
        return;

    config.setId(m_editId);
    m_dataDispatcher.sendControl(new control::animation::CreateEditViewPointAnimation(config));
    accept();
}

void DialogAnimationConfig::onDelete()
{
    if (!m_isEdition || !m_editId.isValid())
        return;

    m_dataDispatcher.sendControl(new control::animation::DeleteViewPointAnimation(m_editId));
    accept();
}

void DialogAnimationConfig::onCancel()
{
    reject();
}

void DialogAnimationConfig::setCurrentConfigToUi(const ViewPointAnimationConfig& config)
{
    m_ui.lineEdit_animationName->setText(config.getName());
    m_ui.animationListWidgetTable->setRowCount(0);

    if (config.getMode() == ViewPointAnimationMode::PositionAsTime)
        m_ui.positionAsTimeRadioButton->setChecked(true);
    else if (config.getMode() == ViewPointAnimationMode::ConstantSpeed)
        m_ui.constantSpeedRadioButton->setChecked(true);
    else
        m_ui.constantIntervalsRadioButton->setChecked(true);

    int row = 0;
    for (const ViewPointAnimationLine& line : config.getLines())
    {
        insertRowAt(row);
        auto* combo = qobject_cast<QComboBox*>(m_ui.animationListWidgetTable->cellWidget(row, 0));
        auto* spin = qobject_cast<QDoubleSpinBox*>(m_ui.animationListWidgetTable->cellWidget(row, 1));
        if (combo)
        {
            const int idx = combo->findData(guidToQString(line.viewpointId));
            combo->setCurrentIndex(idx >= 0 ? idx : 0);
        }
        if (spin)
            spin->setValue(line.position);
        ++row;
    }
}

std::vector<ViewPointAnimationLine> DialogAnimationConfig::readLinesFromUi(bool* ok) const
{
    std::vector<ViewPointAnimationLine> lines;
    if (ok)
        *ok = true;

    for (int row = 0; row < m_ui.animationListWidgetTable->rowCount(); ++row)
    {
        auto* combo = qobject_cast<QComboBox*>(m_ui.animationListWidgetTable->cellWidget(row, 0));
        auto* spin = qobject_cast<QDoubleSpinBox*>(m_ui.animationListWidgetTable->cellWidget(row, 1));
        if (!combo || !spin)
            continue;

        const xg::Guid viewpointId = qStringToGuid(combo->currentData().toString());
        if (!viewpointId.isValid())
        {
            if (ok)
                *ok = false;
            showSplashMessage(tr("You must select a viewpoint for each line."));
            return {};
        }

        ViewPointAnimationLine line;
        line.viewpointId = viewpointId;
        line.viewpointName = combo->currentText();
        line.position = spin->value();
        lines.push_back(line);
    }
    return lines;
}

ViewPointAnimationConfig DialogAnimationConfig::buildConfigFromUi(bool* ok) const
{
    ViewPointAnimationConfig config(m_isEdition ? m_editId : xg::newGuid());
    config.setName(m_ui.lineEdit_animationName->text());
    config.setMode(m_ui.positionAsTimeRadioButton->isChecked() ? ViewPointAnimationMode::PositionAsTime :
                   m_ui.constantSpeedRadioButton->isChecked() ? ViewPointAnimationMode::ConstantSpeed :
                                                               ViewPointAnimationMode::ConstantIntervals);
    config.setLines(readLinesFromUi(ok));

    if (m_isEdition)
    {
        for (const ViewPointAnimationConfig& known : m_knownAnimations)
        {
            if (known.getId() == m_editId)
            {
                config.setOrder(known.getOrder());
                break;
            }
        }
    }

    return config;
}

bool DialogAnimationConfig::validateBeforeSave(bool creationMode) const
{
    const QString name = m_ui.lineEdit_animationName->text().trimmed();
    if (name.isEmpty())
    {
        showSplashMessage(tr("Animation name is required."));
        return false;
    }

    for (const ViewPointAnimationConfig& cfg : m_knownAnimations)
    {
        if (cfg.getName().compare(name, Qt::CaseInsensitive) == 0)
        {
            if (creationMode || cfg.getId() != m_editId)
            {
                showSplashMessage(tr("Animation name already exists."));
                return false;
            }
        }
    }

    if (m_ui.animationListWidgetTable->rowCount() < 2)
    {
        showSplashMessage(tr("You must add at least 2 lines for the viewpoints"));
        return false;
    }

    return true;
}

void DialogAnimationConfig::showSplashMessage(const QString& message) const
{
    MessageSplashScreen splash(const_cast<DialogAnimationConfig*>(this));
    splash.setShowMessage(message);
    splash.exec();
}

AnimationViewpointInfo DialogAnimationConfig::findViewpointInfo(const xg::Guid& id) const
{
    for (const AnimationViewpointInfo& info : m_availableViewpoints)
    {
        if (info.id == id)
            return info;
    }
    return AnimationViewpointInfo();
}

void DialogAnimationConfig::checkRenderingConsistency(const xg::Guid& newViewpointId) const
{
    AnimationViewpointInfo ref;
    bool foundRef = false;
    for (int row = 0; row < m_ui.animationListWidgetTable->rowCount(); ++row)
    {
        auto* combo = qobject_cast<QComboBox*>(m_ui.animationListWidgetTable->cellWidget(row, 0));
        if (!combo)
            continue;
        const xg::Guid id = qStringToGuid(combo->currentData().toString());
        if (!id.isValid() || id == newViewpointId)
            continue;
        ref = findViewpointInfo(id);
        if (ref.id.isValid())
        {
            foundRef = true;
            break;
        }
    }

    if (!foundRef)
        return;

    const AnimationViewpointInfo current = findViewpointInfo(newViewpointId);
    if (!current.id.isValid())
        return;

    const bool consistent =
        current.renderMode == ref.renderMode &&
        current.blendMode == ref.blendMode &&
        current.normals == ref.normals &&
        current.blendColor == ref.blendColor &&
        current.edgeAwareBlur == ref.edgeAwareBlur &&
        current.depthLining == ref.depthLining &&
        current.depthLiningStrongMode == ref.depthLiningStrongMode;

    if (!consistent)
    {
        showSplashMessage(tr("Warning: the added element is not consistent in terms of rendering modes with the other elements in the list.\n"
                             "This may lead to erratic behaviour in the animation and video creation."));
    }
}
