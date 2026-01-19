#include "gui/Dialog/DialogDisplayPresets.h"

DialogDisplayPresets::DialogDisplayPresets(QWidget* parent)
	: QDialog(parent)
{
	m_ui.setupUi(this);

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::MSWindowsFixedSizeDialogHint;
	flags ^= Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	setModal(true);
}

DialogDisplayPresets::~DialogDisplayPresets() = default;

void DialogDisplayPresets::setPresetName(const QString& name)
{
	m_ui.lineEdit_displayPresetName->setText(name);
}

QString DialogDisplayPresets::presetName() const
{
	return m_ui.lineEdit_displayPresetName->text();
}

void DialogDisplayPresets::setEditMode(bool isEditMode)
{
	m_isEditMode = isEditMode;
	m_ui.updateButton->setEnabled(isEditMode);
	m_ui.deleteButton->setEnabled(isEditMode);
}

bool DialogDisplayPresets::isEditMode() const
{
	return m_isEditMode;
}

QPushButton* DialogDisplayPresets::okButton() const
{
	return m_ui.okButton;
}

QPushButton* DialogDisplayPresets::updateButton() const
{
	return m_ui.updateButton;
}

QPushButton* DialogDisplayPresets::defaultButton() const
{
	return m_ui.defaultButton;
}

QPushButton* DialogDisplayPresets::initialPresetButton() const
{
	return m_ui.initialPresetButton;
}

QPushButton* DialogDisplayPresets::deleteButton() const
{
	return m_ui.deleteButton;
}

QPushButton* DialogDisplayPresets::cancelButton() const
{
	return m_ui.cancelButton;
}
