#include "gui/dialog/MarkerIconSelectionDialog.h"
#include "services/MarkerSystem.h"
#include "services/MarkerCategories.h"

#include <QtWidgets/qlayoutitem.h>

MarkerIconSelectionDialog::MarkerIconSelectionDialog(QWidget *parent, float _guiScale)
    : QDialog(parent, Qt::MSWindowsFixedSizeDialogHint)
    , m_iconChecked(scs::MarkerIcon::Max_Enum)
{
    setModal(true);

    // Layout
    m_gridLayout = new QGridLayout(this);
    //m_gridLayout->setContentsMargins(0, 0, 0, 0);
    //m_gridLayout->setSpacing(0);
    setLayout(m_gridLayout);

    int lastRow = 0;
    int maxCol = 0;
    for (int category = 0; category < (int)MarkerCategory::Max_Enum; ++category)
    {
        auto it = markerCategoryDefinitions.find((MarkerCategory)category);
        if (it == markerCategoryDefinitions.end())
            continue;

        MarkerCategoryDefinition def = it->second;

        for (int icon = 0; icon < (int)def.iconCount; ++icon)
        {
            scs::MarkerIcon iconEnum = (scs::MarkerIcon)(icon + (int)def.firstIcon);

            QToolButton* button = new QToolButton(this);
            button->setAutoRaise(true);
            MarkerSystem::Style style = MarkerSystem::getStyle(iconEnum);
            button->setIcon(QIcon(style.qresource));
            button->setToolTip(style.traduction);
            button->setIconSize(QSize(50, 50) * _guiScale);
            button->setToolButtonStyle(Qt::ToolButtonIconOnly);
            button->setCheckable(true);
            m_gridLayout->addWidget(button, category, icon);

            connect(button, &QToolButton::clicked, [this, iconEnum](){ this->selectIconButton(iconEnum); });

            m_iconButton.insert({ iconEnum, button });

            if (m_iconChecked == scs::MarkerIcon::Max_Enum)
            {
                button->setChecked(true);
                m_iconChecked = iconEnum;
            }
            maxCol = std::max(maxCol, icon);
        }
        lastRow++;
    }

    m_okButton = new QToolButton(this);
    m_cancelButton = new QToolButton(this);
    QSpacerItem* spacerButton = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    // TODO - Faire un .ui pour cette dialog
    m_okButton->setText(tr("Ok"));
    m_cancelButton->setText(tr("Cancel"));
    m_okButton->setMinimumSize(50, 30);
    m_cancelButton->setMinimumSize(50, 30);

    m_hLayout = new QHBoxLayout();
    m_hLayout->addWidget(m_okButton, 0);
    m_hLayout->addItem(spacerButton);
    m_hLayout->addWidget(m_cancelButton, 2);
    m_gridLayout->addLayout(m_hLayout, lastRow, 0, 1, maxCol + 1);

    connect(m_okButton, &QToolButton::clicked, [this]() {
        this->emit markerSelected(m_iconChecked);
        this->close();
    });

    connect(m_cancelButton, &QToolButton::clicked, [this]() {
        this->close();
    });
}

MarkerIconSelectionDialog::~MarkerIconSelectionDialog()
{

}

void MarkerIconSelectionDialog::informData(IGuiData *keyValue)
{

}

void MarkerIconSelectionDialog::selectIconButton(scs::MarkerIcon icon)
{
    auto it = m_iconButton.find(m_iconChecked);
    if (it != m_iconButton.end())
    {
        it->second->setChecked(false);
        m_iconChecked = scs::MarkerIcon::Max_Enum;
    }

    it = m_iconButton.find(icon);
    if (it != m_iconButton.end())
    {
        it->second->setChecked(true);
        m_iconChecked = icon;
    }
}