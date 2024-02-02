#include "gui/GuiData/GuiDataTag.h"

//++++++ Tag Default parameters +++++++

GuiDataTagDefaultIcon::GuiDataTagDefaultIcon(scs::MarkerIcon _icon)
    : icon(_icon)
{}

guiDType GuiDataTagDefaultIcon::getType()
{
    return guiDType::tagDefaultIcon;
}
