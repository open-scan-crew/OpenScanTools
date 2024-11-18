#ifndef GUI_DATA_CLIPPING_H
#define GUI_DATA_CLIPPING_H

#include "gui/GuiData/IGuiData.h"
#include "io/exports/ExportParameters.hpp"

#include <QtCore/qstring.h>

class GuiDataClippingSettingsProperties : public IGuiData
{
public:
	GuiDataClippingSettingsProperties(QString windowsName, bool onlyDuplication);
	guiDType getType() override;

public:
	QString m_windowsName;
	bool m_onlyDuplication;
};

class GuiDataPointCloudObjectDialogDisplay : public IGuiData
{
public:
	GuiDataPointCloudObjectDialogDisplay(const PointCloudObjectParameters& parameters);
	guiDType getType() override;

public:
	const PointCloudObjectParameters m_parameters;
};

#endif // !GUI_DATA_CLIPPING_H
