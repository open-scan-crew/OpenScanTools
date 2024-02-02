#ifndef GUI_DATA_TREE_H
#define GUI_DATA_TREE_H

#include "gui/GuiData/IGuiData.h"

#include "models/data/Data.h"

#include <map>
#include <vector>

class AGraphNode;

class GuiDataTreeActualizeNodes : public IGuiData
{
public:
    GuiDataTreeActualizeNodes(const SafePtr<AGraphNode>& object);
    GuiDataTreeActualizeNodes(const std::unordered_set<SafePtr<AGraphNode>>& objects);
    ~GuiDataTreeActualizeNodes();
    guiDType getType() override;

    inline void push_object(const SafePtr<AGraphNode>& object)
    {
        m_objects.insert(object);
    };
public:
    std::unordered_set<SafePtr<AGraphNode>> m_objects;
};


#endif