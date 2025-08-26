#ifndef CONTROL_FUNCTION_DUPLICATION_H_
#define CONTROL_FUNCTION_DUPLICATION_H_

#include "controller/controls/IControl.h"
#include "models/3d/DuplicationTypes.h"

namespace control::duplication
{
    class SetDuplicationMode : public AControl
    {
    public:
        SetDuplicationMode(const DuplicationMode& type);
        ~SetDuplicationMode();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const DuplicationMode m_type;
    };

    class SetDuplicationStepSize : public AControl
    {
    public:
        SetDuplicationStepSize(const glm::ivec3& step);
        ~SetDuplicationStepSize();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const glm::ivec3 m_step;
    };

    class SetDuplicationOffsetValue : public AControl
    {
    public:
        SetDuplicationOffsetValue(const glm::vec3& offset);
        ~SetDuplicationOffsetValue();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const glm::vec3 m_offset;
    };

    class SetDuplicationIsLocal : public AControl
    {
    public:
        SetDuplicationIsLocal(const bool& isLocal);
        ~SetDuplicationIsLocal();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const bool m_isLocal;
    };
}

#endif //! CONTROL_FUNCTION_DUPLICATION_H_