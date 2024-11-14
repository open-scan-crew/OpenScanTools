#ifndef TOOLBAR_STRUCUTREANALYSIS_H
#define TOOLBAR_STRUCUTREANALYSIS_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_structAnalysis.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ToolBarStructureAnalysis;

typedef void (ToolBarStructureAnalysis::* StructureAnalysisMethod)(IGuiData*);

class ToolBarStructureAnalysis : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarStructureAnalysis(IDataDispatcher& dataDispatcher, QWidget* parent, float scale);


	void informData(IGuiData* data);
	void onProjectLoad(IGuiData* data);
	void onFunctionActived(IGuiData* data);
	void onTolerances(IGuiData* data);

public slots:
	void initColumnTiltMeasure();
	void initBeamBendingMeasure();
	void setTiltTolerance();
	void setBendingTolerance();

private:
	~ToolBarStructureAnalysis();

private:
	std::unordered_map<guiDType, StructureAnalysisMethod> m_methods;
	Ui::toolbar_structAnalysis m_ui;
	IDataDispatcher& m_dataDispatcher;
};

#endif // TOOLBAR_STRUCUTREANALYSIS_H

