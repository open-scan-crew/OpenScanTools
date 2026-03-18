#include "controller/controls/ControlPicking.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/IControlListener.h"
#include "controller/controls/ControlFunction.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/FullClickMessage.h"
#include "gui/GuiData/GuiDataMeasure.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/UnitConverter.h"
#include "gui/texts/ContextTexts.hpp"
#include "utils/Logger.h"
#include "utils/ProjectColor.hpp"

#include "models/graph/BoxNode.h"
#include "models/graph/GraphManager.h"
#include "models/graph/TagNode.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "pointCloudEngine/OctreeRayTracing.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "tls_def.h"

#include <QObject>
#include <limits>
#include <vector>

namespace control::picking
{
    namespace
    {
        GeometricBox makeGeometricBoxFromBoundingBox(const BoundingBoxD& bbox)
        {
            std::vector<glm::dvec3> corners;
            corners.reserve(8);
            corners.push_back({ bbox.xMin, bbox.yMin, bbox.zMin });
            corners.push_back({ bbox.xMax, bbox.yMin, bbox.zMin });
            corners.push_back({ bbox.xMin, bbox.yMax, bbox.zMin });
            corners.push_back({ bbox.xMin, bbox.yMin, bbox.zMax });
            corners.push_back({ bbox.xMax, bbox.yMax, bbox.zMin });
            corners.push_back({ bbox.xMax, bbox.yMin, bbox.zMax });
            corners.push_back({ bbox.xMin, bbox.yMax, bbox.zMax });
            corners.push_back({ bbox.xMax, bbox.yMax, bbox.zMax });
            return GeometricBox(corners);
        }

        SafePtr<TagNode> createTemperatureTag(Controller& controller, const PointXYZIRGB& point, double temperature, const char* colorName)
        {
            SafePtr<TagNode> tagNode = make_safe<TagNode>();
            WritePtr<TagNode> wTag = tagNode.get();
            if (!wTag)
                return SafePtr<TagNode>();

            const int digits = controller.cgetContext().m_unitUsage.displayedDigits;
            const QString text = QStringLiteral("%1%2")
                                     .arg(QString::number(temperature, 'f', digits), UnitConverter::getTemperatureUnitText());

            wTag->setDefaultData(controller);
            wTag->setVisible(true);
            wTag->setPosition(glm::dvec3(point.x, point.y, point.z));
            wTag->setMarkerIcon(scs::MarkerIcon::Tag_Base);
            wTag->setColor(ProjectColor::getColor(colorName));
            wTag->setName(text.toStdWString());
            return tagNode;
        }
    }

    //
    // Click
    //

    Click::Click(const ClickInfo& info)
		: m_clickInfo(info)
    {}

    Click::~Click()
    {}

    void Click::doFunction(Controller& controller)
    {
        CONTROLLOG << "Click detected : " << m_clickInfo.picking << Logger::endl;
        // TODO(robin) - Vérifier que les points sont bien envoyés à 'PropertyClippingSettings' et 'PropertyUserOrientation'
        if (controller.getFunctionManager().isActiveContext() == ContextType::none)
        {
            if (m_clickInfo.ctrl)
            {
                GraphManager& graphManager = controller.getGraphManager();
                std::unordered_set<SafePtr<AGraphNode>> selected(graphManager.getSelectedNodes());

                {
                    ReadPtr<AGraphNode> node = m_clickInfo.hover.cget();
                    if (node) {
                        if (node->isSelected())
                            selected.erase(m_clickInfo.hover);
                        else
                            selected.insert(m_clickInfo.hover);
                    }
                }

                controller.changeSelection(selected);
            }
            else
            {
                if(m_clickInfo.hover)
                    controller.changeSelection({ m_clickInfo.hover });
            }

			controller.updateInfo(new GuiDataPoint(m_clickInfo.picking));
			controller.updateInfo(new GuiDataRenderTargetClick());
		}
        else
        {
            FullClickMessage message(m_clickInfo);
            controller.getFunctionManager().feedMessage(controller, &message);
        }
    }

    bool Click::canUndo() const
    {
        return (false);
    }

    void Click::undoFunction(Controller& controller)
    { }

    ControlType Click::getType() const
    {
        return (ControlType::clickPicking);
    }

    FindScanFromPick::FindScanFromPick()
    {
    }

    FindScanFromPick::~FindScanFromPick()
    {
    }

    void FindScanFromPick::doFunction(Controller& controller)
    {
        controller.getFunctionManager().launchFunction(controller, ContextType::findScan);
    }

    bool FindScanFromPick::canUndo() const
    {
        return false;
    }

    void FindScanFromPick::undoFunction(Controller& controller)
    {}

    ControlType FindScanFromPick::getType() const
    {
        return ControlType::findScanFromPicking;
    }

    PickTemperatureFromPick::PickTemperatureFromPick()
    {
    }

    PickTemperatureFromPick::~PickTemperatureFromPick()
    {
    }

    void PickTemperatureFromPick::doFunction(Controller& controller)
    {
        controller.getFunctionManager().launchFunction(controller, ContextType::pickTemperature);
    }

    bool PickTemperatureFromPick::canUndo() const
    {
        return false;
    }

    void PickTemperatureFromPick::undoFunction(Controller& controller)
    {
    }

    ControlType PickTemperatureFromPick::getType() const
    {
        return ControlType::pickTemperatureFromPicking;
    }

    PickColorimetricFromPick::PickColorimetricFromPick()
    {
    }

    PickColorimetricFromPick::~PickColorimetricFromPick()
    {
    }

    void PickColorimetricFromPick::doFunction(Controller& controller)
    {
        controller.getFunctionManager().launchFunction(controller, ContextType::pickColorimetric);
    }

    bool PickColorimetricFromPick::canUndo() const
    {
        return false;
    }

    void PickColorimetricFromPick::undoFunction(Controller& controller)
    {
    }

    ControlType PickColorimetricFromPick::getType() const
    {
        return ControlType::pickColorimetricFromPicking;
    }

    FindMinMaxTemperatureInClipping::FindMinMaxTemperatureInClipping()
    {
    }

    FindMinMaxTemperatureInClipping::~FindMinMaxTemperatureInClipping()
    {
    }

    void FindMinMaxTemperatureInClipping::doFunction(Controller& controller)
    {
        const GraphManager& graphManager = controller.cgetGraphManager();
        const TemperatureScaleData temperatureScale = graphManager.getTemperatureScaleData();
        if (!temperatureScale.isValid || temperatureScale.rgbToTemperature.empty())
        {
            controller.updateInfo(new GuiDataWarning(QObject::tr("You must import a valid temperature scale file first.")));
            return;
        }

        const std::unordered_set<SafePtr<AClippingNode>> clippings = graphManager.getClippingObjects(true, false);
        if (clippings.size() != 1)
        {
            controller.updateInfo(new GuiDataWarning(TEXT_TEMPERATURE_MINMAX_NEEDS_ONE_INTERIOR_CLIP));
            return;
        }

        const SafePtr<AClippingNode>& clipping = *clippings.begin();
        ReadPtr<AClippingNode> rClip = clipping.cget();
        if (!rClip
            || rClip->getType() != ElementType::Box
            || rClip->getClippingMode() != ClippingMode::showInterior
            || rClip->isVisible())
        {
            controller.updateInfo(new GuiDataWarning(TEXT_TEMPERATURE_MINMAX_NEEDS_ONE_INTERIOR_CLIP));
            return;
        }

        controller.updateInfo(new GuiDataProcessingSplashScreenStart(1, QObject::tr("Find min max temperature"), QObject::tr("Please wait...")));
        controller.updateInfo(new GuiDataProcessingSplashScreenEnableCancelButton(false));

        ClippingAssembly clippingAssembly;
        graphManager.getClippingAssembly(clippingAssembly, true, false);

        TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(tls::ScanGuid(), true, true));

        const BoundingBoxD visibleScansBBox = graphManager.getScanBoundingBox(ObjectStatusFilter::VISIBLE);
        if (!visibleScansBBox.isValid())
        {
            controller.updateInfo(new GuiDataProcessingSplashScreenEnd(QObject::tr("Done")));
            controller.updateInfo(new GuiDataWarning(QObject::tr("No visible scan found in the project.")));
            return;
        }

        const GeometricBox searchBox = makeGeometricBoxFromBoundingBox(visibleScansBBox);
        std::vector<PointXYZIRGB> points;
        TlScanOverseer::getInstance().collectPointsInGeometricBox(searchBox, clippingAssembly, tls::ScanGuid(), points);

        bool minFound = false;
        bool maxFound = false;
        double minTemperature = std::numeric_limits<double>::max();
        double maxTemperature = std::numeric_limits<double>::lowest();
        PointXYZIRGB minPoint{};
        PointXYZIRGB maxPoint{};

        for (const PointXYZIRGB& point : points)
        {
            const uint32_t key = makeTemperatureScaleKey(point.r, point.g, point.b);
            auto it = temperatureScale.rgbToTemperature.find(key);
            if (it == temperatureScale.rgbToTemperature.end())
                continue;

            const double value = it->second;
            if (!minFound || value < minTemperature)
            {
                minFound = true;
                minTemperature = value;
                minPoint = point;
            }
            if (!maxFound || value > maxTemperature)
            {
                maxFound = true;
                maxTemperature = value;
                maxPoint = point;
            }
        }

        controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(QObject::tr("Done"), 1));
        controller.updateInfo(new GuiDataProcessingSplashScreenEnd(QObject::tr("Done")));

        if (!minFound || !maxFound)
        {
            controller.updateInfo(new GuiDataWarning(QObject::tr("No temperature from the current scale was found in the clipped area.")));
            return;
        }

        SafePtr<TagNode> minTag = createTemperatureTag(controller, minPoint, minTemperature, "BLUE");
        SafePtr<TagNode> maxTag = createTemperatureTag(controller, maxPoint, maxTemperature, "RED");

        std::unordered_set<SafePtr<AGraphNode>> tagsToAdd;
        if (minTag)
            tagsToAdd.insert(minTag);
        if (maxTag)
            tagsToAdd.insert(maxTag);

        if (!tagsToAdd.empty())
            controller.getControlListener()->notifyUIControl(new control::function::AddNodes(tagsToAdd));
    }

    bool FindMinMaxTemperatureInClipping::canUndo() const
    {
        return false;
    }

    void FindMinMaxTemperatureInClipping::undoFunction(Controller& controller)
    {
    }

    ControlType FindMinMaxTemperatureInClipping::getType() const
    {
        return ControlType::pickMinMaxTemperatureFromClipping;
    }

}
