#ifndef TREE_VIEW_H_
#define TREE_VIEW_H_

#include <QtWidgets/qtreeview.h>
#include <QtGui/QDrag>
#include <QtGui/QDragLeaveEvent>
#include <QtGui/qevent.h>

//#define QTREELOG Logger::log(LoggerMode::GuiLog)

class TreeView : public QTreeView
{
	Q_OBJECT

public:
	TreeView(QWidget *parent = (QWidget*)nullptr);
	//TreeView(TreeModel *model);
	//TreeView(TreeModel *model, QWidget *parent = (QWidget*)nullptr);

	//void setModel(TreeModel *model);
signals:
	void myDropEvent(QDropEvent *event);
	void myMoveEvent(QDragMoveEvent *event);
	void myDragEvent(QDragEnterEvent *event);
protected:
	void dragEnterEvent(QDragEnterEvent *qevent) override;
	void dragMoveEvent(QDragMoveEvent *mevent) override;
	void dropEvent(QDropEvent *qevent) override;
};

#endif // _TREEVIEW_H_