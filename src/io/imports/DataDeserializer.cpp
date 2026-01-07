#include "io/imports/DataDeserializer.h"
#include "io/SerializerKeys.h"

#include "utils/Logger.h"

#include "models/application/UserOrientation.h"
#include "models/project/ProjectInfos.h"
#include "models/application/Author.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"

#include "utils/ProjectColor.hpp"

#include "models/graph/TagNode.h"
#include "models/graph/PointNode.h"

#include "models/graph/PointCloudNode.h"

#include "models/graph/ViewPointNode.h"
#include "models/graph/CameraNode.h"

#include "models/graph/CylinderNode.h"
#include "models/graph/SphereNode.h"
#include "models/graph/TorusNode.h"

#include "models/graph/SimpleMeasureNode.h"
#include "models/graph/PolylineMeasureNode.h"
#include "models/graph/ColumnTiltMeasureNode.h"
#include "models/graph/BeamBendingMeasureNode.h"
#include "models/graph/PointToPipeMeasureNode.h"
#include "models/graph/PointToPlaneMeasureNode.h"
#include "models/graph/PipeToPipeMeasureNode.h"
#include "models/graph/PipeToPlaneMeasureNode.h"

#include "models/graph/ClusterNode.h"
#include "models/graph/BoxNode.h"
#include "models/graph/MeshObjectNode.h"

#include "magic_enum/magic_enum.hpp"

#define IOLOG Logger::log(LoggerMode::IOLog)

bool ImportData(const nlohmann::json& json, Data& data)
{
    bool retVal(true);
    if (json.find(Key_Name) != json.end())
        data.setName(Utils::from_utf8(json.at(Key_Name).get<std::string>()));
    else
    {
        IOLOG << "ImportGenericData Name read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Description) != json.end())
        data.setDescription(Utils::from_utf8(json.at(Key_Description).get<std::string>()));
    else
    {
        IOLOG << "ImportGenericData Description read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Identifier) != json.end())
        data.setIdentifier(Utils::from_utf8(json.at(Key_Identifier).get<std::string>()));
    //Note (Aurélien)
    //Compatibility check
    else if (json.find("Prefix") != json.end())
        data.setIdentifier(Utils::from_utf8(json.at("Prefix").get<std::string>()));
    else
    {
        IOLOG << "ImportGenericData Identifier read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Discipline) != json.end())
        data.setDiscipline(Utils::from_utf8(json.at(Key_Discipline).get<std::string>()));
    else
    {
        IOLOG << "ImportGenericData Discipline read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_UserId) != json.end())
    {
        uint32_t guid(json.at(Key_UserId));
        data.setUserIndex(guid);
    }
    else
    {
        IOLOG << "ImportGenericData UserId read error" << LOGENDL;
        retVal = false;
    }


    if (json.find(Key_Id) != json.end())
    {
        data.setId(xg::Guid(json.at(Key_Id).get<std::string>()));
    }
    //Note (Aurélien)
    //Compatibility check
    else if (json.find("InternId") != json.end())
    {
        data.setId(xg::Guid(json.at("InternId").get<std::string>()));
    }
    else
    {
        IOLOG << "ImportGenericData InternId read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_ImportTime) != json.end())
        data.setCreationTime(json.at(Key_ImportTime).get<uint64_t>());
    else
    {
        IOLOG << "ImportGenericData ImportTime read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_ModificationTime) != json.end())
        data.setModificationTime(json.at(Key_ModificationTime).get<uint64_t>());
    else
        data.setModificationTime(data.getCreationTime());

    if (json.find(Key_Phase) != json.end())
        data.setPhase(Utils::from_utf8(json.at(Key_Phase).get<std::string>()));
    else
    {
        IOLOG << "ImportGenericData Phase read error" << LOGENDL;
        //retVal = false;
    }

    if (json.find(Key_ShowHide) != json.end())
        data.setVisible((json.at(Key_ShowHide).get<bool>()));
    else
    {
        IOLOG << "ImportGenericData ShowHide read error" << LOGENDL;
        retVal = false;
    }

    std::unordered_map<hLinkId, s_hyperlink> linksStored;
    if (json.find(Key_Hyperlinks) != json.end())
    {
        int i = 0;
        for (const nlohmann::json& linkElem : json.at(Key_Hyperlinks))
        {
            if (linkElem.find(Key_LinkId) != linkElem.end()
                && linkElem.find(Key_LinkName) != linkElem.end()
                && linkElem.find(Key_LinkURL) != linkElem.end())
            {
                hLinkId tId = xg::Guid(linkElem.at(Key_LinkId).get<std::string>());
                s_hyperlink hlinkStruct;
                hlinkStruct.name = Utils::from_utf8(linkElem.at(Key_LinkName).get<std::string>());
                hlinkStruct.hyperlink = Utils::from_utf8(linkElem.at(Key_LinkURL).get<std::string>());
                IOLOG << "import template : " << hlinkStruct.name << " with " << tId.str() << LOGENDL;
                linksStored.insert(std::pair<hLinkId, s_hyperlink>(tId, hlinkStruct));
            }
        }
    }
    data.setHyperlinks(linksStored);

    nlohmann::json color;
    if (json.find(Key_ColorRGBA) != json.end())
        color = json.at(Key_ColorRGBA);

    if (color.is_null() || color[0].is_null() || color[1].is_null() || color[2].is_null() || color[3].is_null())
    {
        data.setColor(ProjectColor::getColor("BLUE"));
        IOLOG << "ImportGenericData color read error" << LOGENDL;
    }
    else
        data.setColor(Color32(color[0], color[1], color[2], color[3]));

    if (json.find(Key_IconId) != json.end())
    {
        auto idTmp = magic_enum::enum_cast<scs::MarkerIcon>(json.at(Key_IconId).get<std::string>());
        if (idTmp.has_value() && idTmp.value() != scs::MarkerIcon::Max_Enum)
            data.setMarkerIcon(idTmp.value());
    }
    else
    {
        // This is not an important error as the icon is automatically set by most classes.
        IOLOG << "No icon data found, default used." << LOGENDL;
    }

    return retVal;
}

bool ImportClippingData(const nlohmann::json& json, ClippingData& data)
{
    bool retVal(true);

    //Note (Aurélien)
    //retval not used for compatibility (for the moment)
    if (json.find(Key_ClippingMode) != json.end())
    {
        auto mode = magic_enum::enum_cast<ClippingMode>(json.at(Key_ClippingMode).get<std::string>());
        data.setClippingMode(mode.has_value() ? mode.value() : ClippingMode::showExterior);
    }
    else
    {
        IOLOG << "ImportClippingData ClippingMode read error" << LOGENDL;
        data.setClippingMode(ClippingMode::showExterior);
    }

    if (json.find(Key_MinClipDistance) != json.end())
        data.setMinClipDist((json.at(Key_MinClipDistance).get<float>()));
    else
    {
        IOLOG << "ImportClippingData MinClipDistance read error" << LOGENDL;
    }

    if (json.find(Key_MaxClipDistance) != json.end())
        data.setMaxClipDist((json.at(Key_MaxClipDistance).get<float>()));
    else
    {
        IOLOG << "ImportClippingData MaxClipDistance read error" << LOGENDL;
    }

    if (json.find(Key_Active) != json.end())
        data.setClippingActive((json.at(Key_Active).get<bool>()));
    else
    {
        IOLOG << "ImportClippingData Active read error" << LOGENDL;
        data.setClippingActive(false);
    }

    if (json.find(Key_RampActive) != json.end())
        data.setRampActive((json.at(Key_RampActive).get<bool>()));
    else
    {
        IOLOG << "ImportClippingData RampActive read error" << LOGENDL;
        data.setRampActive(false);
    }

    if (json.find(Key_MinRampDistance) != json.end())
        data.setRampMin((json.at(Key_MinRampDistance).get<float>()));
    else
    {
        IOLOG << "ImportClippingData RampMinimum read error" << LOGENDL;
        data.setRampMin(0.f);
    }

    if (json.find(Key_MaxRampDistance) != json.end())
        data.setRampMax((json.at(Key_MaxRampDistance).get<float>()));
    else
    {
        IOLOG << "ImportClippingData RampMaximum read error" << LOGENDL;
        data.setRampMax(1.f);
    }

    if (json.find(Key_RampSteps) != json.end())
        data.setRampSteps((json.at(Key_RampSteps).get<int>()));
    else
    {
        IOLOG << "ImportClippingData RampSteps read error" << LOGENDL;
        data.setRampSteps(8);
    }

    if (json.find(Key_RampClamped) != json.end())
        data.setRampClamped((json.at(Key_RampClamped).get<bool>()));
    else
    {
        IOLOG << "ImportClippingData RampClamped read error" << LOGENDL;
        data.setRampClamped(true);
    }

    return retVal;
}

bool ImportTransformationModule(const nlohmann::json& json, TransformationModule& data)
{
    bool retVal = true;

    {
        nlohmann::json pos;
        if (json.find(Key_Center) != json.end())
            pos = json.at(Key_Center);
        //Note (Aurélien) : compatibilty check
        else if (json.find("Position") != json.end())
            pos = json.at("Position");
        else if (json.find("Pos") != json.end())
            pos = json.at("Pos");
        else
        {
            IOLOG << "Import3DData Center read error" << LOGENDL;
            retVal = false;
        }

        if (!pos.is_null() && !pos[0].is_null() && !pos[1].is_null() && !pos[2].is_null())
            data.setPosition(Pos3D({ pos[0], pos[1], pos[2] }));
    }

    {
        nlohmann::json qrot;
        if (json.find(Key_Quaternion) != json.end())
            qrot = json.at(Key_Quaternion);
        //Note (Aurélien) : compatibilty check
        else if (json.find("Quaternion_xyzw") != json.end())
            qrot = json.at("Quaternion_xyzw");
        else
        {
            IOLOG << "Import3DData Quaternion read error" << LOGENDL;
            retVal = false;
        }

        if (!qrot.is_null() && !qrot[0].is_null() && !qrot[1].is_null() && !qrot[2].is_null() && !qrot[3].is_null())
            data.setRotation(glm::dquat({ qrot[3], qrot[0], qrot[1], qrot[2] }));
    }

    {
        nlohmann::json size;
        if (json.find(Key_Size) != json.end())
            size = json.at(Key_Size);
        else
        {
            IOLOG << "Import3DData Size read error" << LOGENDL;
            retVal = false;
        }


        if (!size.is_null() && !size[0].is_null() && !size[1].is_null() && !size[2].is_null())
            data.setScale(Pos3D({ size[0], size[1], size[2] }));
    }

    return retVal;
}

bool ImportScanData(const nlohmann::json& json, ScanData& data)
{
    bool retVal = true;

    if (json.find(Key_Clippable) != json.end())
        data.setClippable(json.at(Key_Clippable));
    else
        data.setClippable(true);

    if (json.find(Key_Type) != json.end())
    {
        auto elemType = magic_enum::enum_cast<ElementType>(json.at(Key_Type).get<std::string>());
        data.setIsObject(elemType == ElementType::PCO);
    }

    return retVal;
}

bool ImportPointCloudNode(const nlohmann::json& json, PointCloudNode& data)
{
    bool retVal = true;

    if (json.find(Key_Path) != json.end())
        data.setTlsFilePath(Utils::from_utf8(json.at(Key_Path).get<std::string>()), false);
    else
    {
        IOLOG << "Scan Path read error" << LOGENDL;
        retVal = false;
    }
    return retVal;
}

bool ImportDisplayParameters(const nlohmann::json& json, DisplayParameters& data)
{
    bool retVal(true);

    if (json.find(Key_Rendering_Mode) != json.end())
    {
        auto mode = magic_enum::enum_cast<UiRenderMode>(json.at(Key_Rendering_Mode).get<std::string>());
        data.m_mode = (mode.has_value() ? mode.value() : UiRenderMode::Intensity);
    }
    else
    {
        IOLOG << "ViewPoint RenderingMode read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Background_Color) != json.end())
    {
        nlohmann::json color = json.at(Key_Background_Color);
        data.m_backgroundColor = Color32{ color[0], color[1], color[2], 0 };
    }
    else
    {
        IOLOG << "ViewPoint BackgroundColor read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Point_Size) != json.end())
    {
        data.m_pointSize = json.at(Key_Point_Size).get<float>();
    }
    else
    {
        IOLOG << "ViewPoint Blending read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Delta_Filling) != json.end())
    {
        data.m_deltaFilling = json.at(Key_Delta_Filling).get<float>();
    }
    else
    {
        IOLOG << "ViewPoint DeltaFilling read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Gap_Filling_Texel_Threshold) != json.end())
    {
        data.m_gapFillingTexelThreshold = json.at(Key_Gap_Filling_Texel_Threshold).get<int>();
    }

    if (json.find(Key_Alpha_Object) != json.end())
    {
        data.m_alphaObject = json.at(Key_Alpha_Object).get<float>();
    }
    else
    {
    IOLOG << "ViewPoint AlphaObject read error" << LOGENDL;
    retVal = false;
    }

    if (json.find(Key_Contrast) != json.end())
    {
        data.m_contrast = json.at(Key_Contrast).get<float>();
    }
    else
    {
        IOLOG << "ViewPoint Contrast read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Brightness) != json.end())
    {
        data.m_brightness = json.at(Key_Brightness).get<float>();
    }
    else
    {
        IOLOG << "ViewPoint Brightness read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Saturation) != json.end())
    {
        data.m_saturation = json.at(Key_Saturation).get<float>();
    }
    else
    {
        IOLOG << "ViewPoint Saturation read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Luminance) != json.end())
    {
        data.m_luminance = json.at(Key_Luminance).get<float>();
    }
    else
    {
        IOLOG << "ViewPoint Luminance read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Blending) != json.end())
    {
        data.m_hue = json.at(Key_Blending).get<float>();
    }
    else
    {
        IOLOG << "ViewPoint Blending read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Flat_Color) != json.end())
    {
        nlohmann::json color = json.at(Key_Flat_Color);
        data.m_flatColor = glm::vec3(color[0], color[1], color[2]);
    }
    else
    {
        IOLOG << "ViewPoint FlatColor read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_DistRamp) != json.end())
    {
        nlohmann::json distRamp = json.at(Key_DistRamp);
        data.m_distRampMin = distRamp[0];
        data.m_distRampMax = distRamp[1];
    }
    else
    {
        IOLOG << "ViewPoint DistRamp read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_DistRampSteps) != json.end())
    {
        data.m_distRampSteps = json.at(Key_DistRampSteps).get<int>();
    }
    else
    {
        IOLOG << "ViewPoint DistRampSteps read error" << LOGENDL;
    }

    if (json.find(Key_Blend_Mode) != json.end())
    {
        auto mode = magic_enum::enum_cast<BlendMode>(json.at(Key_Blend_Mode).get<std::string>());
        data.m_blendMode = (mode.has_value() ? mode.value() : BlendMode::Opaque);
    }
    else
    {
        IOLOG << "ViewPoint BlendMode read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_NegativeEffect) != json.end())
    {
        data.m_negativeEffect = json.at(Key_NegativeEffect).get<bool>();
    }
    else
    {
        IOLOG << "ViewPoint NegativeEffect read error" << LOGENDL;
        data.m_negativeEffect = false;
    }

    if (json.find(Key_ReduceFlash) != json.end())
    {
        data.m_reduceFlash = json.at(Key_ReduceFlash).get<bool>();
    }
    else
    {
        IOLOG << "ViewPoint ReduceFlash read error" << LOGENDL;

    }

    if (json.find(Key_FlashAdvanced) != json.end())
    {
        data.m_flashAdvanced = json.at(Key_FlashAdvanced).get<bool>();
    }
    else
    {
        data.m_flashAdvanced = false;
    }

    if (json.find(Key_FlashControl) != json.end())
    {
        data.m_flashControl = json.at(Key_FlashControl).get<float>();
    }
    else
    {
        data.m_flashControl = 50.f;
    }

    if (json.find(Key_Transparency) != json.end())
    {
        data.m_transparency = json.at(Key_Transparency).get<float>();
    }
    else
    {
        IOLOG << "ViewPoint Transparency read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Display_Guizmo) != json.end())
    {
        data.m_displayGizmo = json.at(Key_Display_Guizmo).get<bool>();
    }
    else
    {
        IOLOG << "ViewPoint DisplayGuizmo read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Ramp_Scale_Options) != json.end())
    {
        nlohmann::json options = json.at(Key_Ramp_Scale_Options);
        data.m_rampScale = { options[0].get<bool>(), options[2].get<bool>(), options[1].get<int>() };
    }
    else
    {
        IOLOG << "ViewPoint RampScale read error" << LOGENDL;
    }

    {
        //Retro-compatibility
        if (json.find(Key_Display_Parameters) != json.end())
        {
            nlohmann::json options = json.at(Key_Display_Parameters);
            if (!options[0].is_null() && !options[1].is_null() && !options[2].is_null())
                data.m_unitUsage = { options[0], options[1], UnitType::M3, options[2] };
        }
        else
            IOLOG << "ViewPoint DisplayParameters read error" << LOGENDL;

        if (json.find(Key_Distance_Unit) != json.end())
        {
            auto mode = magic_enum::enum_cast<UnitType>(json.at(Key_Distance_Unit).get<std::string>());
            if (mode.has_value())
                data.m_unitUsage.distanceUnit = mode.value();
        }
        else
            IOLOG << "ViewPoint Distance Unit read error" << LOGENDL;

        if (json.find(Key_Diameter_Unit) != json.end())
        {
            auto mode = magic_enum::enum_cast<UnitType>(json.at(Key_Diameter_Unit).get<std::string>());
            if (mode.has_value())
                data.m_unitUsage.diameterUnit = mode.value();
        }
        else
            IOLOG << "ViewPoint Diameter Unit read error" << LOGENDL;

        if (json.find(Key_Volume_Unit) != json.end())
        {
            auto mode = magic_enum::enum_cast<UnitType>(json.at(Key_Volume_Unit).get<std::string>());
            if (mode.has_value())
                data.m_unitUsage.volumeUnit = mode.value();
        }
        else
            IOLOG << "ViewPoint Volume Unit read error" << LOGENDL;

        if (json.find(Key_Displayed_Digits) != json.end())
            data.m_unitUsage.displayedDigits = (json.at(Key_Displayed_Digits).get<int>());
        else
            IOLOG << "ViewPoint Displayed Digits read error" << LOGENDL;
    }

    if (json.find(Key_Measure_Show_Mask) != json.end())
    {
        data.m_measureMask = json.at(Key_Measure_Show_Mask).get<unsigned int>();
    }
    else
    {
        IOLOG << "ViewPoint MeasureShowMask read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Marker_Show_Mask) != json.end())
    {
        data.m_markerMask = json.at(Key_Marker_Show_Mask).get<unsigned int>();
    }
    else
    {
        IOLOG << "ViewPoint MarkerShowMask read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Text_Display_Options) != json.end())
    {
        nlohmann::json options = json.at(Key_Text_Display_Options);
        data.m_textOptions = { options[0].get<unsigned int>(), options[1].get<int>(), options[2].get<float>() };
    }
    else
    {
        IOLOG << "ViewPoint MarkerTextParameters read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Display_All_Marker_Texts) != json.end())
    {
        data.m_displayAllMarkersTexts = json.at(Key_Display_All_Marker_Texts).get<bool>();
    }
    else
    {
        IOLOG << "ViewPoint DisplayAllMarkerTexts read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Display_All_Measures) != json.end())
    {
        data.m_displayAllMeasures = json.at(Key_Display_All_Measures).get<bool>();
    }
    else
    {
        IOLOG << "ViewPoint DisplatAllMeasures read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Marker_Rendering_Parameters) != json.end())
    {
        nlohmann::json options = json.at(Key_Marker_Rendering_Parameters);
        data.m_markerOptions = { options[0], options[1], options[2], options[3], options[4], options[5] };
    }
    else
    {
        IOLOG << "ViewPoint DisplayAllMarkerTexts read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Post_Rendering_Normals) != json.end())
    {
        nlohmann::json options = json.at(Key_Post_Rendering_Normals);
        if (options.size() == 5)
            data.m_postRenderingNormals = { options[0], options[1], options[2], options[3], options[4] };
        else
            data.m_postRenderingNormals = { true, false, true, 0.5f, 1.f };
    }
    else
    {
        IOLOG << "ViewPoint PostRenderingNormals read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Post_Rendering_Ambient_Occlusion) != json.end())
    {
        nlohmann::json options = json.at(Key_Post_Rendering_Ambient_Occlusion);
        if (options.size() == 3)
            data.m_postRenderingAmbientOcclusion = { options[0], options[1], options[2] };
        else
            IOLOG << "ViewPoint PostRenderingAmbientOcclusion malformed" << LOGENDL;
    }

    if (json.find(Key_Edge_Aware_Blur) != json.end())
    {
        nlohmann::json options = json.at(Key_Edge_Aware_Blur);
        if (options.size() == 5)
            data.m_edgeAwareBlur = { options[0], options[1], options[2], options[3], options[4] };
        else
            IOLOG << "ViewPoint EdgeAwareBlur malformed" << LOGENDL;
    }

    if (json.find(Key_Depth_Lining) != json.end())
    {
        nlohmann::json options = json.at(Key_Depth_Lining);
        if (options.size() == 5)
            data.m_depthLining = { options[0], options[1], options[2], options[3], options[4] };
        else
            IOLOG << "ViewPoint DepthLining malformed" << LOGENDL;
    }

    if (json.find(Key_Ortho_Grid_Active) != json.end())
    {
        data.m_orthoGridActive = json.at(Key_Ortho_Grid_Active).get<bool>();
    }
    else
    {
        IOLOG << "ViewPoint Key_Ortho_Grid_Active read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Ortho_Grid_Color) != json.end())
    {
        nlohmann::json options = json.at(Key_Ortho_Grid_Color);
        if (options.size() == 4)
            data.m_orthoGridColor = Color32(options[0], options[1], options[2], options[3]);
        else
            data.m_orthoGridColor = Color32(128, 128, 128);
    }
    else
    {
        IOLOG << "ViewPoint Key_Ortho_Grid_Color read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Ortho_Grid_Step) != json.end())
    {
        data.m_orthoGridStep = json.at(Key_Ortho_Grid_Step).get<float>();
    }
    else
    {
        IOLOG << "ViewPoint Key_Ortho_Grid_Step read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Ortho_Grid_Linewidth) != json.end())
    {
        data.m_orthoGridLineWidth = json.at(Key_Ortho_Grid_Linewidth).get<uint32_t>();
    }
    else
    {
        IOLOG << "ViewPoint Key_Ortho_Grid_Linewidth read error" << LOGENDL;
        retVal = false;
    }

    return retVal;
}

bool ImportProjectionData(const nlohmann::json& json, ProjectionData& data)
{
    bool retVal(true);

    ProjectionMode projMod;
    ProjectionFrustum projFrus;
    if (json.find(Key_Projection_Mode) != json.end())
    {
        auto mode = magic_enum::enum_cast<ProjectionMode>(json.at(Key_Projection_Mode).get<std::string>());
        projMod = (mode.has_value() ? mode.value() : ProjectionMode::Perspective);
    }
    else
    {
        IOLOG << "ViewPoint ProjectionMode read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Projection_Box) != json.end())
    {
        nlohmann::json box = json.at(Key_Projection_Box);
        if (!box.is_null() && !box[0].is_null() && !box[1].is_null() && !box[2].is_null() && !box[3].is_null() && !box[4].is_null() && !box[5].is_null())
            projFrus = { box[0], box[1], box[2], box[3], box[4], box[5] };
    }
    else
    {
        IOLOG << "ViewPoint ProjectionFrustum read error" << LOGENDL;
        retVal = false;
    }

    if (retVal)
        data.setProjectionData(ProjectionData(projMod, projFrus));
    else
        data.setProjectionData(ProjectionData());

    return retVal;
}

bool ImportClusterData(const nlohmann::json& json, ClusterData& data)
{
    bool retVal = true;
    if (json.find(Key_TreeType) != json.end())
    {
        auto type = magic_enum::enum_cast<TreeType>(json.at(Key_TreeType).get<std::string>());
        data.setTreeType(type.has_value() ? type.value() : TreeType::RawData);
    }
    else
    {
        IOLOG << "ClusterData treetype read error" << LOGENDL;
        data.setTreeType(TreeType::RawData);
    }

    return retVal;
}

bool ImportGridData(const nlohmann::json& json, BoxNode& data)
{
    bool retVal(true);
    if (json.find(Key_GridType) != json.end())
    {
        if (magic_enum::enum_cast<GridType>(json.at(Key_GridType).get<std::string>()).has_value() == true)
            data.setGridType(magic_enum::enum_cast<GridType>(json.at(Key_GridType).get<std::string>()).value());
        else
            data.setGridType(GridType::ByMultiple);
    }
    else
    {
        IOLOG << "Box GridType read error" << LOGENDL;
    }

    if (json.find(Key_Division) != json.end())
    {
        nlohmann::json pos = json.at(Key_Division);
        if(!pos.is_null())
            data.setGridDivision({ pos[0], pos[1], pos[2] });
    }
    else
    {
        IOLOG << "Box grid_division read error" << LOGENDL;
    }

    return retVal;
}

bool ImportStandardRadiusData(const nlohmann::json& json, StandardRadiusData& data, const Controller& controller)
{
    bool retVal(true);

    if (json.find(Key_DetectedRadius) != json.end())
        data.setDetectedRadius(json.at(Key_DetectedRadius).get<double>());
    else if (json.find("detectedRadius") != json.end())
        data.setDetectedRadius(json.at("detectedRadius").get<double>());
    else
    {
        IOLOG << "Cylinder DetectedRadius read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_ForcedRadius) != json.end())
        data.setForcedRadius(json.at(Key_ForcedRadius).get<double>());
    else
    {
        IOLOG << "Cylinder Key_ForcedRadius read error" << LOGENDL;
        //retVal = false;
    }

    if (json.find(Key_DiameterSet) != json.end())
    {
        if (magic_enum::enum_cast<StandardRadiusData::DiameterSet>(json.at(Key_DiameterSet).get<std::string>()).has_value() == true)
            data.setDiameterSet(magic_enum::enum_cast<StandardRadiusData::DiameterSet>(json.at(Key_DiameterSet).get<std::string>()).value());
        else
            data.setDiameterSet(StandardRadiusData::DiameterSet::Detected);
    }
    else
    {
        IOLOG << "Cylinder DiameterSet read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_TubeStandardId) != json.end())
    {
        data.setStandard(controller.cgetContext().getStandard(StandardType::Pipe, xg::Guid(json.at(Key_TubeStandardId).get<std::string>())));
    }
    else
    {
        IOLOG << "Cylinder Standard read error" << LOGENDL;
    }

    return retVal;
}


bool ImportInsulationData(const nlohmann::json& json, InsulationData& data, const Controller& controller)
{
    if (json.find(Key_InsulationRadius) != json.end())
        data.setInsulationRadius(json.at(Key_InsulationRadius).get<double>());
    else
        data.setInsulationRadius(0.0);

    return true;
}

bool ImportTorusData(const nlohmann::json& json, TorusData& data)
{
    bool retVal(true);
    if (json.find(Key_MainRadius) != json.end())
        data.setMainRadius(json.at(Key_MainRadius).get<double>());
    else
    {
        IOLOG << "Torus MainRadius read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_MainAngle) != json.end())
        data.setMainAngle(json.at(Key_MainAngle).get<double>());
    else
    {
        IOLOG << "Torus MainAngle read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_TubeRadius) != json.end())
        data.setTubeRadius(json.at(Key_TubeRadius).get<double>());
    else
    {
        IOLOG << "Torus TubeRadius read error" << LOGENDL;
        retVal = false;
    }

    /*
    if (json.find(Key_TorusCenter) != json.end())
    {
        nlohmann::json pos = json.at(Key_TorusCenter);
        data.setTorusCenter(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "Torus TorusCenter read error" << LOGENDL;
        data.setTorusCenter(Pos3D({ 0, 0, 0 }));
    }
    */

    return retVal;
}

bool ImportMeshObjectData(const nlohmann::json& json, MeshObjectData& data, const std::filesystem::path& objectFolder)
{
    bool retVal(true);

    if (json.find(Key_Path) != json.end())
    {
        std::filesystem::path relativePath = Utils::from_utf8(json.at(Key_Path).get<std::string>());
        data.setFilePath(objectFolder / relativePath.filename());
    }
    else
    {
        IOLOG << "MeshObject Path read error" << LOGENDL;
        retVal = false;
    }

    try {
        if (json.find(Key_ObjectName) != json.end())
            data.setObjectName(Utils::from_utf8(json.at(Key_ObjectName).get<std::string>()));
        else
        {
            IOLOG << "MeshObject ObjectName read error" << LOGENDL;
            retVal = false;
        }
    }
    catch (...)
    {
        retVal = false;
    }

    if (json.find(Key_MeshId) != json.end())
        data.setMeshId(xg::Guid(json.at(Key_MeshId).get<std::string>()));
    else
    {
        IOLOG << "MeshObject Key_MeshId read error" << LOGENDL;
        retVal = false;
    }

    return retVal;
}

bool ImportSimpleMeasureData(const nlohmann::json& json, SimpleMeasureData& data)
{
    bool retVal(true);
    if (json.find(Key_OriginPos) != json.end())
    {
        nlohmann::json pos = json.at(Key_OriginPos);
        data.setOriginPos(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "SimpleMeasure OriginPos read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_DestPos) != json.end())
    {
        nlohmann::json pos = json.at(Key_DestPos);
        data.setDestinationPos(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "SimpleMeasure DestPos read error" << LOGENDL;
        retVal = false;
    }

    return retVal;
}

bool ImportPolylineMeasureData(const nlohmann::json& json, PolylineMeasureData& data)
{
    bool retVal(true);
    if (json.find(Key_Points) != json.end())
    {
        nlohmann::json points = json.at(Key_Points);
        std::vector<Measure> measures;
        std::vector<glm::dvec3> gpoints;
        for (const nlohmann::json& point : points)
            gpoints.push_back(glm::dvec3({ point[0], point[1], point[2] }));

        for (uint32_t iterator(0); iterator < (uint32_t)gpoints.size() - 1; iterator++)
            measures.push_back({ gpoints[iterator],gpoints[iterator + 1] });

        data.setMeasures(measures);
    }
    else
    {
        IOLOG << "PolylineMeasure Points read error" << LOGENDL;
        retVal = false;
    }

    return retVal;
}

bool ImportPointToPlaneMeasureData(const nlohmann::json& json, PointToPlaneMeasureData& data)
{
    bool retVal(true);
    if (json.find(Key_PointToPlaneDist) != json.end())
        data.setPointToPlaneD(json.at(Key_PointToPlaneDist).get<float>());
    //Note (Aurélien) : compatibilty check
    else if (json.find("PointToPlaneD") != json.end())
        data.setPointToPlaneD(json.at("PointToPlaneD").get<float>());
    else
    {
        IOLOG << "PointToPlaneMeasure PointToPlaneDist read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Horizontal) != json.end())
        data.setHorizontal(json.at(Key_Horizontal).get<float>());
    //Note (Aurélien) : compatibilty check
    else if (json.find("Horizontale") != json.end())
        data.setHorizontal(json.at("Horizontale").get<float>());
    else
    {
        IOLOG << "PointToPlaneMeasure Horizontale read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Vertical) != json.end())
        data.setVertical(json.at(Key_Vertical).get<float>());
    else
    {
        IOLOG << "PointToPlaneMeasure Vertical read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_PointCoord) != json.end())
    {
        nlohmann::json pos = json.at(Key_PointCoord);
        data.setPointCoord(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "PointToPlaneMeasure PointCoord read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_PointOnPlane) != json.end())
    {
        nlohmann::json pos = json.at(Key_PointOnPlane);
        data.setpointOnPlane(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "PointToPlanesMeasure PointOnPlane read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_NormalToPlane) != json.end())
    {
        nlohmann::json pos = json.at(Key_NormalToPlane);
        data.setNormalToPlane(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "PointToPlaneMeasure NormalToPlane read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_ProjPoint) != json.end())
    {
        nlohmann::json pos = json.at(Key_ProjPoint);
        data.setProjPoint(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "PointToPlaneMeasure ProjPoint read error" << LOGENDL;
        retVal = false;
    }

    return retVal;
}

bool ImportPointToPipeMeasureData(const nlohmann::json& json, PointToPipeMeasureData& data)
{
    bool retVal(true);
    if (json.find(Key_PointToAxeDist) != json.end())
        data.setPointToAxeDist(json.at(Key_PointToAxeDist).get<float>());
    else
    {
        IOLOG << "PointToPipeMeasure PointToAxeDist read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_PointToAxeHorizontal) != json.end())
        data.setPointToAxeHorizontal(json.at(Key_PointToAxeHorizontal).get<float>());
    else
    {
        IOLOG << "PointToPipeMeasure PointToAxeHorizontal read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_PointToAxeVertical) != json.end())
        data.setPointToAxeVertical(json.at(Key_PointToAxeVertical).get<float>());
    else
    {
        IOLOG << "PointToPipeMeasure PointToAxeVertical read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_FreeDist) != json.end())
        data.setFreeD(json.at(Key_FreeDist).get<float>());

    //Note (Aurélien) : compatibilty check
    else if (json.find("FreeD") != json.end())
        data.setFreeDistHorizontal(json.at("FreeD").get<float>());
    else
    {
        IOLOG << "PointToPipeMeasure FreeDist read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_FreeDistHorizontal) != json.end())
        data.setFreeDistHorizontal(json.at(Key_FreeDistHorizontal).get<float>());
    //Note (Aurélien) : compatibilty check
    else if (json.find("FreeDHorizontal") != json.end())
        data.setFreeDistHorizontal(json.at("FreeDHorizontal").get<float>());
    else
    {
        IOLOG << "PointToPipeMeasure FreeDistHorizontal read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_FreeDistVertical) != json.end())
        data.setFreeDistVertical(json.at(Key_FreeDistVertical).get<float>());
    //Note (Aurélien) : compatibilty check
    else if (json.find("FreeDVertical") != json.end())
        data.setFreeDistVertical(json.at("FreeDVertical").get<float>());
    else
    {
        IOLOG << "PointToPipeMeasure FreeDistVertical read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_TotalFootprint) != json.end())
        data.setTotalFootprint(json.at(Key_TotalFootprint).get<float>());
    //Note (Aurélien) : compatibilty check
    else if (json.find("TotalF") != json.end())
        data.setTotalFootprint(json.at("TotalF").get<float>());
    else
    {
        IOLOG << "PointToPipeMeasure FreeDistTotal read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_PipeDiameter) != json.end())
        data.setPipeDiameter(json.at(Key_PipeDiameter).get<float>());
    else
    {
        IOLOG << "PointToPipeMeasure PipeDiameter read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_PipeCenter) != json.end())
    {
        nlohmann::json pos = json.at(Key_PipeCenter);
        data.setPipeCenter(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "PointToPipeMeasure PipeCenter read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_PointCoord) != json.end())
    {
        nlohmann::json pos = json.at(Key_PointCoord);
        data.setPointCoord(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "PointToPipeMeasure PointCoord read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_ProjPoint) != json.end())
    {
        nlohmann::json pos = json.at(Key_ProjPoint);
        data.setProjPoint(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "PointToPipeMeasure measure ProjPoint read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_PipeCenterToProj) != json.end())
        data.setPipeCenterToProj(json.at(Key_PipeCenterToProj).get<float>());
    else
    {
        IOLOG << "PointToPipeMeasure PipeCenterToProj read error" << LOGENDL;
        retVal = false;
    }

    return retVal;
}

bool ImportPipeToPlaneMeasureData(const nlohmann::json& json, PipeToPlaneMeasureData& data)
{
    bool retVal(true);
    if (json.find(Key_CenterToPlaneDist) != json.end())
        data.setCenterToPlaneDist(json.at(Key_CenterToPlaneDist).get<float>());
    //Note (Aurélien) : compatibilty check
    else if (json.find("CenterToPlaneD") != json.end())
        data.setCenterToPlaneDist(json.at("CenterToPlaneD").get<float>());
    else
    {
        IOLOG << "PipeToPlaneMeasure CenterToPlaneDist read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_PlaneCenterHorizontal) != json.end())
        data.setPlaneCenterHorizontal(json.at(Key_PlaneCenterHorizontal).get<float>());
    else
    {
        IOLOG << "PipeToPlaneMeasure PlaneCenterHorizontal read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_PlaneCenterVertical) != json.end())
        data.setPlaneCenterVertical(json.at(Key_PlaneCenterVertical).get<float>());
    else
    {
        IOLOG << "PipeToPlaneMeasure PlaneCenterVertical read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_FreeDist) != json.end())
        data.setFreeDist(json.at(Key_FreeDist).get<float>());
    else
    {
        IOLOG << "PipeToPlaneMeasure FreeDist read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_FreeDistHorizontal) != json.end())
        data.setFreeDistHorizontal(json.at(Key_FreeDistHorizontal).get<float>());
    else
    {
        IOLOG << "PipeToPlaneMeasure FreeDistHorizontal read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_FreeDistVertical) != json.end())
        data.setFreeDistVertical(json.at(Key_FreeDistVertical).get<float>());
    else
    {
        IOLOG << "PipeToPlaneMeasure FreeDistVertical read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_TotalFootprint) != json.end())
        data.setTotalFootprint(json.at(Key_TotalFootprint).get<float>());
    //Note (Aurélien) : compatibilty check
    else if (json.find("TotalF") != json.end())
        data.setTotalFootprint(json.at("TotalF").get<float>());
    else
    {
        IOLOG << "PipeToPlaneMeasure FreeDistTotal read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_PipeDiameter) != json.end())
        data.setPipeDiameter(json.at(Key_PipeDiameter).get<float>());
    else
    {
        IOLOG << "PipeToPlaneMeasure PipeDiameter read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_PipeCenter) != json.end())
    {
        nlohmann::json pos = json.at(Key_PipeCenter);
        data.setPipeCenter(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "PipeToPlaneMeasure PipeCenter read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_PointOnPlane) != json.end())
    {
        nlohmann::json pos = json.at(Key_PointOnPlane);
        data.setPointOnPlane(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "PipeToPlaneMeasure PointOnPlane read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_NormalOnPlane) != json.end())
    {
        nlohmann::json pos = json.at(Key_NormalOnPlane);
        data.setNormalOnPlane(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "PipeToPlaneMeasure NormalOnPlane read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_ProjPoint) != json.end())
    {
        nlohmann::json pos = json.at(Key_ProjPoint);
        data.setProjPoint(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "PipeToPlaneMeasure ProjPoint read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_PointOnPlaneToProj) != json.end())
        data.setPointOnPlaneToProj(json.at(Key_PointOnPlaneToProj).get<float>());
    else
    {
        IOLOG << "PipeToPlaneMeasure PointOnPlaneToProj read error" << LOGENDL;
        retVal = false;
    }

    return retVal;
}

bool ImportPipeToPipeMeasureData(const nlohmann::json& json, PipeToPipeMeasureData& data)
{
    bool retVal(true);

    if (json.find(Key_CenterP1ToAxeP2) != json.end())
        data.setCenterP1ToAxeP2(json.at(Key_CenterP1ToAxeP2).get<float>());
    else
    {
        IOLOG << "PipeToPipeMeasure measure CenterP1ToAxeP2 read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_P1ToP2Horizontal) != json.end())
        data.setP1ToP2Horizontal(json.at(Key_P1ToP2Horizontal).get<float>());
    else
    {
        IOLOG << "PipeToPipeMeasure P1ToP2Horizontal read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_P1ToP2Vertical) != json.end())
        data.setP1ToP2Vertical(json.at("P1ToP2Vertical").get<float>());
    else
    {
        IOLOG << "PipeToPipeMeasure P1ToP2Vertical read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_FreeDist) != json.end())
        data.setFreeDist(json.at(Key_FreeDist).get<float>());
    //Note (Aurélien) : compatibilty check
    else if (json.find("FreeD") != json.end())
        data.setFreeDist(json.at("FreeD").get<float>());
    else
    {
        IOLOG << "PipeToPipeMeasure FreeDist read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_FreeDistHorizontal) != json.end())
        data.setFreeDistHorizontal(json.at(Key_FreeDistHorizontal).get<float>());
    //Note (Aurélien) : compatibilty check
    else if (json.find("FreeDHorizontal") != json.end())
        data.setFreeDistHorizontal(json.at("FreeDHorizontal").get<float>());
    else
    {
        IOLOG << "PipeToPipeMeasure FreeDistHorizontal read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_FreeDistVertical) != json.end())
        data.setFreeDistVertical(json.at(Key_FreeDistVertical).get<float>());
    //Note (Aurélien) : compatibilty check
    else if (json.find("FreeDVertical") != json.end())
        data.setFreeDistVertical(json.at("FreeDVertical").get<float>());
    else
    {
        IOLOG << "PipeToPipeMeasure FreeDistVertical read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_TotalFootprint) != json.end())
        data.setTotalFootprint(json.at(Key_TotalFootprint).get<float>());
    //Note (Aurélien) : compatibilty check
    else if (json.find("TotalF") != json.end())
        data.setTotalFootprint(json.at("TotalF").get<float>());
    else
    {
        IOLOG << "PipeToPipeMeasure TotalFootprint read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Pipe1Diameter) != json.end())
        data.setPipe1Diameter(json.at(Key_Pipe1Diameter).get<float>());
    else
    {
        IOLOG << "PipeToPipeMeasure Pipe1Diameter read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Pipe2Diameter) != json.end())
        data.setPipe2Diameter(json.at(Key_Pipe2Diameter).get<float>());
    else
    {
        IOLOG << "PipeToPipeMeasure Pipe2Diameter read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Pipe1Center) != json.end())
    {
        nlohmann::json pos = json.at(Key_Pipe1Center);
        data.setPipe1Center(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "PipeToPipeMeasure Pipe1Center read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Pipe2Center) != json.end())
    {
        nlohmann::json pos = json.at(Key_Pipe2Center);
        data.setPipe2Center(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "PipeToPipeMeasure Pipe2Center read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_ProjPoint) != json.end())
    {
        nlohmann::json pos = json.at(Key_ProjPoint);
        data.setProjPoint(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "PipeToPipeMeasure ProjPoint read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Pipe2CenterToProj) != json.end())
        data.setPipe2CenterToProj(json.at(Key_Pipe2CenterToProj).get<float>());
    else
    {
        IOLOG << "PipeToPipeMeasure PointOnPlaneToProj read error" << LOGENDL;
        retVal = false;
    }

    return retVal;
}

bool ImportBeamBendingMeasureData(const nlohmann::json& json, BeamBendingMeasureData& data)
{
    bool retVal(true);

    if (json.find(Key_Point1) != json.end())
    {
        nlohmann::json pos = json.at(Key_Point1);
        data.setPoint1Pos(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    //Note (Aurélien) : compatibilty check
    else if (json.find("Point1Pos") != json.end())
    {
        nlohmann::json pos = json.at("Point1Pos");
        data.setPoint1Pos(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "BeamBendingMeasure Point1 read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Point2) != json.end())
    {
        nlohmann::json pos = json.at(Key_Point2);
        data.setPoint2Pos(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    //Note (Aurélien) : compatibilty check
    else if (json.find("Point2Pos") != json.end())
    {
        nlohmann::json pos = json.at("Point2Pos");
        data.setPoint1Pos(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "BeamBendingMeasure Point2 read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_MaxBendingPos) != json.end())
    {
        nlohmann::json pos = json.at(Key_MaxBendingPos);
        data.setMaxBendingPos(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "BeamBendingMeasure MaxBendingPos read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_BendingValue) != json.end())
        data.setBendingValue(json.at(Key_BendingValue).get<float>());
    else
    {
        IOLOG << "BeamBendingMeasure BendingValue read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Length) != json.end())
        data.setLength(json.at(Key_Length).get<float>());
    //Note (Aurélien) : compatibilty check
    else if (json.find("Lenght") != json.end())
        data.setLength(json.at("Lenght").get<float>());
    else
    {
        IOLOG << "BeamBendingMeasure Length read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Ratio) != json.end())
        data.setRatio(json.at(Key_Ratio).get<float>());
    else
    {
        IOLOG << "BeamBendingMeasure Ratio read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_MaxRatio) != json.end())
        data.setMaxRatio(json.at(Key_MaxRatio).get<float>());
    else
    {
        IOLOG << "BeamBendingMeasure MaxRatio read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_RatioSup) != json.end())
        data.setRatioSup(magic_enum::enum_cast<RatioSup>(json.at(Key_RatioSup).get<std::string>()).value());
    //Note (Aurélien) : compatibilty check
    else if (json.find("isRatioSup") != json.end())
        data.setRatioSup(magic_enum::enum_cast<RatioSup>(json.at("isRatioSup").get<std::string>()).value());
    else
    {
        IOLOG << "BeamBendingMeasure RatioSup read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Result) != json.end())
        data.setResultReliability(magic_enum::enum_cast<Reliability>(json.at(Key_Result).get<std::string>()).value());
    else
    {
        IOLOG << "BeamBendingMeasure Result read error" << LOGENDL;
        retVal = false;
    }

    return retVal;
}

bool ImportColumnTiltMeasureData(const nlohmann::json& json, ColumnTiltMeasureData& data)
{
    bool retVal(true);
    if (json.find(Key_Point1) != json.end())
    {
        nlohmann::json pos = json.at(Key_Point1);
        data.setPoint1(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "ColumnTiltMeasure Point1 read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Point2) != json.end())
    {
        nlohmann::json pos = json.at(Key_Point2);
        data.setPoint2(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "ColumnTiltMeasure Point2 read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_BottomPoint) != json.end())
    {
        nlohmann::json pos = json.at(Key_BottomPoint);
        data.setBottomPoint(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "ColumnTiltMeasure BottomPoint read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_TopPoint) != json.end())
    {
        nlohmann::json pos = json.at(Key_TopPoint);
        data.setTopPoint(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "ColumnTiltMeasure TopPoint read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_TiltValue) != json.end())
        data.setTiltValue(json.at(Key_TiltValue).get<float>());
    else
    {
        IOLOG << "ColumnTiltMeasure TiltValue read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Height) != json.end())
        data.setHeight(json.at(Key_Height).get<float>());
    else
    {
        IOLOG << "ColumnTiltMeasure Height read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Ratio) != json.end())
        data.setRatio(json.at(Key_Ratio).get<float>());
    else
    {
        IOLOG << "ColumnTiltMeasure Ratio read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_MaxRatio) != json.end())
        data.setMaxRatio(json.at(Key_MaxRatio).get<float>());
    else
    {
        IOLOG << "ColumnTiltMeasure MaxRatio read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_RatioSup) != json.end())
        data.setRatioSup(magic_enum::enum_cast<RatioSup>(json.at(Key_RatioSup).get<std::string>()).value());
    else
    {
        IOLOG << "ColumnTiltMeasure RatioSup read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Result) != json.end())
        data.setResultReliability(magic_enum::enum_cast<Reliability>(json.at(Key_Result).get<std::string>()).value());
    else
    {
        IOLOG << "ColumnTiltMeasure measure Result read error" << LOGENDL;
        retVal = false;
    }

    return retVal;
}

bool ImportViewPointData(const nlohmann::json& json, ViewPointData& data, const std::unordered_map<xg::Guid, SafePtr<AGraphNode>>& nodeById)
{
    bool retVal(true);
    if (json.find(Key_Edge_Aware_Blur) != json.end())
    {
        nlohmann::json options = json.at(Key_Edge_Aware_Blur);
        if (options.size() == 5)
            data.m_edgeAwareBlur = { options[0], options[1], options[2], options[3], options[4] };
        else
            IOLOG << "ViewPoint EdgeAwareBlur malformed" << LOGENDL;
    }

    if (json.find(Key_Depth_Lining) != json.end())
    {
        nlohmann::json options = json.at(Key_Depth_Lining);
        if (options.size() == 5)
            data.m_depthLining = { options[0], options[1], options[2], options[3], options[4] };
        else
            IOLOG << "ViewPoint DepthLining malformed" << LOGENDL;
    }

    if (json.find(Key_Active_Clippings) != json.end())
    {
        std::unordered_set<SafePtr<AClippingNode>> list;
        for (const nlohmann::json& child : json.at(Key_Active_Clippings))
        {
            xg::Guid guid = xg::Guid(child.get<std::string>());
            if (nodeById.find(guid) == nodeById.end())
            {
                IOLOG << "ViewPoint ActiveClippings couldnt find object" << LOGENDL;
                continue;
            }
            SafePtr<AGraphNode> childNode = nodeById.at(guid);
            if (childNode)
                list.insert(static_pointer_cast<AClippingNode>(childNode));
        }
        data.setActiveClippings(list);
    }
    else
    {
        IOLOG << "ViewPoint ActiveClippings read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Interior_Clippings) != json.end())
    {
        std::unordered_set<SafePtr<AClippingNode>> list;
        for (const nlohmann::json& child : json.at(Key_Interior_Clippings))
        {
            xg::Guid guid = xg::Guid(child.get<std::string>());
            if (nodeById.find(guid) == nodeById.end())
            {
                IOLOG << "ViewPoint InteriorClippings couldnt find object" << LOGENDL;
                continue;
            }
            SafePtr<AGraphNode> childNode = nodeById.at(guid);
            if (childNode)
                list.insert(static_pointer_cast<AClippingNode>(childNode));
        }
        data.setInteriorClippings(list);
    }
    else
    {
        IOLOG << "ViewPoint InteriorClippings read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Active_Ramps) != json.end())
    {
        std::unordered_set<SafePtr<AClippingNode>> list;
        for (const nlohmann::json& child : json.at(Key_Active_Ramps))
        {
            xg::Guid guid = xg::Guid(child.get<std::string>());
            if (nodeById.find(guid) == nodeById.end())
            {
                IOLOG << "ViewPoint ActiveRamps couldnt find object" << LOGENDL;
                continue;
            }
            SafePtr<AGraphNode> childNode = nodeById.at(guid);
            if (childNode)
                list.insert(static_pointer_cast<AClippingNode>(childNode));
        }
        data.setActiveRamps(list);
    }
    else
    {
        IOLOG << "ViewPoint ActiveRamps read missing" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Visible_Objects) != json.end())
    {
        std::unordered_set<SafePtr<AGraphNode>> list;
        for (const nlohmann::json& child : json.at(Key_Visible_Objects))
        {
            xg::Guid guid = xg::Guid(child.get<std::string>());
            if (nodeById.find(guid) == nodeById.end())
            {
                IOLOG << "ViewPoint VisibleObjects couldnt find object" << LOGENDL;
                continue;
            }
            SafePtr<AGraphNode> childNode = nodeById.at(xg::Guid(guid));
            if (childNode)
                list.insert(childNode);
        }
        data.setVisibleObjects(list);
    }
    else
    {
        IOLOG << "ViewPoint VisibleObjects read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Objects_Colors) != json.end())
    {
        std::unordered_map<SafePtr<AGraphNode>, Color32> map;
        for (const nlohmann::json& child : json.at(Key_Objects_Colors))
        {
            Color32 color = Color32(child[1].get<uint8_t>(), child[2].get<uint8_t>(), child[3].get<uint8_t>(), child[4].get<uint8_t>());
            xg::Guid guid = xg::Guid(child[0].get<std::string>());
            if (nodeById.find(guid) == nodeById.end())
            {
                IOLOG << "ViewPoint ObjectsColors couldnt find object" << LOGENDL;
                continue;
            }
            SafePtr<AGraphNode> childNode = nodeById.at(guid);
            if (childNode)
                map[childNode] = color;
        }
        data.setScanClusterColors(map);
    }
    else
    {
        IOLOG << "ViewPoint ObjectsColors read error" << LOGENDL;
    }

    //if (json.find(Key_Active_Scans) != json.end())
    //{
    //	std::unordered_set<xg::Guid> list;
    //	for (const nlohmann::json& child : json.at(Key_Active_Scans))
    //		list.insert(xg::Guid(child.get<std::string>()));
    //	data.setActiveScans(list);
    //}
    //else
    //{
    //	IOLOG << "ViewPoint ActiveScans read error" << LOGENDL;
    //	retVal = false;
    //}

    return retVal;
}

bool ImportTagData(const nlohmann::json& json, TagData& data, const std::unordered_set<SafePtr<sma::TagTemplate>>& templates)
{
    bool retVal = true;
    sma::templateId templateId;

    if (json.find(Key_TemplateId) != json.end())
        templateId = (xg::Guid(json.at(Key_TemplateId).get<std::string>()));
    else
    {
        IOLOG << "Tag TemplateId read error" << LOGENDL;
        retVal = false;
    }

    SafePtr<sma::TagTemplate> currentTemp;
    for (SafePtr<sma::TagTemplate> tagTemp : templates)
    {
        ReadPtr<sma::TagTemplate> rTagTemp = tagTemp.cget();
        if (rTagTemp && rTagTemp->getId() == templateId)
        {
            currentTemp = tagTemp;
            data.setTemplate(tagTemp);
            break;
        }
    }

    ReadPtr<sma::TagTemplate> rCurrentTemp = currentTemp.cget();
    if (rCurrentTemp)
    {
        std::vector<sma::tField> fields = rCurrentTemp->getFieldsCopy();

        for (auto itF = fields.begin(); itF != fields.end(); itF++)
        {
            if (json.find(itF->m_id.str()) != json.end())
                data.setValue(itF->m_id, Utils::from_utf8(json.at(itF->m_id.str()).get<std::string>()));
            else
                IOLOG << "Can't find field " << itF->m_name << " id : " << itF->m_id.str() << LOGENDL;
        }
    }
    else
    {
        IOLOG << "Can't find template for the tag " << LOGENDL;
        retVal = false;
    }

    return retVal;
}

bool ImportNodeLinks(const nlohmann::json& json, const SafePtr<AGraphNode>& nodePtr, const std::unordered_map<xg::Guid, SafePtr<AGraphNode>>& nodeById)
{
    bool retVal = true;
    if (json.find(Key_GeometricChildrens) != json.end())
    {
        for (const nlohmann::json& child : json.at(Key_GeometricChildrens))
        {
            xg::Guid guid = xg::Guid(child.get<std::string>());
            if (nodeById.find(guid) == nodeById.end())
            {
                IOLOG << "AGraphNode couldnt find geometric child" << LOGENDL;
                continue;
            }
            SafePtr<AGraphNode> childNode = nodeById.at(guid);
            AGraphNode::addGeometricLink(nodePtr, childNode);
        }
    }
    else
    {
        retVal = false;
        IOLOG << "AGraphNode couldnt read geometric children" << LOGENDL;
    }

    // NOTE - All nodes when added to the GraphManager are automatically connected to the root.
    //      - In short, the geometric link is not exploited correctly.
    if (json.find(Key_GeometricParent) != json.end())
    {
        xg::Guid guid = xg::Guid(json.at(Key_GeometricParent).get<std::string>());
        if (nodeById.find(guid) != nodeById.end())
        {
            SafePtr<AGraphNode> parentNode = nodeById.at(guid);
            AGraphNode::addGeometricLink(parentNode, nodePtr);
        }
    }
    else
    {
        retVal = false;
        IOLOG << "AGraphNode couldnt read geometric parent" << LOGENDL;
    }

    if (json.find(Key_OwningChildrens) != json.end())
    {
        for (const nlohmann::json& child : json.at(Key_OwningChildrens))
        {
            xg::Guid guid = xg::Guid(child.get<std::string>());
            if (nodeById.find(guid) == nodeById.end())
            {
                IOLOG << "AGraphNode couldnt find owning child" << LOGENDL;
                continue;
            }
            SafePtr<AGraphNode> childNode = nodeById.at(guid);
            AGraphNode::addOwningLink(nodePtr, childNode);
        }
    }
    //Retro-compatibility old piping object
    else if (json.find(Key_PipingElems) != json.end())
    {
        for (const nlohmann::json& elemIt : json.at(Key_PipingElems))
        {
            if (elemIt.find(Key_InternId) != elemIt.end())
            {
                xg::Guid guid = xg::Guid(elemIt.at(Key_InternId).get<std::string>());
                if (nodeById.find(guid) != nodeById.end())
                {
                    SafePtr<AGraphNode> childNode = nodeById.at(guid);
                    AGraphNode::addOwningLink(nodePtr, childNode);
                }
                else
                    IOLOG << "AGraphNode couldnt find owning child" << LOGENDL;
            }
            else
            {
                IOLOG << "Piping InternId read error" << LOGENDL;
                continue;
            }
        }
    }
    else
    {
        retVal = false;
        IOLOG << "AGraphNode couldnt read owning children" << LOGENDL;
    }

    if (json.find(Key_OwningParents) != json.end())
    {
        for (const nlohmann::json& parent : json.at(Key_OwningParents))
        {
            xg::Guid guid = xg::Guid(parent.get<std::string>());
            if (nodeById.find(guid) == nodeById.end())
            {
                IOLOG << "AGraphNode couldnt find owning parent" << LOGENDL;
                continue;
            }
            SafePtr<AGraphNode> parentNode = nodeById.at(guid);
            AGraphNode::addOwningLink(parentNode, nodePtr);
        }
    }
    if (json.find(Key_OwningHierarchyParent) != json.end())
    {
        xg::Guid guid = xg::Guid(json.at(Key_OwningHierarchyParent).get<std::string>());
        if (nodeById.find(guid) != nodeById.end())
        {
            SafePtr<AGraphNode> parentNode = nodeById.at(guid);
            AGraphNode::addOwningLink(parentNode, nodePtr);
        }
        else
            IOLOG << "AGraphNode couldnt find owning parent" << LOGENDL;
    }
    if (json.find(Key_OwningObjectParent) != json.end())
    {
        xg::Guid guid = xg::Guid(json.at(Key_OwningObjectParent).get<std::string>());
        if (nodeById.find(guid) != nodeById.end())
        {
            SafePtr<AGraphNode> parentNode = nodeById.at(guid);
            AGraphNode::addOwningLink(parentNode, nodePtr);
        }
        else
            IOLOG << "AGraphNode couldnt find owning parent" << LOGENDL;
    }

    return retVal;
}

void DataDeserializer::PostDeserializeNode(const nlohmann::json& json, const SafePtr<AGraphNode> node, const std::unordered_map<xg::Guid, SafePtr<AGraphNode>>& nodeById)
{
    ElementType type;
    {
        ReadPtr<AGraphNode> rNode = node.cget();
        if (rNode)
            type = rNode->getType();
    }

    ImportNodeLinks(json, node, nodeById);

    switch (type)
    {
        case ElementType::ViewPoint:
        {
            WritePtr<ViewPointNode> wVp = static_pointer_cast<ViewPointNode>(node).get();
            if (wVp)
                ImportViewPointData(json, *&wVp, nodeById);
        }
        break;
    }
}

bool DataDeserializer::DeserializeTagNode(SafePtr<TagNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<TagNode> wNode = node.get();
    if (!wNode)
        return false;

    const std::unordered_set<SafePtr<sma::TagTemplate>>& templates = controller.getContext().getTemplates();
            
    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportClippingData(json, *&wNode);
    success &= ImportTagData(json, *&wNode, templates);

    return success;
}

bool DataDeserializer::DeserializeMeshObjectNode(SafePtr<MeshObjectNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<MeshObjectNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportMeshObjectData(json, *&wNode, controller.getContext().cgetProjectInternalInfo().getObjectsFilesFolderPath());

    return success;
}

bool DataDeserializer::DeserializeSimpleMeasureNode(SafePtr<SimpleMeasureNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<SimpleMeasureNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportClippingData(json, *&wNode);
    success &= ImportSimpleMeasureData(json, *&wNode);

    return success;
}

bool DataDeserializer::DeserializePolylineMeasureNode(SafePtr<PolylineMeasureNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<PolylineMeasureNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportClippingData(json, *&wNode);
    success &= ImportPolylineMeasureData(json, *&wNode);

    return success;
}

bool DataDeserializer::DeserializePointToPlaneMeasureNode(SafePtr<PointToPlaneMeasureNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<PointToPlaneMeasureNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportClippingData(json, *&wNode);
    success &= ImportPointToPlaneMeasureData(json, *&wNode);

    return success;
}

bool DataDeserializer::DeserializePointToPipeMeasureNode(SafePtr<PointToPipeMeasureNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<PointToPipeMeasureNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportClippingData(json, *&wNode);
    success &= ImportPointToPipeMeasureData(json, *&wNode);

    return success;
}

bool DataDeserializer::DeserializePipeToPlaneMeasureNode(SafePtr<PipeToPlaneMeasureNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<PipeToPlaneMeasureNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportClippingData(json, *&wNode);
    success &= ImportPipeToPlaneMeasureData(json, *&wNode);

    return success;
}

bool DataDeserializer::DeserializePipeToPipeMeasureNode(SafePtr<PipeToPipeMeasureNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<PipeToPipeMeasureNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportClippingData(json, *&wNode);
    success &= ImportPipeToPipeMeasureData(json, *&wNode);

    return success;
}

bool DataDeserializer::DeserializeBeamBendingMeasureNode(SafePtr<BeamBendingMeasureNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<BeamBendingMeasureNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportBeamBendingMeasureData(json, *&wNode);

    return success;
}

bool DataDeserializer::DeserializeColumnTiltMeasureNode(SafePtr<ColumnTiltMeasureNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<ColumnTiltMeasureNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportColumnTiltMeasureData(json, *&wNode);

    return success;
}

bool DataDeserializer::DeserializePointCloudNode(SafePtr<PointCloudNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<PointCloudNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportScanData(json, *&wNode);
    success &= ImportPointCloudNode(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);

    return success;
}

bool DataDeserializer::DeserializeViewPointNode(SafePtr<ViewPointNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<ViewPointNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportDisplayParameters(json, *&wNode);
    success &= ImportProjectionData(json, *&wNode);

    return success;
}

bool DataDeserializer::DeserializeClusterNode(SafePtr<ClusterNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<ClusterNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportClusterData(json, *&wNode);

    return success;
}

bool DataDeserializer::DeserializeBoxNode(SafePtr<BoxNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<BoxNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;
    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportClippingData(json, *&wNode);
    success &= ImportGridData(json, *&wNode);
    return success;
}

bool DataDeserializer::DeserializeCameraNode(SafePtr<CameraNode> node, const nlohmann::json& json)
{
    WritePtr<CameraNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportData(json, *&wNode);
    success &= ImportDisplayParameters(json, *&wNode);
    success &= ImportProjectionData(json, *&wNode);

    return success;
}

bool DataDeserializer::DeserializePointNode(SafePtr<PointNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<PointNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportClippingData(json, *&wNode);

    return success;
}

bool DataDeserializer::DeserializeCylinderNode(SafePtr<CylinderNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<CylinderNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportClippingData(json, *&wNode);
    success &= ImportStandardRadiusData(json, *&wNode, controller);
    success &= ImportInsulationData(json, *&wNode, controller);

    if (json.find(Key_Length) != json.end())
        wNode->setLength((json.at(Key_Length).get<double>()));
    else
        wNode->setLength(wNode->getScale().z * 2.0);

    return success;
}

bool DataDeserializer::DeserializeTorusNode(SafePtr<TorusNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<TorusNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportClippingData(json, *&wNode);
    success &= ImportTorusData(json, *&wNode);

    wNode->updateTorusMesh();

    return success;
}

bool DataDeserializer::DeserializeSphereNode(SafePtr<SphereNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<SphereNode> wNode = node.get();
    if (!wNode)
        return false;

    bool success = true;

    success &= ImportData(json, *&wNode);
    success &= ImportTransformationModule(json, *&wNode);
    success &= ImportClippingData(json, *&wNode);

    if (json.find(Key_ForcedRadius) != json.end())
        wNode->setRadius((json.at(Key_ForcedRadius).get<double>()));
    else
        success &= false;

    return success;
}

bool DataDeserializer::DeserializePiping(SafePtr<ClusterNode> node, const nlohmann::json& json, Controller& controller)
{
    WritePtr<ClusterNode> wNode = node.get();
    if (!wNode)
        return false;

    wNode->setTreeType(TreeType::Piping);
    return ImportData(json, *&wNode);
}

bool DataDeserializer::DeserializeStandards(const nlohmann::json& json, std::unordered_map<StandardType, std::vector<StandardList>>& data)
{
    if (json.find(Key_Standards) == json.end())
    {
        IOLOG << "Key_Standards read error" << LOGENDL;
        return false;
    }

    data.clear();
    for (const nlohmann::json& stdsJson : json.at(Key_Standards))
    {
        if (stdsJson.find(Key_StandardType) == stdsJson.end())
            continue;
        auto mode = magic_enum::enum_cast<StandardType>(stdsJson.at(Key_StandardType).get<std::string>());
        if (!mode.has_value())
            continue;
        StandardType type = mode.value();

        if (stdsJson.find(Key_StandardLists) == stdsJson.end())
            continue;
        std::vector<List<double>> stds;
        for (const nlohmann::json& stdJson : stdsJson.at(Key_StandardLists))
        {
            StandardList std;
            if (!DataDeserializer::DeserializeList<StandardList>(stdJson, std))
                continue;
            stds.push_back(std);
        }

        if (!stds.empty())
            data[type] = stds;
    } 

    return true;
}

bool DataDeserializer::DeserializeUserOrientation(const nlohmann::json& json, UserOrientation& data)
{
    bool retVal(true);
    if (json.find(Key_Id) != json.end())
        data.setId(xg::Guid(json.at(Key_Id).get<std::string>()));
    //Note (Aurélien) : compatibilty check
    else if (json.find("id") != json.end())
        data.setId(xg::Guid(json.at("id").get<std::string>()));
    else
    {
        IOLOG << "UserOrientation Id read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Name) != json.end())
        data.setName(QString::fromStdWString(Utils::from_utf8(json.at(Key_Name).get<std::string>())));
    //Note (Aurélien) : compatibilty check
    else if (json.find("name") != json.end())
        data.setName(QString::fromStdWString(Utils::from_utf8(json.at("name").get<std::string>())));
    else
    {
        IOLOG << "UserOrientation Name read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Point1) != json.end())
    {
        nlohmann::json pos = json.at(Key_Point1);
        data.setPoint1(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    //Note (Aurélien) : compatibilty check
    else if (json.find("point1") != json.end())
    {
        nlohmann::json pos = json.at("point1");
        data.setPoint1(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "UserOrientation Point1 read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Point2) != json.end())
    {
        nlohmann::json pos = json.at(Key_Point2);
        data.setPoint2(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    //Note (Aurélien) : compatibilty check
    else if (json.find("point2") != json.end())
    {
        nlohmann::json pos = json.at("point2");
        data.setPoint2(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "UserOrientation Point2 read error" << LOGENDL;
        retVal = false;
    }

    UOAxisType axisType = UOAxisType::Custom;
    if (json.find(Key_AxisType) != json.end())
    {
        auto axisTypeOpt = magic_enum::enum_cast<UOAxisType>(json.at(Key_AxisType).get<std::string>());
        axisType = axisTypeOpt.has_value() ? axisTypeOpt.value() : UOAxisType::Custom;
        data.setAxisType(axisType);
    }
    //Note (Aurélien) : compatibilty check
    else if (json.find("AxisType") != json.end())
    {
        if (json.at("AxisType").get<bool>())
            data.setAxisType(UOAxisType::YAxis);
        else
            data.setAxisType(UOAxisType::XAxis);
    }
    else
    {
        IOLOG << "UserOrientation AxisPoints read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_CustomAxis) != json.end())
    {
        nlohmann::json customAxis = json.at(Key_CustomAxis);
        data.setCustomAxis({ Pos3D(customAxis[0][0], customAxis[0][1], customAxis[0][2])
                           , Pos3D(customAxis[1][0], customAxis[1][1], customAxis[1][2]) });
    }
    else if (axisType != UOAxisType::Custom)
    {
        if (axisType == UOAxisType::XAxis)
            data.setCustomAxis({ Pos3D(0.,0.,0.), Pos3D(1.,0.,0.) });
        else
            data.setCustomAxis({ Pos3D(0.,0.,0.), Pos3D(0.,1.,0.) });
    }
    else
    {
        IOLOG << "UserOrientation CustomAxis read error" << LOGENDL;
        retVal = false;
    }


    if (json.find(Key_OldPoint) != json.end())
    {
        nlohmann::json pos = json.at(Key_OldPoint);
        data.setOldPoint(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "UserOrientation OldPoint read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_NewPoint) != json.end())
    {
        nlohmann::json pos = json.at(Key_NewPoint);
        data.setNewPoint(Pos3D({ pos[0], pos[1], pos[2] }));
    }
    else
    {
        IOLOG << "UserOrientation NewPoint read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Order) != json.end())
        data.setOrder(json.at(Key_Order).get<uint32_t>());
    //Note (Aurélien) : compatibilty check
    else if (json.find("order") != json.end())
        data.setOrder(json.at("order").get<uint32_t>());
    else
    {
        IOLOG << "UserOrientation Order read error" << LOGENDL;
        retVal = false;
    }
    return retVal;
}

bool DataDeserializer::DeserializeProjectInfos(const nlohmann::json& json, const Controller& controller, ProjectInfos& data)
{
    bool retVal(true);

    if (json.find(Key_Project_Id) != json.end())
    {
        std::string id = json.at(Key_Project_Id).get<std::string>();
        data.m_id = xg::Guid(id);
    }
    else
    {
        data.m_id = xg::Guid();
    }

    if (json.find(Key_Company) != json.end())
        data.m_company = Utils::from_utf8(json.at(Key_Company).get<std::string>());
    else
    {
        IOLOG << "ProjectInfos Compagny read error" << LOGENDL;
    }

    if (json.find(Key_Author) != json.end())
    {
        Author auth;
        DeserializeAuthor(json.at(Key_Author), auth);
        data.m_author = controller.cgetContext().createAuthor(auth);
    }
    else
    {
        IOLOG << "ProjectInfos Author read error" << LOGENDL;
    }

    if (json.find(Key_Location) != json.end())
        data.m_location = Utils::from_utf8(json.at(Key_Location).get<std::string>());
    else
    {
        IOLOG << "ProjectInfos Location read error" << LOGENDL;
    }

    if (json.find(Key_Description) != json.end())
        data.m_description = Utils::from_utf8(json.at(Key_Description).get<std::string>());
    //Note (Aurélien) : compatibilty check 
    else if (json.find("Desc") != json.end())
        data.m_description = Utils::from_utf8(json.at("Desc").get<std::string>());
    else
    {
        IOLOG << "ProjectInfos Description read error" << LOGENDL;
    }

    if (json.find(Key_BeamBendingTolerance) != json.end())
        data.m_beamBendingTolerance = json.at(Key_BeamBendingTolerance).get<double>();
    //Note (Aurélien) : compatibilty check
    else if (json.find("BeambendingTolerance") != json.end())
        data.m_beamBendingTolerance = json.at("BeambendingTolerance").get<double>();
    else
    {
        IOLOG << "ProjectInfos BeamBendingTolerance read error" << LOGENDL;
    }

    if (json.find(Key_ColumnTiltTolerance) != json.end())
        data.m_columnTiltTolerance = json.at(Key_ColumnTiltTolerance).get<double>();
    else
    {
        IOLOG << "ProjectInfos ColumnTiltTolerance read error" << LOGENDL;
    }

    if (json.find(Key_DefaultClipMode) != json.end())
    {
        data.m_defaultClipMode = ClippingMode(json.at(Key_DefaultClipMode));
    }
    else
    {
        data.m_defaultClipMode = ClippingMode::showExterior;
    }

    if (json.find(Key_DefaultClipDistances) != json.end())
    {
        nlohmann::json distances = json.at(Key_DefaultClipDistances);
        data.m_defaultMinClipDistance = distances[0];
        data.m_defaultMaxClipDistance = distances[1];
    }
    else
    {
        data.m_defaultMinClipDistance = 0.0f;
        data.m_defaultMaxClipDistance = 0.5f;
        IOLOG << "ProjectInfos DefaultClipDistances reset" << LOGENDL;
    }

    if (json.find(Key_DefaultRampDistances) != json.end())
    {
        nlohmann::json distances = json.at(Key_DefaultRampDistances);
        data.m_defaultMinRampDistance = distances[0];
        data.m_defaultMaxRampDistance = distances[1];
    }
    else
    {
        data.m_defaultMinRampDistance = 0.0f;
        data.m_defaultMaxRampDistance = 0.5f;
        IOLOG << "ProjectInfos DefaultRampDistances reset" << LOGENDL;
    }

    if (json.find(Key_DefaultRampSteps) != json.end())
    {
        data.m_defaultRampSteps = json.at(Key_DefaultRampSteps);
    }
    else
    {
        data.m_defaultRampSteps = 8;
    }

    if (json.find(Key_DefaultScanId) != json.end())
        data.m_defaultScan = xg::Guid(json.at(Key_DefaultScanId).get<std::string>());
    else
        IOLOG << "ProjectInfos defaultScan not found" << LOGENDL;

    if (json.find(Key_ImportScanTranslation) != json.end())
    {
        nlohmann::json pos = json.at(Key_ImportScanTranslation);
        data.m_importScanTranslation = glm::dvec3(pos[0], pos[1], pos[2]);
    }
    else
        IOLOG << "ProjectInfos importScanTranslation not found" << LOGENDL;

    if (json.find(Key_CustomScanFolderPath) != json.end())
        data.m_customScanFolderPath = (Utils::from_utf8(json.at(Key_CustomScanFolderPath).get<std::string>()));
    else
        IOLOG << "ProjectInfos CustomScanFolderPath not found" << LOGENDL;

    return retVal;
}

bool DataDeserializer::DeserializeTagTemplate(const nlohmann::json& json, const Controller& controller, sma::TagTemplate& data)
{
    bool retVal(true);
    if (json.find(Key_Id) != json.end())
        data.setId(xg::Guid(json.at(Key_Id).get<std::string>()));
    //Note (Aurélien) : compatibilty check
    else if (json.find("TemplateId") != json.end())
        data.setId(xg::Guid(json.at("TemplateId").get<std::string>()));
    else
    {
        IOLOG << "TagTemplate Id read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Name) != json.end())
        data.renameTagTemplate(Utils::from_utf8(json.at(Key_Name).get<std::string>()));
    //Note (Aurélien) : compatibilty check
    else if (json.find("TemplateName") != json.end())
        data.renameTagTemplate(Utils::from_utf8(json.at("TemplateName").get<std::string>()));
    else
    {
        IOLOG << "TagTemplate Name read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_OriginTemplate) != json.end())
        data.setOriginTemplate(json.at(Key_OriginTemplate).get<bool>());
    //Note (Aurélien) : compatibilty check
    else if (json.find("originTemplate") != json.end())
        data.setOriginTemplate(json.at("originTemplate").get<bool>());
    else
    {
        IOLOG << "TagTemplate OriginTemplate read error" << LOGENDL;
        retVal = false;
    }

    if (json.find(Key_Fields) != json.end() && json.at(Key_Fields).is_array())
    {
        for (const nlohmann::json& iterator : json.at(Key_Fields))
        {
            sma::tField field;
            if (iterator.find(Key_Id) != iterator.end())
                field.m_id = xg::Guid(iterator.at(Key_Id).get<std::string>());
            else
            {
                IOLOG << "Field Id read error" << LOGENDL;
                retVal = false;
            }

            if (iterator.find(Key_Name) != iterator.end())
                field.m_name = Utils::from_utf8(iterator.at(Key_Name).get<std::string>());
            //Note (Aurélien) : compatibilty check
            else if (iterator.find("FieldName") != iterator.end())
                field.m_name = Utils::from_utf8(iterator.at("FieldName").get<std::string>());
            else
            {
                IOLOG << "Field Name read error" << LOGENDL;
                retVal = false;
            }

            if (iterator.find(Key_Type) != iterator.end())
            {
                auto type = magic_enum::enum_cast<sma::tFieldType>(iterator.at(Key_Type).get<std::string>());
                field.m_type = type.has_value() ? type.value() : sma::tFieldType::none;
            }
            else
            {
                IOLOG << "Field Type read error" << LOGENDL;
                retVal = false;
            }

            if (iterator.find(Key_Reference) != iterator.end())
                field.m_fieldReference = controller.cgetContext().getUserList(xg::Guid(iterator.at(Key_Reference).get<std::string>()));
            else
            {
                IOLOG << "Field Reference read error" << LOGENDL;
                retVal = false;
            }

            if (iterator.find(Key_DefaultValue) != iterator.end())
                field.m_defaultValue = Utils::from_utf8(iterator.at(Key_DefaultValue).get<std::string>());
            else
            {
                IOLOG << "Field DefaultValue read error" << LOGENDL;
                retVal = false;
            }
            data.addNewField(field);
        }
    }
    else
    {
        IOLOG << "TagTemplate Fields read error" << LOGENDL;
        retVal = false;
    }

    return retVal;
}

bool DataDeserializer::DeserializeAuthor(const nlohmann::json& json, Author& auth)
{
    if (json.find(Key_Id) != json.end())
    {
        xg::Guid authorId(json.at(Key_Id).get<std::string>());
        auth.setId(authorId);
    }
    else
        return false;

    if (json.find(Key_Name) != json.end())
    {
        std::string authorName(json.at(Key_Name).get<std::string>());
        auth.setName(Utils::from_utf8(authorName));
    }
    else
        return false;

    return true;
}
