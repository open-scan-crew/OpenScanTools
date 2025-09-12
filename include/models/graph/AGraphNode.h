#ifndef AGRAPH_NODE_H
#define AGRAPH_NODE_H

#include "utils/IdGiver.hpp"
#include "models/graph/TransformationModule.h"
#include "models/3d/ManipulationTypes.h"
#include "models/data/Data.h"
#include "models/ElementType.h"
#include "models/TreeType.h"
#include "models/3d/MeshDrawData.h"

#include <unordered_set>

class Controller;

class AGraphNode : public Data, public TransformationModule
{
public:
    enum class Type{ Default, Manipulator, Camera };

public:
    AGraphNode(const AGraphNode& node);
    AGraphNode();
    virtual ~AGraphNode();

    bool isDead() const;
    virtual void setDead(bool dead);

    virtual bool isDisplayed() const;

    bool isHovered() const;
    void setHovered(bool hovered);

    uint32_t getGraphicId() const;

    virtual Type getGraphType() const;
    virtual ElementType getType() const;
    virtual TreeType getDefaultTreeType() const;
    std::vector<TreeType> getArboTreeTypes() const;

    virtual void setDefaultData(const Controller& controller) override;
    virtual MeshDrawData getMeshDrawData(const glm::dmat4& gTransfo) const;

    glm::dvec3 getTranslation(const bool& cumulated = true) const;
    glm::dquat getRotation(const bool& cumulated = true) const;
    glm::dvec3 getScale(const bool& cumulated = true) const;

    virtual glm::dmat4 getCumulatedTransformation() const;
    TransformationModule getCumulTransformationModule() const;

    void manipulateTransfo(const ManipulateData& manipData);
    virtual bool isManipulable(ManipulationMode mode) const;
    virtual std::unordered_set<Selection> getAcceptableSelections(ManipulationMode mode) const;

    virtual bool isAcceptableOwningChild(const SafePtr<AGraphNode>& child) const;

    ///Links
    //Applique de manière récursive en profondeur (à partir du currentPtr en suivant le getterSet) 
    //une action recAction (de base vide, si elle retourne faux ça ne va pas plus loin dans la récursion). 
    // 
    //Les noeuds visités sont dans visitedNodes.
    static void recOnAncestors(const SafePtr<AGraphNode>& currentPtr, std::function<std::unordered_set<SafePtr<AGraphNode>>(const SafePtr<AGraphNode>&)> getterSet, std::unordered_set<SafePtr<AGraphNode>>& visitedNodes, std::function<bool(const SafePtr<AGraphNode>&)> recAction = [](const SafePtr<AGraphNode>&) {return true; });
    static void recOnAncestors_read(const SafePtr<AGraphNode>& currentPtr, std::function<std::unordered_set<SafePtr<AGraphNode>>(const ReadPtr<AGraphNode>&)> getterSet, std::unordered_set<SafePtr<AGraphNode>>& visitedNodes, std::function<bool(const SafePtr<AGraphNode>&)> recAction = [](const SafePtr<AGraphNode>&) {return true; });


    //GeometricLinks
    static SafePtr<AGraphNode> getGeometricParent(const SafePtr<AGraphNode>& child);
    static std::unordered_set<SafePtr<AGraphNode>> getGeometricChildren_rec(const SafePtr<AGraphNode>& parent);
    static std::unordered_set<SafePtr<AGraphNode>> getGeometricChildren(const SafePtr<AGraphNode>& parent);
    static std::unordered_set<SafePtr<AGraphNode>> getGeometricChildren_read(const ReadPtr<AGraphNode>& parent);

    static bool addGeometricLink(const SafePtr<AGraphNode>& parent, const SafePtr<AGraphNode>& child);
    static void duplicateLink(const SafePtr<AGraphNode>& copyNode, const AGraphNode& dataToCopy);
    static bool removeGeometricLink(const SafePtr<AGraphNode>& parent, const SafePtr<AGraphNode>& child);
    static void cleanGeometricLinks(const SafePtr<AGraphNode>& nodeId);
    static void cleanLinks(const SafePtr<AGraphNode>& nodeId);

    //OwningLinks
    SafePtr<AGraphNode> getOwningObjectParent() const;
    static SafePtr<AGraphNode> getOwningParent(const SafePtr<AGraphNode>& child, TreeType type = TreeType::RawData);
    static std::unordered_set<SafePtr<AGraphNode>> getOwningParent_rec(const SafePtr<AGraphNode>& child, TreeType type = TreeType::RawData);
    static std::unordered_set<SafePtr<AGraphNode>> getOwningParents(const SafePtr<AGraphNode>& child);
    static std::unordered_set<SafePtr<AGraphNode>> getOwningParents_rec(const SafePtr<AGraphNode>& child);
    static std::unordered_set<SafePtr<AGraphNode>> getOwningChildren(const SafePtr<AGraphNode>& parent);
    static std::unordered_set<SafePtr<AGraphNode>> getOwningChildren_rec(const SafePtr<AGraphNode>& parent);

    static bool isOwningAncestor(const SafePtr<AGraphNode>& ancestor, const SafePtr<AGraphNode>& node);

    static bool addOwningLink(const SafePtr<AGraphNode>& parent, const SafePtr<AGraphNode>& child);
    static bool removeOwningLink(const SafePtr<AGraphNode>& parent, const SafePtr<AGraphNode>& child);

    static void cleanOwningLinks(const SafePtr<AGraphNode>& nodeId);

protected:
    std::unordered_set<SafePtr<AGraphNode>> m_geometricChildren;
    SafePtr<AGraphNode>						m_geometricParent = SafePtr<AGraphNode>();

    std::unordered_set<SafePtr<AGraphNode>> m_owningChildren;
    SafePtr<AGraphNode>						m_owningObjectParent;
    SafePtr<AGraphNode>						m_owningPipingParent;
    SafePtr<AGraphNode>						m_owningHierarchyParent;

    bool is_hovered_ = false;
    bool is_dead_ = false;
    uint32_t graphic_id_ = 0;

private:
    static IdGiver<uint32_t> graphic_id_giver;
};

#endif //! AGRAPH_NODE_H_