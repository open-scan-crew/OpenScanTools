#ifndef MARKER_DEFINITION_H
#define MARKER_DEFINITION_H

#include "models/data/Marker.h"
#include "gui/texts/MarkerTexts.hpp"
#include "gui/texts/DefaultNameTexts.hpp"

#include <unordered_map>
#include <QtCore/qstring.h>

namespace scs
{
    const static std::unordered_map<MarkerIcon, MarkerSystem::Style> markerStyleDefs = {
        { MarkerIcon::Scan_Base, { MarkerShape::Top_Arrow, false, QString(":icons/100x100/survey_equipment.png"), TEXT_MARKER_DEFINITION_SURVEY_EQUIPEMENT } },
        { MarkerIcon::Target, { MarkerShape::Centered, false, QString(":icons/100x100/target.png"), TEXT_MARKER_DEFINITION_TARGET } },
        { MarkerIcon::BeamBending, { MarkerShape::Top_Arrow, false, QString(":icons/100x100/beam_bending.png"), TEXT_MARKER_DEFINITION_BEAMBENDING } },
        { MarkerIcon::ColumnTilt, { MarkerShape::Top_Arrow, false, QString(":/icons/100x100/column_tilt.png"), TEXT_MARKER_DEFINITION_COLUMNTILT } },
        { MarkerIcon::PointObject, { MarkerShape::Centered, false, QString(":icons/100x100/point.png"), TEXT_MARKER_DEFINITION_POINT } },
        { MarkerIcon::ViewPoint, { MarkerShape::Top_No_Arrow, false, QString(":icons/100x100/viewpoint.png"), TEXT_MARKER_DEFINITION_VIEWPOINT } },
        { MarkerIcon::Tag_Base, { MarkerShape::Top_Arrow, false, ":icons/tag/tag_base.png", TEXT_MARKER_DEFINITION_BASIC_TAG } },
        { MarkerIcon::Tag_Attention, { MarkerShape::Top_Arrow, false, ":icons/tag/tag_attention.png", TEXT_MARKER_DEFINITION_VIGILANCE } },
        { MarkerIcon::Tag_Cone, { MarkerShape::Top_Arrow, false, ":icons/tag/tag_cone.png", TEXT_MARKER_DEFINITION_CONE } },
        { MarkerIcon::Tag_Delete, { MarkerShape::Top_Arrow, false, ":icons/tag/tag_delete.png", TEXT_MARKER_DEFINITION_DELETE } },
        { MarkerIcon::Tag_Drop, { MarkerShape::Top_Arrow, false, ":icons/tag/tag_drop.png", TEXT_MARKER_DEFINITION_DROP } },
        { MarkerIcon::Tag_Flag, { MarkerShape::Top_Arrow, false, ":icons/tag/tag_flag.png", TEXT_MARKER_DEFINITION_FLAG } },
        { MarkerIcon::Tag_Information, { MarkerShape::Top_Arrow, false, ":icons/tag/tag_information.png",TEXT_MARKER_DEFINITION_INFORMATION } },
        { MarkerIcon::Tag_Instrumentation, { MarkerShape::Top_Arrow, false, ":icons/tag/tag_instrumentation.png", TEXT_MARKER_DEFINITION_INSTRUMENTATION } },
        { MarkerIcon::Tag_Recycle, { MarkerShape::Top_Arrow, false, ":icons/tag/tag_recycle.png", TEXT_MARKER_DEFINITION_RECYCLE } },
        { MarkerIcon::Tag_Repair, { MarkerShape::Top_Arrow, false, ":icons/tag/tag_repair.png", TEXT_MARKER_DEFINITION_REPAIR } },
        { MarkerIcon::Tag_Tick, { MarkerShape::Top_Arrow, false, ":icons/tag/tag_tick.png", TEXT_MARKER_DEFINITION_TICK } },
        { MarkerIcon::Tag_Trashcan, { MarkerShape::Top_Arrow, false, ":icons/tag/tag_trashcan.png", TEXT_MARKER_DEFINITION_TRASHCAN } },
        { MarkerIcon::Picto_Danger_EX, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_1_danger_atmosphere_explosive.png", TEXT_MARKER_DEFINITION_EXPLOSIVE_ATMOSPHERE } },
        { MarkerIcon::Picto_Danger_Fall, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_1_danger_chute.png", TEXT_MARKER_DEFINITION_RISK_OF_FAILING } },
        { MarkerIcon::Picto_Danger_Electricity, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_1_danger_electricite.png", TEXT_MARKER_DEFINITION_ELECTRICITY } },
        { MarkerIcon::Picto_Danger_Laser, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_1_danger_laser.png", TEXT_MARKER_DEFINITION_LASER } },
        { MarkerIcon::Picto_Danger_Radiation, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_1_danger_radiations.png", TEXT_MARKER_DEFINITION_RADIATIONS } },
        { MarkerIcon::Picto_Danger_Hot_Surface, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_1_danger_chaud.png", TEXT_MARKER_DEFINITION_HOT_SURFACE } },
        { MarkerIcon::Picto_Danger_Falling_Objects, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_1_danger_chute_objet.png", TEXT_MARKER_DEFINITION_FALLING_OBJECTS } },
        { MarkerIcon::Picto_Danger_Camera, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_1_danger_camera.png", TEXT_MARKER_DEFINITION_CAMERA } },
        { MarkerIcon::Picto_Danger_Forklift, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_1_danger_chariot.png", TEXT_MARKER_DEFINITION_FORKLIFT } },
        { MarkerIcon::Picto_Danger_Attention, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_danger_attention.png", TEXT_MARKER_DEFINITION_WARNING } },
        { MarkerIcon::Picto_Danger_Comburant, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_danger_comburant.png", TEXT_MARKER_DEFINITION_OXIDIZE } },
        { MarkerIcon::Picto_Danger_Combustible, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_danger_combustible.png", TEXT_MARKER_DEFINITION_FLAMMABLE } },
        { MarkerIcon::Picto_Danger_Explosive, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_danger_explosif.png", TEXT_MARKER_DEFINITION_EXPLOSIVE } },
        { MarkerIcon::Picto_Danger_Mortal, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_danger_mortel.png",TEXT_MARKER_DEFINITION_TOXIC } },
        { MarkerIcon::Picto_Danger_Pression, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_danger_pression.png", TEXT_MARKER_DEFINITION_PRESSURE_EQUIPEMENT } },
        { MarkerIcon::Picto_Danger_Health, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_danger_sante.png", TEXT_MARKER_DEFINITION_CARCINOGEN } },
        { MarkerIcon::Picto_Danger_Toxic, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_danger_toxique.png", TEXT_MARKER_DEFINITION_ENVIRONMENT } },
        { MarkerIcon::Picto_Epi_Ari, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_epi_ari.png", TEXT_MARKER_DEFINITION_BREATHING_APPARATUS } },
        { MarkerIcon::Picto_Epi_Helmet, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_epi_casque.png", TEXT_MARKER_DEFINITION_SAFETY_HELMET } },
        { MarkerIcon::Picto_Epi_Shoes, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_epi_chaussures.png", TEXT_MARKER_DEFINITION_SAFETY_SHOES } },
        { MarkerIcon::Picto_Epi_Filter, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_epi_filtre.png", TEXT_MARKER_DEFINITION_FILTER } },
        { MarkerIcon::Picto_Epi_Gloves, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_epi_gants.png", TEXT_MARKER_DEFINITION_SAFETY_GLOVES } },
        { MarkerIcon::Picto_Epi_Harness, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_epi_harnais.png", TEXT_MARKER_DEFINITION_HARNESS } },
        { MarkerIcon::Picto_Epi_Goggles, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_epi_lunettes.png", TEXT_MARKER_DEFINITION_SAFETY_GLASSES } },
        { MarkerIcon::Picto_Epi_Mask, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_epi_masque.png", TEXT_MARKER_DEFINITION_MASK } },
        { MarkerIcon::Picto_Epi_NoiseProtection, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_epi_protection_bruit.png", TEXT_MARKER_DEFINITION_EAR_PROTECTION } },
        { MarkerIcon::Picto_Epi_Uniform, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_epi_tenue.png", TEXT_MARKER_DEFINITION_WORK_CLOTHES } },
        { MarkerIcon::Picto_Epi_Hygiene_Cap, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_epi_charlotte.png", TEXT_MARKER_DEFINITION_HYGIENE_CAP } },
        { MarkerIcon::Picto_Epi_Overshoes, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_epi_surchaussures.png", TEXT_MARKER_DEFINITION_OVERSHOES } },
        { MarkerIcon::Picto_Rescue_Ari, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_secours_ari.png", TEXT_MARKER_DEFINITION_BREATHING_APPARATUS } },
        { MarkerIcon::Picto_Rescue_Stop, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_secours_arret.png",TEXT_MARKER_DEFINITION_STOP } },
        { MarkerIcon::Picto_Rescue_Defibrillator, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_secours_defibrillateur.png", TEXT_MARKER_DEFINITION_DEFIBRILLATOR } },
        { MarkerIcon::Picto_Rescue_Shower, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_secours_douche.png", TEXT_MARKER_DEFINITION_SHOWER } },
        { MarkerIcon::Picto_Rescue_RallyPoint, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_secours_rassemblement.png", TEXT_MARKER_DEFINITION_MEETING_POINT } },
        { MarkerIcon::Picto_Rescue_EyeShower, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_secours_rince_oeil.png", TEXT_MARKER_DEFINITION_EYE_WASH } },
        { MarkerIcon::Picto_Rescue_FirstAid, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_secours_soins.png", TEXT_MARKER_DEFINITION_INFIRMARY } },
        { MarkerIcon::Picto_Rescue_Exit, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_secours_sortie.png", TEXT_MARKER_DEFINITION_EMERGENCY_EXIT } },
        { MarkerIcon::Picto_Rescue_Phone, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_secours_telephone.png", TEXT_MARKER_DEFINITION_EMERGENCY_PHONE } },
        { MarkerIcon::Picto_Security_Alarm, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_secu_alarme.png", TEXT_MARKER_DEFINITION_ALARM } },
        { MarkerIcon::Picto_Security_Extinguisher, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_secu_extincteur.png", TEXT_MARKER_DEFINITION_FIRE_EXTINGUISHER } },
        { MarkerIcon::Picto_Security_Ria, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_secu_ria.png", TEXT_MARKER_DEFINITION_FIRE_HOSE } },
        { MarkerIcon::Picto_Security_Phone, { MarkerShape::Top_Arrow, true, ":icons/picto/picto_secu_tel.png", TEXT_MARKER_DEFINITION_FIRE_PHONE } },
        // Other objects type
        { MarkerIcon::Torus, { MarkerShape::Centered, false, ":icons/100x100/elbow.png", TEXT_DEFAULT_NAME_TORUS } },
        { MarkerIcon::Sphere, { MarkerShape::Centered, false, ":icons/100x100/sphere.png", TEXT_DEFAULT_NAME_SPHERE } },
        { MarkerIcon::Cylinder, { MarkerShape::Centered, false, ":icons/100x100/pipe.png", TEXT_DEFAULT_NAME_PIPE } },
        { MarkerIcon::Box, { MarkerShape::Centered, false, ":icons/100x100/Ortho-yk.png", TEXT_DEFAULT_NAME_BOX } },
        { MarkerIcon::Grid, { MarkerShape::Centered, false, ":icons/100x100/gridded_box.png", TEXT_DEFAULT_NAME_BOX } },
        { MarkerIcon::PCO, { MarkerShape::Centered, false, ":icons/100x100/global_box.png", TEXT_DEFAULT_NAME_BOX } },
        { MarkerIcon::Simple_Measure, { MarkerShape::Centered, false, ":icons/100x100/measure_points.png", TEXT_DEFAULT_NAME_SIMPLE_MEASURE } },
        { MarkerIcon::Polyline_Measure, { MarkerShape::Centered, false, ":icons/100x100/polyline.png", TEXT_DEFAULT_NAME_POLYLINE } },
        { MarkerIcon::PointToPlane_Measure, { MarkerShape::Centered, false, ":icons/100x100/point_plane_measurement.png", TEXT_DEFAULT_NAME_POINT_TO_PLANE } },
        { MarkerIcon::PipeToPipe_Measure, { MarkerShape::Centered, false, ":icons/100x100/cylinder_cylinder_measurement.png", TEXT_DEFAULT_NAME_PIPE_TO_PIPE } },
        { MarkerIcon::PointToPipe_Measure, { MarkerShape::Centered, false, ":icons/100x100/cylinder_point_measurement.png", TEXT_DEFAULT_NAME_POINT_TO_PIPE } },
        { MarkerIcon::PipeToPlane_Measure, { MarkerShape::Centered, false, ":icons/100x100/cylinder_plane_measurement.png", TEXT_DEFAULT_NAME_PIPE_TO_PLANE } },
        { MarkerIcon::MeshObject, { MarkerShape::Centered, false, ":icons/100x100/3dmodel.png", TEXT_DEFAULT_NAME_MESH } },
        /**/
    };


}

#endif