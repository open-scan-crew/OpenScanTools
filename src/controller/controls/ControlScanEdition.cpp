#include "controller/controls/ControlScanEdition.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "pointCloudEngine/PCE_core.h"

#include "models/3d/Graph/GraphManager.hxx"
#include "models/3d/Graph/ScanNode.h"
#include "controller/controls/AEditionControl.hxx"

#include "utils/ColorConversion.h"

#include <glm/glm.hpp>


namespace control::scanEdition
{
    /*
    ** SetClippable
    */

    SetClippable::SetClippable(SafePtr<APointCloudNode> toEditData, bool clippable)
        : ATEditionControl({ toEditData }, clippable, "SetClippable", &APointCloudNode::setClippable, &APointCloudNode::getClippable)
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
            //pas obligé d'être un ScanNode ici
            WritePtr<ScanNode> wScan = static_pointer_cast<ScanNode>(nodePtr).get();
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

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

    ChangeScanGuid::ChangeScanGuid(SafePtr<ScanNode> toEditData, const tls::ScanGuid& newGuid)
        : m_toEditData(toEditData)
        , m_newGuid(newGuid)
    {}

    ChangeScanGuid::~ChangeScanGuid()
    {}

    void ChangeScanGuid::doFunction(Controller& controller)
    {
        WritePtr<ScanNode> writePtr = m_toEditData.get();
        if (!writePtr)
            return;
        if (m_newGuid == xg::Guid())
        {
            // NOTE - On choisi de ne pas supprimer le fichier au cas où cela soit une mauvaise manipulation.
            tlFreeScan(writePtr->getScanGuid());
            return;
        }
        else
        {
            std::filesystem::path absolutePath = writePtr->getCurrentScanPath();
            tlFreeScan(writePtr->getScanGuid());
            writePtr->setScanGuid(m_newGuid);
            doTimeModified(*&writePtr);
            tlCopyScanFile(m_newGuid, absolutePath, false, true, true);
        }
    }

    bool ChangeScanGuid::canUndo() const
    {
        return (false);
    }

    void ChangeScanGuid::undoFunction(Controller& controller)
    {}

    ControlType ChangeScanGuid::getType() const
    {
        return ControlType::changeScanGuid;
    }
}
