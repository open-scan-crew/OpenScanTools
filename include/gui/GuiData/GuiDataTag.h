#ifndef GUI_DATA_TAG_H
#define GUI_DATA_TAG_H

#include "gui/GuiData/IGuiData.h"
#include "models/data/Marker.h"

class GuiDataTagDefaultIcon : public IGuiData
{
public:
    GuiDataTagDefaultIcon(scs::MarkerIcon _icon);
    guiDType getType() override;
public:
    scs::MarkerIcon icon;
};


#endif