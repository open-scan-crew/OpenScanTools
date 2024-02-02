#include "gui/GuiData/GuiDataContextRequest.h"

/*** CameraPosition ***/
GuiDataContextRequestActiveCamera::GuiDataContextRequestActiveCamera(ContextId contextId)
	: m_contextId(contextId)
{}

guiDType GuiDataContextRequestActiveCamera::getType()
{
	return guiDType::contextRequestCameraPosition;
}
