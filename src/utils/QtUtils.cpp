#include "utils/QtUtils.h"
#include "QtWidgets/qlayout.h"
#include "QtGui/qstandarditemmodel.h"
namespace utils
{
	//https://stackoverflow.com/questions/32584320/programmatically-promote-qwidget
	void replace(QWidget* old, QWidget* replacement)
	{
		QWidget* parent = dynamic_cast<QWidget*>(old->parent());
		if (!parent)
			return;
		QLayout* layout = parent->layout();
		// name the new widget the same
		replacement->setObjectName(old->objectName());
		// swap the widgets and delete the layout item
		delete layout->replaceWidget(old, replacement);
		// delete the old widget
		delete old;
		// don't leave a dangling pointer
		old = nullptr;
	}


	void setComboBoxItemEnabled(QComboBox* comboBox, int index, bool enabled)
	{
		//get combobox model
		QStandardItemModel* model = qobject_cast<QStandardItemModel*>(comboBox->model());
		assert(model);
		if (!model) return;

		//get indexed item
		QStandardItem* item = model->item(index);
		assert(item);
		if (!item) return;

		//disable/enable
		item->setEnabled(enabled);
	}
}