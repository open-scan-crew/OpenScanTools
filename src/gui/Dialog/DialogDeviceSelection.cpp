#include "gui/Dialog/DialogDeviceSelection.h"
#include <QtCore/qvariant.h>

DialogDeviceSelection::DialogDeviceSelection(const std::unordered_map<uint32_t, std::string>& devices, QWidget* parent)
	: QDialog(parent)
{
	setModal(true);
	m_ui.setupUi(this);

	// ----- Important | Window Flags -----
	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::MSWindowsFixedSizeDialogHint;
	flags ^= Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);
	// ------------------------------------

	for (auto iterator : devices)
		m_ui.comboBoxDevices->addItem(QString::fromStdString(iterator.second), QVariant(iterator.first));
	m_ui.comboBoxDevices->setCurrentIndex(0);

	connect(m_ui.pushButtonOk, &QPushButton::clicked, this, &DialogDeviceSelection::onOk);
}

DialogDeviceSelection::~DialogDeviceSelection()
{}

int DialogDeviceSelection::exec()
{
	QDialog::exec();
	return m_ui.comboBoxDevices->currentData().toInt();
}

void DialogDeviceSelection::onOk()
{
	accept();
}