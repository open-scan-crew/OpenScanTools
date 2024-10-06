#include "gui/Dialog/DialogDeletePoints.h"

#include "gui/texts/ExportTexts.hpp"
#include "io/FileUtils.h"
#include "controller/controls/ControlFunction.h"
#include "controller/messages/ClippingExportParametersMessage.h"
#include "controller/messages/DeletePointsMessage.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "models/graph/BoxNode.h"


DialogDeletePoints::DialogDeletePoints(IDataDispatcher& dataDispatcher, QWidget *parent)
    : ADialog(dataDispatcher, parent)
    , m_clippingFilter(ExportClippingFilter::SELECTED)
{
    m_ui.setupUi(this);
    translateUI();
    initSourceOption();

    connect(m_ui.comboBox_source, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogDeletePoints::onSelectSource);

    m_ui.comboBox_source->setCurrentIndex((int)m_clippingFilter);

    connect(m_ui.pushButton_apply, &QPushButton::clicked, this, &DialogDeletePoints::startDeletion);
    connect(m_ui.pushButton_cancel, &QPushButton::clicked, this, &DialogDeletePoints::cancelDeletion);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::deletePointsDialogDisplay);
    adjustSize();
}

DialogDeletePoints::~DialogDeletePoints()
{
    m_dataDispatcher.unregisterObserver(this);
}

void DialogDeletePoints::informData(IGuiData *data)
{
    switch (data->getType())
    {
    case guiDType::deletePointsDialogDisplay:
    {
        auto displayParam = static_cast<GuiDataDeletePointsDialogDisplay*>(data);
        m_ui.label_msg->clear();

        m_clippings.clear();
        for (const SafePtr<AClippingNode>& clipping : displayParam->m_clippings)
        {
            ReadPtr<AClippingNode> readClip = clipping.cget();
            // We do not want the grids
            if (readClip->getType() == ElementType::Grid)
                continue;

            m_clippings.push_back(clipping);
        }
        // Show the names of the clippings in the dialog
        refreshUI();
        break;
    }
    break;
    }
}

void DialogDeletePoints::closeEvent(QCloseEvent* event)
{
    cancelDeletion();
}

void DialogDeletePoints::onSelectSource(int index)
{
    m_clippingFilter = (ExportClippingFilter)m_ui.comboBox_source->currentData().toInt();
    refreshClippingNames();
}

void DialogDeletePoints::startDeletion()
{
    // Get the export precision
    m_dataDispatcher.sendControl(new control::function::ForwardMessage(new DeletePointsMessage(m_clippingFilter)));

    hide();
}


void DialogDeletePoints::cancelDeletion()
{
    m_dataDispatcher.sendControl(new control::function::Abort());
    hide();
}

void DialogDeletePoints::translateUI()
{
    setWindowTitle(TEXT_EXPORT_TITLE_DELETE_POINTS);

    m_exportSourceTexts = {
        { ExportClippingFilter::SELECTED, TEXT_EXPORT_CLIPPING_SOURCE_SELECTED },
        { ExportClippingFilter::ACTIVE, TEXT_EXPORT_CLIPPING_SOURCE_ACTIVE }
    };
}

void DialogDeletePoints::refreshUI()
{
    // Show source options (all, some or none)
    refreshClippingNames();

    m_ui.label_msg->setVisible(m_ui.label_msg->text() != "");
    adjustSize();
    show();
}

void DialogDeletePoints::refreshClippingNames()
{
    m_ui.listWidget_clippingNames->clear();

    for (const SafePtr<AClippingNode>& clip : m_clippings)
    {
        ReadPtr<AClippingNode> readClip = clip.cget();
        if ((readClip->isSelected() && m_clippingFilter == ExportClippingFilter::SELECTED) ||
            (readClip->isClippingActive() && m_clippingFilter == ExportClippingFilter::ACTIVE))
        {
            m_ui.listWidget_clippingNames->addItem(QString::fromStdWString(readClip->getComposedName()));
        }
    }
}

void DialogDeletePoints::initSourceOption()
{
    std::set<ExportClippingFilter> authorizedValues;
    authorizedValues.insert(ExportClippingFilter::SELECTED);
    authorizedValues.insert(ExportClippingFilter::ACTIVE);

    m_ui.comboBox_source->clear();
    for (auto value : authorizedValues)
    {
        if (m_exportSourceTexts.find(value) != m_exportSourceTexts.end())
            m_ui.comboBox_source->addItem(m_exportSourceTexts.at(value), QVariant((int)value));
        else
            m_ui.comboBox_source->addItem(TEXT_EXPORT_LABEL_MISSING, QVariant((int)value));
    }
}