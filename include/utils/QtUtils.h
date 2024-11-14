#ifndef QTUTILS_H_
#define QTUTILS_H_

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qcombobox.h>

namespace utils
{
	void replace(QWidget* old, QWidget* replacement);
	void setComboBoxItemEnabled(QComboBox* comboBox, int index, bool enabled);
}

#endif // QTUTILS_H_