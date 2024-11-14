#include "gui/Dialog/ADialog.h"

ADialog::ADialog(IDataDispatcher& dataDispacher, QWidget* parent)
	: QDialog(parent)
	, m_dataDispatcher(dataDispacher)
{
	setModal(true);

    // ----- Important | Window Flags -----
    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::MSWindowsFixedSizeDialogHint;
    flags ^= Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);
    // ------------------------------------
}

ADialog::~ADialog()
{}
