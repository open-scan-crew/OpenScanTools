#ifndef _PROPERTIESCLUSTER_PANEL_H_
#define _PROPERTIESCLUSTER_PANEL_H_

#include "gui/IPanel.h"
#include "ui_Property_Cluster.h"
#include "gui/widgets/APropertyGeneral.h"

#include <crossguid/guid.hpp>
#include <unordered_map>

class ClusterNode;
class Controller;

class PropertiesClusterPanel : public APropertyGeneral
{
	Q_OBJECT

public:
	PropertiesClusterPanel(const Controller& controller, QWidget* parent);
	~PropertiesClusterPanel();

	void informData(IGuiData *keyValue) override;

	void onProjectLoad(IGuiData *data);
	void hideEvent(QHideEvent* event);

	virtual bool actualizeProperty(SafePtr<AGraphNode> object );

public slots:
	void showColorPicker();

private:

	void selectColor(QPushButton *color);

private:
	SafePtr<ClusterNode> m_storedFolder;
	
	Ui::PropertyCluster m_ui;
};

#endif // _PROPERTIESCLUSTER_PANEL_H_