#ifndef CONTROL_CLIPPING_EDITION_H_
#define CONTROL_CLIPPING_EDITION_H_

#include "controller/controls/AEditionControl.h"
#include "models/data/Clipping/ClippingData.h"
#include "models/data/Grid/GridData.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <unordered_set>

class BoxNode;
class AClippingNode;


namespace control::clippingEdition
{
    class SetMode : public ATEditionControl<AClippingNode, ClippingMode>
    {
    public:
        SetMode(const ClippingMode& mode, bool filterIsActive, bool filterIsSelected);
        SetMode(SafePtr<AClippingNode> toEditData, ClippingMode newMode);
        SetMode(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, const ClippingMode& newMode);

        ~SetMode();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;

    private:
        ClippingMode m_mode;
        bool m_filterIsSelected = false;
        bool m_filterIsActive = false;
    };

    class SetClipActive : public ATEditionControl<AClippingNode, bool>
    {
    public:
        SetClipActive(bool active, bool filterClipActive, bool filterSelected);
        SetClipActive(SafePtr<AClippingNode> toEditData, bool active);
        SetClipActive(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, bool active);
        ~SetClipActive();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;

    private:
        bool m_active;
        bool m_filterSelected = false;
        bool m_filterClipActive = false;
    };

    class SetRampActive : public ATEditionControl<AClippingNode, bool>
    {
    public:
        SetRampActive(bool active, bool filterRampActive, bool filterSelected);
        SetRampActive(SafePtr<AClippingNode> toEditData, bool active);
        SetRampActive(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, bool active);
        ~SetRampActive() {}
        void doFunction(Controller& controller) override;
        ControlType getType() const override;

    private:
        bool m_active;
        bool m_filterSelected = false;
        bool m_filterRampActive = false;
    };

    class SetMinClipDist : public ATEditionControl<AClippingNode, float>
    {
    public:
        SetMinClipDist(SafePtr<AClippingNode> toEditData, float userClippingValue);
        SetMinClipDist(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, float userClippingValue);
        ~SetMinClipDist();
        ControlType getType() const override;
    };

    class SetMaxClipDist : public ATEditionControl<AClippingNode, float>
    {
    public:
        SetMaxClipDist(SafePtr<AClippingNode> toEditData, float userClippingValue);
        SetMaxClipDist(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, float userClippingValue);
        ~SetMaxClipDist();
        ControlType getType() const override;
    };

    class SetDefaultMinClipDistance : public AControl
    {
    public:
        SetDefaultMinClipDistance(float value);
        ~SetDefaultMinClipDistance();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        float m_value;
    };

    class SetDefaultMaxClipDistance : public AControl
    {
    public:
        SetDefaultMaxClipDistance(float value);
        ~SetDefaultMaxClipDistance();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        float m_value;
    };

    class SetDefaultClippingMode : public AControl
    {
    public:
        SetDefaultClippingMode(ClippingMode mode);
        ~SetDefaultClippingMode();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        ClippingMode m_mode;
    };

    class SetDefaultRampValues : public AControl
    {
    public:
        SetDefaultRampValues(float _min, float _max, int _steps);
        ~SetDefaultRampValues();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        float m_min;
        float m_max;
        int m_steps;
        // bool m_clamped; // TODO - add the GUI
    };

    class SwitchBoxGrid : public ATEditionControl<BoxNode, bool>
    {
    public:
        SwitchBoxGrid(SafePtr<BoxNode> toEditData, bool upgrade);
        SwitchBoxGrid(const std::unordered_set<SafePtr<BoxNode>>& toEditDatas, bool upgrade);
        ~SwitchBoxGrid();
        ControlType getType() const override;
    };

    class GridDivisionType : public ATEditionControl<BoxNode, GridType>
    {
    public:
        GridDivisionType(SafePtr<BoxNode> toEditData, const GridType& type);
        GridDivisionType(const std::unordered_set<SafePtr<BoxNode>>& toEditDatas, const GridType& type);
        ~GridDivisionType();
        ControlType getType() const override;
    };

    class GridChangeValue : public ATEditionControl<BoxNode, glm::vec3>
    {
    public:
        GridChangeValue(SafePtr<BoxNode> toEditData, const glm::vec3& value);
        GridChangeValue(const std::unordered_set<SafePtr<BoxNode>>& toEditDatas, const glm::vec3& value);
        ~GridChangeValue();
        ControlType getType() const override;
    };

    class SetMaxRamp : public ATEditionControl<AClippingNode, double>
    {
    public:
        SetMaxRamp(SafePtr<AClippingNode> toEditData, double max);
        SetMaxRamp(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, double max);
        ~SetMaxRamp() {}
        ControlType getType() const override;
    };

    class SetMinRamp : public ATEditionControl<AClippingNode, double>
    {
    public:
        SetMinRamp(SafePtr<AClippingNode> toEditData, double min);
        SetMinRamp(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, double min);
        ~SetMinRamp() {}
        ControlType getType() const override;
    };

    class SetRampSteps : public ATEditionControl<AClippingNode, int>
    {
    public:
        SetRampSteps(SafePtr<AClippingNode> toEditData, int step);
        SetRampSteps(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, int step);
        ~SetRampSteps() {}
        ControlType getType() const override;
    };

    class SetRampClamped : public ATEditionControl<AClippingNode, int>
    {
    public:
        SetRampClamped(SafePtr<AClippingNode> toEditData, bool clamped);
        SetRampClamped(const std::unordered_set<SafePtr<AClippingNode>>& toEditDatas, bool clamped);
        ~SetRampClamped() {}
        ControlType getType() const override;
    };
}
#endif //CONTROL_CLIPPING_EDITION_H_