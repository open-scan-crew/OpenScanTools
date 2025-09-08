#include "controller/controls/ControlScanEdition.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"

#include "models/graph/GraphManager.h"
#include "models/graph/PointCloudNode.h"
#include "controller/controls/AEditionControl.hxx"

#include "utils/ColorConversion.h"

#include <glm/glm.hpp>


namespace control::scanEdition
{
    /*
    ** SetClippable
    */

    SetClippable::SetClippable(SafePtr<PointCloudNode> toEditData, bool clippable)
        : ATEditionControl({ toEditData }, clippable, "SetClippable", &PointCloudNode::setClippable, &PointCloudNode::getClippable)
    {}

    SetClippable::~SetClippable()
    {}

    ControlType SetClippable::getType() const
    {
        return (ControlType::setScanClippable);
    }

    /*
    ** RandomScansColors
    */

    RandomScansColors::RandomScansColors()
    {}

    RandomScansColors::~RandomScansColors()
    {}

    void RandomScansColors::doFunction(Controller& controller)
    {
        time_t seed_t;
        time(&seed_t);
        srand(seed_t * 239);

        GraphManager& graphManager = controller.getGraphManager();
        std::unordered_set<SafePtr<AGraphNode>> scanToEdit = graphManager.getNodesByTypes({ ElementType::Scan });
        // On prend une base assez grande (>240) pour faire les modulo de hue et saturation.
        int base = 1031;
        for (const SafePtr<AGraphNode>& nodePtr : scanToEdit)
        {
            WritePtr<PointCloudNode> wScan = static_pointer_cast<PointCloudNode>(nodePtr).get();
            if (!wScan)
                continue;

            // On souhaite une saturation random entre [0.66, 1.0]
            float rnd_sat = 1.0 - (rand() % base / 3) / (float)base;
            glm::vec3 hsl_color(rand() % base / (float)base, rnd_sat, 0.5f);
            wScan->setColor(Color32(utils::color::hsl2rgb(hsl_color)));
            wScan->setModificationTime(std::time(nullptr));
        }

        controller.getContext().setIsCurrentProjectSaved(false);

        controller.actualizeTreeView(scanToEdit);
    }

    bool RandomScansColors::canUndo() const
    {
        return (false);
    }

    void RandomScansColors::undoFunction(Controller& controller)
    {}

    ControlType RandomScansColors::getType() const
    {
        return ControlType::randomScansColors;
    }
}
