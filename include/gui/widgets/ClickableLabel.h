#ifndef CLICKABLELABEL_H_
#define CLICKABLELABEL_H_

#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

class ClickableLabel : public QLabel
{

	Q_OBJECT

public:
	explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
	~ClickableLabel();

signals:

	void clicked();

protected:
	void mousePressEvent(QMouseEvent* event);
};

#endif // CLICKABLELABEL_H_