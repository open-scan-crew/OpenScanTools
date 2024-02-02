#include "gui/Dialog/DialogShortcuts.h"

DialogShortcuts::DialogShortcuts(QWidget* parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    setModal(true);

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::MSWindowsFixedSizeDialogHint;
    flags ^= Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    connect(m_ui.okButton, &QPushButton::released, [this]() {this->hide(); });
}

DialogShortcuts::~DialogShortcuts()
{
}