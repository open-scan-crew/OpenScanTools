#ifndef DIALOG_DISPLAY_PRESETS_H
#define DIALOG_DISPLAY_PRESETS_H

#include "ui_DialogDisplayPresets.h"

#include <QtWidgets/qdialog.h>

class DialogDisplayPresets : public QDialog
{
	Q_OBJECT

public:
	explicit DialogDisplayPresets(QWidget* parent = nullptr);
	~DialogDisplayPresets();

	void setPresetName(const QString& name);
	QString presetName() const;

	void setEditMode(bool isEditMode);
	bool isEditMode() const;

	QPushButton* okButton() const;
	QPushButton* updateButton() const;
	QPushButton* defaultButton() const;
	QPushButton* initialPresetButton() const;
	QPushButton* deleteButton() const;
	QPushButton* cancelButton() const;

private:
	Ui::DialogDisplayPresets m_ui;
	bool m_isEditMode = false;
};

#endif // DIALOG_DISPLAY_PRESETS_H
