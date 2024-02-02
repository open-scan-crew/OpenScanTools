#ifndef GUI_DATA_CONTEXT_H
#define GUI_DATA_CONTEXT_H

#include "gui/GuiData/IGuiData.h"
#include "controller/functionSystem/AContext.h"
#include "crossguid/Guid.hpp"

class GuiDataContextRequestActiveCamera : public IGuiData
{
public:
	GuiDataContextRequestActiveCamera(ContextId contextId);
	~GuiDataContextRequestActiveCamera() {}
	guiDType getType() override;
public:
	const ContextId m_contextId;
};

#endif