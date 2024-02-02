#ifndef TOOLBAR_BEAM_DETECTION_H
#define TOOLBAR_BEAM_DETECTION_H

#include <QtWidgets/QWidget>
#include "ui_toolbar_beamDetection.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ToolBarBeamDetection;

typedef void (ToolBarBeamDetection::* BeamDetectionMethod)(IGuiData*);

class ToolBarBeamDetection : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarBeamDetection(IDataDispatcher& dataDispatcher, QWidget* parent, float scale);

	void informData(IGuiData* data);

public slots:
	void onSimpleDetection();
	void onManualExtension();
	void onApplyStandard(int checkState);

private:
	~ToolBarBeamDetection();

	void onProjectLoad(IGuiData* data);
	void onActivateFunction(IGuiData* data);


private:
	std::unordered_map<guiDType, BeamDetectionMethod> m_methods;
	Ui::toolbar_beamDetection m_ui;
	IDataDispatcher& m_dataDispatcher;
};

#endif // TOOLBAR_BEAM_DETECTION_H

