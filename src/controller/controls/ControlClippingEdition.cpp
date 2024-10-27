#include "controller/Controls/ControlClippingEdition.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/controls/AEditionControl.hxx"

#include "pointCloudEngine/RenderingLimits.h"

#include "models/graph/GraphManager.h"
#include "models/graph/BoxNode.h"

#include "gui/Texts.hpp"

#include "utils/Logger.h"

namespace control::clippingEdition
{
    /*
    ** SetMode
    */

    SetMode::SetMode(const ClippingMode& mode, bool filterIsActive, bool filterIsSelected)
        : ATEditionControl("SetMode", & ClippingData::setClippingMode, & ClippingData::getClippingMode)
    {
        m_mode = mode;
        m_filterIsActive = filterIsActive;
        m_filterIsSelected = filterIsSelected;
    }

    SetMode::SetMode(SafePtr<AClippingNode> toEditData, ClippingMode mode)
        : ATEditionControl({ toEditData }, mode, "SetMode", & ClippingData::setClippingMode, & ClippingData::getClippingMode)
    {
        m_actualize_tree_view = true;
    }

    SetMode::SetMode(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, const ClippingMode& mode)
        : ATEditionControl(toEditDatas, mode, "SetMode", &ClippingData::setClippingMode, &ClippingData::getClippingMode)
    {
        m_actualize_tree_view = true;
    }

    SetMode::~SetMode()
    {
    }

    void SetMode::doFunction(Controller& controller)
    {
        if (m_filterIsActive || m_filterIsSelected)
        {
            GraphManager& graphManager = controller.getGraphManager();
            setToEditData(graphManager.getClippingObjects(m_filterIsActive, m_filterIsSelected), m_mode);
        }
        doSimpleEdition(controller);
    }

    ControlType SetMode::getType() const
    {
        return ControlType::setClippingModeEdit;
    }

    /*
    ** SetActive
    */

    SetClipActive::SetClipActive(bool active, bool filterClipActive, bool filterSelected)
        : ATEditionControl("SetClipActive", & ClippingData::setClippingActive, & ClippingData::isClippingActive)
    {
        m_active = active;
        m_filterClipActive = filterClipActive;
        m_filterSelected = filterSelected;

        m_actualize_tree_view = true;
    }

    SetClipActive::SetClipActive(SafePtr<AClippingNode> toEditData, bool active)
        : ATEditionControl({ toEditData }, active, "SetClipActive", & ClippingData::setClippingActive, & ClippingData::isClippingActive)
    {
        m_actualize_tree_view = true;
    }

    SetClipActive::SetClipActive(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, bool active)
        : ATEditionControl(toEditDatas, active, "SetClipActive", &ClippingData::setClippingActive, &ClippingData::isClippingActive)
    {
        m_actualize_tree_view = true;
    }

    SetClipActive::~SetClipActive()
    {}

    void SetClipActive::doFunction(Controller& controller)
    {
        GraphManager& graphManager = controller.getGraphManager();

        uint32_t clipCount = graphManager.getActiveClippingAndRampCount();

        if (m_filterClipActive || m_filterSelected)
            setToEditData(graphManager.getClippingObjects(m_filterClipActive, m_filterSelected), m_active);

        m_cannotEditWarningMessage = TEXT_ACTIVE_CLIPPING_RAMP_MAX_REACHED.arg(MAX_ACTIVE_CLIPPING);

        setEditCondition([this, clipCount](const AClippingNode& node, bool newValue)
        {
             return (!newValue || (clipCount + m_oldValues.size() < MAX_ACTIVE_CLIPPING));
        }
        );
        doSimpleEdition(controller);
    }

    ControlType SetClipActive::getType() const
    {
        return (ControlType::setClippingActiveEdit);
    }

    /*
    ** SetRampActive
    */

    SetRampActive::SetRampActive(bool active, bool filterRampActive, bool filterSelected)
        : ATEditionControl("SetRampActive", & AClippingNode::setRampActive, & AClippingNode::isRampActive)
    {
        m_active = active;
        m_filterRampActive = filterRampActive;
        m_filterSelected = filterSelected;
    }

    SetRampActive::SetRampActive(SafePtr<AClippingNode> toEditData, bool active)
        : ATEditionControl({ toEditData }, active, "SetRampActive", & AClippingNode::setRampActive, & AClippingNode::isRampActive)
    {
    }

    SetRampActive::SetRampActive(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, bool active)
        : ATEditionControl(toEditDatas, active, "SetRampActive", &AClippingNode::setRampActive, &AClippingNode::isRampActive)
    {
    }

    void SetRampActive::doFunction(Controller& controller)
    {
        GraphManager& graphManager = controller.getGraphManager();

        uint32_t clipCount = graphManager.getActiveClippingAndRampCount();

        if (m_filterRampActive || m_filterSelected)
            setToEditData(graphManager.getRampObjects(m_filterRampActive, m_filterSelected), m_active);

        m_cannotEditWarningMessage = TEXT_ACTIVE_CLIPPING_RAMP_MAX_REACHED.arg(MAX_ACTIVE_CLIPPING);

        setEditCondition([this, clipCount](const AClippingNode& node, bool newValue)
            {
                return (!newValue || (clipCount + m_oldValues.size() < MAX_ACTIVE_CLIPPING));
            }
        );
        doSimpleEdition(controller);
    }

    ControlType SetRampActive::getType() const
    {
        return ControlType::activeRamp;
    }

    /*
    ** SetMinClipDist
    */

    SetMinClipDist::SetMinClipDist(SafePtr<AClippingNode> clipping, float userClippingValue)
        : ATEditionControl({ clipping }, userClippingValue, "SetMinClipDist", & ClippingData::setMinClipDist, & ClippingData::getMinClipDist)
    {
    }

    SetMinClipDist::SetMinClipDist(const std::unordered_set<SafePtr<AClippingNode>>& clippings, float userClippingValue)
        : ATEditionControl(clippings, userClippingValue, "SetMinClipDist", &ClippingData::setMinClipDist, &ClippingData::getMinClipDist)
    {
    }

    SetMinClipDist::~SetMinClipDist()
    {
    }

    ControlType SetMinClipDist::getType() const
    {
        return ControlType::setMinClipDistance;
    }

    /*
    ** SetMaxClipDist
    */

    SetMaxClipDist::SetMaxClipDist(SafePtr<AClippingNode> clipping, float userClippingValue)
        : ATEditionControl({ clipping }, userClippingValue, "SetMaxClipDist", & ClippingData::setMaxClipDist, & ClippingData::getMaxClipDist)
    {
    }

    SetMaxClipDist::SetMaxClipDist(const std::unordered_set<SafePtr<AClippingNode>>& clippings, float userClippingValue)
        : ATEditionControl(clippings, userClippingValue, "SetMaxClipDist", &ClippingData::setMaxClipDist, &ClippingData::getMaxClipDist)
    {
    }

    SetMaxClipDist::~SetMaxClipDist()
    {
    }

    ControlType SetMaxClipDist::getType() const
    {
        return ControlType::setMaxClipDistance;
    }

    /*
    ** SetDefaultMinClipDistance
    */

    SetDefaultMinClipDistance::SetDefaultMinClipDistance(float value)
        : m_value(value)
    {}

    SetDefaultMinClipDistance::~SetDefaultMinClipDistance()
    {}

    void SetDefaultMinClipDistance::doFunction(Controller& controller)
    {
        ProjectInfos& info = controller.getContext().getProjectInfo();

        CONTROLLOG << "control::clippingEdition::SetUserClippingValueByType do : change value from " << info.m_defaultMinClipDistance << " to " << m_value << LOGENDL;
        info.m_defaultMinClipDistance = m_value;

        CONTROLLOG << "control::clippingEdition::SetUserClippingValueByType do : done" << LOGENDL;
    }

    ControlType SetDefaultMinClipDistance::getType() const
    {
        return (ControlType::setDefaultMinClipDistance);
    }

    /*
    ** SetDefaultMaxClipDistance
    */

    SetDefaultMaxClipDistance::SetDefaultMaxClipDistance(float value)
        : m_value(value)
    {}

    SetDefaultMaxClipDistance::~SetDefaultMaxClipDistance()
    {}

    void SetDefaultMaxClipDistance::doFunction(Controller& controller)
    {
        ProjectInfos& info = controller.getContext().getProjectInfo();

        CONTROLLOG << "control::clippingEdition::SetUserClippingValueByType do : change value from " << info.m_defaultMaxClipDistance << " to " << m_value << LOGENDL;
        info.m_defaultMaxClipDistance = m_value;

        CONTROLLOG << "control::clippingEdition::SetUserClippingValueByType do : done" << LOGENDL;
    }

    ControlType SetDefaultMaxClipDistance::getType() const
    {
        return (ControlType::setDefaultMaxClipDistance);
    }

    /*
    ** SetDefaultClippingMode
    */

    SetDefaultClippingMode::SetDefaultClippingMode(ClippingMode mode)
    {
        m_mode = mode;
    }

    SetDefaultClippingMode::~SetDefaultClippingMode()
    {
    }

    void SetDefaultClippingMode::doFunction(Controller& controller)
    {
        ProjectInfos& info = controller.getContext().getProjectInfo();
        info.m_defaultClipMode = m_mode;
    }

    ControlType SetDefaultClippingMode::getType() const
    {
        return ControlType::setDefaultClipMode;
    }

    /*
    ** SetDefaultRampValues
    */

    SetDefaultRampValues::SetDefaultRampValues(float _min, float _max, int _steps)
        : m_min(_min)
        , m_max(_max)
        , m_steps(_steps)
    {}

    SetDefaultRampValues::~SetDefaultRampValues()
    {}

    void SetDefaultRampValues::doFunction(Controller& controller)
    {
        ProjectInfos& info = controller.getContext().getProjectInfo();
        info.m_defaultMinRampDistance = m_min;
        info.m_defaultMaxRampDistance = m_max;
        info.m_defaultRampSteps = m_steps;
    }

    ControlType SetDefaultRampValues::getType() const
    {
        return ControlType::setDefaultRampValues;
    }

    /*
    ** SwitchBoxGrid
    */

    SwitchBoxGrid::SwitchBoxGrid(SafePtr<BoxNode> box, bool upgrade)
        : ATEditionControl({ box }, upgrade, "SwitchBoxGrid", & BoxNode::setIsSimpleBox, & BoxNode::isSimpleBox)
    {
        m_actualize_tree_view = true;
    }

    SwitchBoxGrid::SwitchBoxGrid(const std::unordered_set<SafePtr<BoxNode>>& boxs, bool upgrade)
        : ATEditionControl(boxs, upgrade, "SwitchBoxGrid", &BoxNode::setIsSimpleBox, &BoxNode::isSimpleBox)
    {
        m_actualize_tree_view = true;
    }

    SwitchBoxGrid::~SwitchBoxGrid()
    {}

    ControlType SwitchBoxGrid::getType() const
    {
        return (ControlType::setClippingBoxExtend);
    }

    /*
    ** GridDivisionType
    */

    GridDivisionType::GridDivisionType(SafePtr<BoxNode> box, GridType type)
        : ATEditionControl({ box }, type, "GridDivisionType", & BoxNode::setGridType, & BoxNode::getGridType)
    {
    }

    GridDivisionType::GridDivisionType(const std::unordered_set<SafePtr<BoxNode>>& boxs, GridType type)
        : ATEditionControl(boxs, type, "GridDivisionType", &BoxNode::setGridType, &BoxNode::getGridType)
    {
    }

    GridDivisionType::~GridDivisionType()
    {}

    ControlType GridDivisionType::getType() const
    {
        return (ControlType::setGridType);
    }

    /*
    ** GridChangeValue
    */

    GridChangeValue::GridChangeValue(SafePtr<BoxNode> box, const glm::vec3& value)
        : ATEditionControl({ box }, value, "GridChangeValue", & BoxNode::setGridDivision, & BoxNode::getGridDivision)
    {
    }

    GridChangeValue::GridChangeValue(const std::unordered_set<SafePtr<BoxNode>>& boxs, const glm::vec3& value)
        : ATEditionControl(boxs, value, "GridChangeValue", &BoxNode::setGridDivision, &BoxNode::getGridDivision)
    {
    }

    GridChangeValue::~GridChangeValue()
    {}

    ControlType  GridChangeValue::getType() const
    {
        return (ControlType::setGridValue);
    }

    /*
    ** SetMaxRamp
    */

    SetMaxRamp::SetMaxRamp(SafePtr<AClippingNode> toEditData, double max)
        : ATEditionControl({ toEditData }, max, "SetMaxRamp", & AClippingNode::setRampMax, & AClippingNode::getRampMax)
    {
    }
    SetMaxRamp::SetMaxRamp(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, double max)
        : ATEditionControl(toEditDatas, max, "SetMaxRamp", &AClippingNode::setRampMax, &AClippingNode::getRampMax)
    {
    }
    ControlType SetMaxRamp::getType() const
    {
        return ControlType::setMaxRamp;
    }

    /*
    ** SetMinRamp
    */

    SetMinRamp::SetMinRamp(SafePtr<AClippingNode> toEditData, double min)
        : ATEditionControl({ toEditData }, min, "SetMinRamp", & AClippingNode::setRampMin, & AClippingNode::getRampMin)
    {
    }
    SetMinRamp::SetMinRamp(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, double min)
        : ATEditionControl(toEditDatas, min, "SetMinRamp", &AClippingNode::setRampMin, &AClippingNode::getRampMin)
    {
    }
    ControlType SetMinRamp::getType() const
    {
        return ControlType::setMinRamp;
    }

    /*
    ** SetRampSteps
    */

    SetRampSteps::SetRampSteps(SafePtr<AClippingNode> toEditData, int step)
        : ATEditionControl({ toEditData }, step, "SetRampSteps", & AClippingNode::setRampSteps, & AClippingNode::getRampSteps)
    {
    }
    SetRampSteps::SetRampSteps(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, int step)
        : ATEditionControl(toEditDatas, step, "SetRampSteps", &AClippingNode::setRampSteps, &AClippingNode::getRampSteps)
    {
    }
    ControlType SetRampSteps::getType() const
    {
        return ControlType::setRampSteps;
    }


    /*
    ** SetRampClamped
    */

    SetRampClamped::SetRampClamped(SafePtr<AClippingNode> toEditData, bool clamped)
        : ATEditionControl({ toEditData }, clamped, "SetRampClamped", & AClippingNode::setRampClamped, & AClippingNode::isRampClamped)
    {
    }
    SetRampClamped::SetRampClamped(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, bool clamped)
        : ATEditionControl(toEditDatas, clamped, "SetRampClamped", &AClippingNode::setRampClamped, &AClippingNode::isRampClamped)
    {
    }
    ControlType SetRampClamped::getType() const
    {
        return ControlType::setRampClamped;
    }
}