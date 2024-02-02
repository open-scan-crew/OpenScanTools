#include "gui/GuiData/GuiDataTree.h"

//---------------------------------
//  GuiDataTreeActualizeNodes    ||
//---------------------------------

GuiDataTreeActualizeNodes::GuiDataTreeActualizeNodes(const SafePtr<AGraphNode>& object)
	: m_objects({object})
{
}

GuiDataTreeActualizeNodes::GuiDataTreeActualizeNodes(const std::unordered_set<SafePtr<AGraphNode>>& objects)
	: m_objects(objects)
{
}

GuiDataTreeActualizeNodes::~GuiDataTreeActualizeNodes()
{}

guiDType GuiDataTreeActualizeNodes::getType()
{
    return (guiDType::actualizeNodes);
}