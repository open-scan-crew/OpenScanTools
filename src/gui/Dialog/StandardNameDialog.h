#ifndef STANDARD_NAME_DIALOG_H_
#define STANDARD_NAME_DIALOG_H_

#include "gui/Dialog/AListNameDialog.h"

enum class StandardType;

class StandardNameDialog : public AListNameDialog
{
	Q_OBJECT

public:
	explicit StandardNameDialog(IDataDispatcher &dataDispacher, const StandardType& type, QWidget *parent = 0);
	~StandardNameDialog();

public slots:
	void acceptCreation(); 
private:
	const StandardType m_type;
};

#endif // !PIPE_STANDARD_NAME_DIALOG_H_