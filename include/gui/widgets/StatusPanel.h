#ifndef _STATUS_PANEL_H_
#define _STATUS_PANEL_H_

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "gui/UnitUsage.h"

#include "glm/glm.hpp"

#include <QtWidgets/qstatusbar.h>
#include <QtWidgets/qlabel.h>

class StatusPanel : public QStatusBar, public IPanel
{
    Q_OBJECT

public:

	StatusPanel(IDataDispatcher &dataDispatcher);
    ~StatusPanel();

    void informData(IGuiData *keyValue) override;

	void onCurrentScanData(IGuiData *data);
	void onTmpMsgData(IGuiData *data);
	void onFunctionUpdate(IGuiData *data);
	void onCameraData(IGuiData* data);
	void onRenderUnitUsage(IGuiData* data);

public slots:
    void onCameraPos(double x, double y, double z);
    void onPicking(double x, double y, double z);

private:
    IDataDispatcher &m_dataDispatcher;
	typedef void (StatusPanel::*statusPanelMethod)(IGuiData*);
	std::unordered_map<guiDType, statusPanelMethod> methods;
	UnitUsage m_valueDisplayParameters;

	int m_tmpMessageTimeout;
	glm::dvec3 m_camPos;
	QLabel* m_projectPointCount;
    QLabel* m_cameraInfo;
	QLabel* m_scanInfo;
	QLabel* m_functionMode;
    QLabel* m_timings;
};

#endif // _STATUS_PANEL_H_
