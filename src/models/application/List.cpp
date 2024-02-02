#include "models/application/List.h"
#include "gui/texts/DefaultUserLists.hpp"
#include "models/application/Ids.hpp"
#include <nlohmannJson/json.hpp>

template<>
bool List<std::wstring>::insertStrValue(const std::wstring& strValue)
{
	insertValue(strValue);
	return true;
}

template<>
bool List<double>::insertStrValue(const std::wstring& strValue)
{
	try {
		insertValue(std::stod(strValue));
		return true;
	}
	catch (std::invalid_argument)
	{
		return false;
	}
}

template<>
std::string List<std::wstring>::toJson(const std::wstring& value)
{
	return Utils::to_utf8(value);
}

template<>
std::string List<double>::toJson(const double& value)
{
	return Utils::to_utf8(std::to_wstring(value));
}

std::vector<UserList> generateDefaultLists()
{
	std::vector<UserList> lists;

	UserList discipline(xg::Guid(LIST_DISCIPLINE_ID), DISCIPLINE_USERLIST_NAME.toStdWString());
	discipline.setOrigin(true);
	discipline.insertStrValue(ALL_FIELD_NAME.toStdWString());
	discipline.insertStrValue(NA_FIELD_NAME.toStdWString());
	discipline.insertStrValue(ALARM_FIELD_NAME.toStdWString());
	discipline.insertStrValue(ARCHITECTURE_FIELD_NAME.toStdWString());
	discipline.insertStrValue(ASBESTOS_FIELD_NAME.toStdWString());
	discipline.insertStrValue(CERIFICATION_FIELD_NAME.toStdWString());
	discipline.insertStrValue(CIVILENGINEERING_FIELD_NAME.toStdWString());
	discipline.insertStrValue(CLADDING_FIELD_NAME.toStdWString());
	discipline.insertStrValue(DEMOLITION_FIELD_NAME.toStdWString());
	discipline.insertStrValue(DIAGNOSTIC_FIELD_NAME.toStdWString());
	discipline.insertStrValue(DISMANTLING_FIELD_NAME.toStdWString());
	discipline.insertStrValue(DOORSANDOPENINGS_FIELD_NAME.toStdWString());
	discipline.insertStrValue(ELECTRICITY_FIELD_NAME.toStdWString());
	discipline.insertStrValue(ELEVATOR_FIELD_NAME.toStdWString());
	discipline.insertStrValue(FIERPROTECTION_FIELD_NAME.toStdWString());
	discipline.insertStrValue(FLOORING_FIELD_NAME.toStdWString());
	discipline.insertStrValue(FRAMEWORK_FIELD_NAME.toStdWString());
	discipline.insertStrValue(FURNITURES_FIELD_NAME.toStdWString());
	discipline.insertStrValue(HVAC_FIELD_NAME.toStdWString());
	discipline.insertStrValue(HEATPRODUCTION_FIELD_NAME.toStdWString());
	discipline.insertStrValue(INSPECTION_FIELD_NAME.toStdWString());
	discipline.insertStrValue(INSULATION_FIELD_NAME.toStdWString());
	discipline.insertStrValue(INVESTIGATION_FIELD_NAME.toStdWString());
	discipline.insertStrValue(JOINERIES_FIELD_NAME.toStdWString());
	discipline.insertStrValue(LIFTING_FIELD_NAME.toStdWString());
	discipline.insertStrValue(MAINTENANCE_FIELD_NAME.toStdWString());
	discipline.insertStrValue(MEASUREMENT_FIELD_NAME.toStdWString());
	discipline.insertStrValue(METALWORK_FIELD_NAME.toStdWString());
	discipline.insertStrValue(MISC_FIELD_NAME.toStdWString());
	discipline.insertStrValue(MODELING_FIELD_NAME.toStdWString());
	discipline.insertStrValue(PARTITION_FIELD_NAME.toStdWString());
	discipline.insertStrValue(PIPING_FIELD_NAME.toStdWString());
	discipline.insertStrValue(PLUMBING_FIELD_NAME.toStdWString());
	discipline.insertStrValue(POWERGENERATION_FIELD_NAME.toStdWString());
	discipline.insertStrValue(RECYLCLING_FIELD_NAME.toStdWString());
	discipline.insertStrValue(REFREGERATION_FIELD_NAME.toStdWString());
	discipline.insertStrValue(ROOFING_FIELD_NAME.toStdWString());
	discipline.insertStrValue(SCAFFOLDS_FIELD_NAME.toStdWString());
	discipline.insertStrValue(SECURITY_FIELD_NAME.toStdWString());
	discipline.insertStrValue(SPECIALEQUIPEMENT_FIELD_NAME.toStdWString());
	discipline.insertStrValue(SPECIALFUILDS_FIELD_NAME.toStdWString());
	discipline.insertStrValue(STORAGE_FIELD_NAME.toStdWString());
	discipline.insertStrValue(SURFACETREATEMENT_FIELD_NAME.toStdWString());
	discipline.insertStrValue(WATERTREATEMENT_FIELD_NAME.toStdWString());
	lists.push_back(discipline);

	UserList phase(xg::Guid(LIST_PHASE_ID), PHASE_USERLIST_NAME.toStdWString());
	phase.setOrigin(true);
	phase.insertStrValue(ALL_FIELD_NAME.toStdWString());
	phase.insertStrValue(NA_FIELD_NAME.toStdWString());
	phase.insertStrValue(AUDIT_FIELD_NAME.toStdWString());
	phase.insertStrValue(DIAGNOSTIC_FIELD_NAME.toStdWString());
	phase.insertStrValue(EXISTING_FIELD_NAME.toStdWString());
	phase.insertStrValue(PROJECT_FIELD_NAME.toStdWString());
	phase.insertStrValue(CALLFORTENDERS_FIELD_NAME.toStdWString());
	phase.insertStrValue(TECHNICALSTUDIES_FIELD_NAME.toStdWString());
	phase.insertStrValue(DEMOLITION_FIELD_NAME.toStdWString());
	phase.insertStrValue(DISMANTLING_FIELD_NAME.toStdWString());
	phase.insertStrValue(CONSTRUCTION_FIELD_NAME.toStdWString());
	phase.insertStrValue(RECEPTION_FIELD_NAME.toStdWString());
	phase.insertStrValue(WARRANTY_FIELD_NAME.toStdWString());
	phase.insertStrValue(PRODUCTION_FIELD_NAME.toStdWString());
	phase.insertStrValue(MAINTENANCE_FIELD_NAME.toStdWString());
	phase.insertStrValue(P1_FIELD_NAME.toStdWString());
	phase.insertStrValue(P2_FIELD_NAME.toStdWString());
	phase.insertStrValue(P3_FIELD_NAME.toStdWString());
	phase.insertStrValue(P4_FIELD_NAME.toStdWString());
	phase.insertStrValue(P5_FIELD_NAME.toStdWString());
	phase.insertStrValue(P6_FIELD_NAME.toStdWString());
	phase.insertStrValue(P7_FIELD_NAME.toStdWString());
	phase.insertStrValue(P8_FIELD_NAME.toStdWString());
	phase.insertStrValue(P9_FIELD_NAME.toStdWString());
	phase.insertStrValue(P10_FIELD_NAME.toStdWString());
	lists.push_back(phase);

	UserList frequency(xg::Guid(LIST_FREQUENCY_ID), FREQUENCY_USERLIST_NAME.toStdWString());
	frequency.setOrigin(true);
	frequency.insertStrValue(VERYLOW_FIELD_NAME.toStdWString());
	frequency.insertStrValue(LOW_FIELD_NAME.toStdWString());
	frequency.insertStrValue(MEDIUM_FIELD_NAME.toStdWString());
	frequency.insertStrValue(HIGH_FIELD_NAME.toStdWString());
	frequency.insertStrValue(VERYHIGH_FIELD_NAME.toStdWString());
	lists.push_back(frequency);

	UserList lod(xg::Guid(LIST_LOD_ID), LOD_USERLIST_NAME.toStdWString());
	lod.setOrigin(true);
	lod.insertStrValue(NA_FIELD_NAME.toStdWString());
	lod.insertStrValue(LOD100_FIELD_NAME.toStdWString());
	lod.insertStrValue(LOD200_FIELD_NAME.toStdWString());
	lod.insertStrValue(LOD300_FIELD_NAME.toStdWString());
	lod.insertStrValue(LOD350_FIELD_NAME.toStdWString());
	lod.insertStrValue(LOD400_FIELD_NAME.toStdWString());
	lists.push_back(lod);

	UserList severity(xg::Guid(LIST_SEVERITY_ID), SEVERITY_USERLIST_NAME.toStdWString());
	severity.setOrigin(true);
	severity.insertStrValue(CATASTROPHIC_FIELD_NAME.toStdWString());
	severity.insertStrValue(MAJOR_FIELD_NAME.toStdWString());
	severity.insertStrValue(MINOR_FIELD_NAME.toStdWString());
	severity.insertStrValue(NEGLIGIBLE_FIELD_NAME.toStdWString());
	severity.insertStrValue(SERIOUS_FIELD_NAME.toStdWString());
	lists.push_back(severity);

	UserList modelingType(xg::Guid(LIST_MODELING_TYPE_ID), MODELINGTYPE_USERLIST_NAME.toStdWString());
	modelingType.setOrigin(true);
	modelingType.insertStrValue(DRAWING2D_FIELD_NAME.toStdWString());
	modelingType.insertStrValue(DRAWING3D_FIELD_NAME.toStdWString());
	modelingType.insertStrValue(BIM_FIELD_NAME.toStdWString());
	modelingType.insertStrValue(MESH_FIELD_NAME.toStdWString());
	modelingType.insertStrValue(MODELINPLACE_FIELD_NAME.toStdWString());
	lists.push_back(modelingType);

	UserList modelingAcc(xg::Guid(LIST_MOD_ACCU_ID), MODELINGACCURACY_USERLIST_NAME.toStdWString());
	modelingAcc.setOrigin(true);
	modelingAcc.insertStrValue(M1_FIELD_NAME.toStdWString());
	modelingAcc.insertStrValue(M2_FIELD_NAME.toStdWString());
	modelingAcc.insertStrValue(M3_FIELD_NAME.toStdWString());
	modelingAcc.insertStrValue(M5_FIELD_NAME.toStdWString());
	modelingAcc.insertStrValue(M10_FIELD_NAME.toStdWString());
	modelingAcc.insertStrValue(M20_FIELD_NAME.toStdWString());
	modelingAcc.insertStrValue(M30_FIELD_NAME.toStdWString());
	modelingAcc.insertStrValue(M50_FIELD_NAME.toStdWString());
	lists.push_back(modelingAcc);

	UserList status(xg::Guid(LIST_STATUS_ID), STATUS_USERLIST_NAME.toStdWString());
	status.setOrigin(true);
	status.insertStrValue(NA_FIELD_NAME.toStdWString());
	status.insertStrValue(TODO_FIELD_NAME.toStdWString());
	status.insertStrValue(INPROGRESS_FIELD_NAME.toStdWString());
	status.insertStrValue(DONE_FIELD_NAME.toStdWString());
	status.insertStrValue(VALIDATED_FIELD_NAME.toStdWString());
	status.insertStrValue(PAUSED_FIELD_NAME.toStdWString());
	status.insertStrValue(CANCELLED_FIELD_NAME.toStdWString());
	lists.push_back(status);

	UserList risks(xg::Guid(LIST_RISKS_ID), RISKS_USERLIST_NAME.toStdWString());
	risks.setOrigin(true);
	risks.insertStrValue(ASPHYXIATION_FILED_NAME.toStdWString());
	risks.insertStrValue(BURNSSCALDS_FILED_NAME.toStdWString());
	risks.insertStrValue(COLLISION_FILED_NAME.toStdWString());
	risks.insertStrValue(COMPRESSEDGAZ_FILED_NAME.toStdWString());
	risks.insertStrValue(CORROSIVE_FILED_NAME.toStdWString());
	risks.insertStrValue(CUTS_FILED_NAME.toStdWString());
	risks.insertStrValue(ELECTRICSHOCK_FILED_NAME.toStdWString());
	risks.insertStrValue(ENTRAPEMENT_FILED_NAME.toStdWString());
	risks.insertStrValue(ENVITOXICITY_FILED_NAME.toStdWString());
	risks.insertStrValue(EXPLOSIVE_FILED_NAME.toStdWString());
	risks.insertStrValue(EYEINJURY_FILED_NAME.toStdWString());
	risks.insertStrValue(FALLS_FILED_NAME.toStdWString());
	risks.insertStrValue(FIRE_FILED_NAME.toStdWString());
	risks.insertStrValue(FLAMMABLE_FILED_NAME.toStdWString());
	risks.insertStrValue(IRRITANT_FILED_NAME.toStdWString());
	risks.insertStrValue(MUSCULOSKELETALDISORDER_FILED_NAME.toStdWString());
	risks.insertStrValue(NOISE_FILED_NAME.toStdWString());
	risks.insertStrValue(OBSTRUCTION_FILED_NAME.toStdWString());
	risks.insertStrValue(OCIDIZING_FILED_NAME.toStdWString());
	risks.insertStrValue(POSTUREHAZARD_FILED_NAME.toStdWString());
	risks.insertStrValue(RADIATION_FILED_NAME.toStdWString());
	risks.insertStrValue(SLIPPERY_FILED_NAME.toStdWString());
	risks.insertStrValue(STRUCK_FILED_NAME.toStdWString());
	risks.insertStrValue(TOXIC_FILED_NAME.toStdWString());
	lists.push_back(risks);

	UserList secEquip(xg::Guid(LIST_SECU_EQUIP_ID), SECURITYEQUIPEMENT_USERLIST_NAME.toStdWString());
	secEquip.setOrigin(true);
	secEquip.insertStrValue(AIDPOST_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(ALARM_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(ASSEMBLYPOINT_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(BREAKER_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(CO2EXTINGUISHER_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(CHEMICALEXTINGUISHER_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(CONTAINMENTAREA_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(DEFIBRILLATOR_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(EMERGENCYEXIT_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(EYEWASH_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(FIREDOOR_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(FIREHOSE_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(INFIRMARY_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(PHONE_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(SCBA_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(SHOWER_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(SMOKEVENT_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(SPRINKLER_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(STOPPUSHBUTTON_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(STOPVALVE_FIELD_NAME.toStdWString());
	secEquip.insertStrValue(WATERSPRAYEXTINGUISHER_FIELD_NAME.toStdWString());
	lists.push_back(secEquip);

	UserList nonCompliance(xg::Guid(LIST_NON_COMPLI_ID), NONCONPLIANTCE_USERLIST_NAME.toStdWString());
	nonCompliance.setOrigin(true);
	nonCompliance.insertStrValue(ASPERITY_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(BADPERFORMANCES_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(BADWELDING_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(BADLYPAINTED_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(BARE_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(BENT_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(BREAKDOWN_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(BROKEN_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(BUMP_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(BURNT_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(CLOGGED_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(CLUTTERED_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(COLD_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(CONDENSATION_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(CONTACTFAILURE_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(CRACK_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(DARK_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(DIRT_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(DISTORTED_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(DUSTY_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(ERASED_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(FROZEN_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(HOLE_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(HOLLOW_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(HOT_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(HUMID_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(INCORRECTASSEMBLY_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(INFILTRATION_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(LEAKAGE_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(LEVELTOOHIGH_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(LEVELTOOLOW_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(LOOSE_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(LOWLIGHT_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(LOWPRESSURE_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(MELTED_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(MISALIGNEMENT_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(MISMATCH_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(MOLD_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(NOISY_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(NOTCALIBRATED_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(NOTINSULATED_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(NOTLEVELED_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(NOTMARKEDOUT_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(NOTPROTECTED_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(OUTOFTOLERANCE_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(OUTDATED_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(OVERLOADED_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(PEELING_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(POORFLOW_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(POORSIGNAGE_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(POORVENTILATION_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(PRESSURETOOHIGH_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(PRESSURETOOLOW_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(PUDDLE_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(RUSTY_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(SCRATCH_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(SHARP_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(SLIPPERY_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(SMELL_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(STAGNANTWATER_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(STAIN_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(TORN_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(UNSCREWED_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(VIBRATIONS_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(WET_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(WRONGCOLOR_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(WRONGITEM_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(WRONGLOCATION_FILED_NAME.toStdWString());
	nonCompliance.insertStrValue(WRONGSIZE_FILED_NAME.toStdWString());
	lists.push_back(nonCompliance);

	return lists;
}

std::vector<StandardList> generateDefaultPipeStandardList()
{
	std::vector<StandardList> standardlists;

	StandardList naStandard(xg::Guid(STANDARDPIPE_NA_ID), STANDARDPIPE_NA_NAME);
	naStandard.setOrigin(true);
	standardlists.push_back(naStandard);

	StandardList stainlessSMSEN10357(xg::Guid(STANDARDPIPE_STAINLESS_SMS_EN_10357_ID), STANDARDPIPE_STAINLESS_SMS_EN_10357_NAME);
	stainlessSMSEN10357.setOrigin(false);
	stainlessSMSEN10357.insertValue(0.025);
	stainlessSMSEN10357.insertValue(0.038);
	stainlessSMSEN10357.insertValue(0.051);
	stainlessSMSEN10357.insertValue(0.0635);
	stainlessSMSEN10357.insertValue(0.0761);
	stainlessSMSEN10357.insertValue(0.1016);
	stainlessSMSEN10357.insertValue(0.104);
	standardlists.push_back(stainlessSMSEN10357);

	StandardList stainlessDIN11850(xg::Guid(STANDARDPIPE_STAINLESS_DIN_11850_ID), STANDARDPIPE_STAINLESS_DIN_11850_NAME);
	stainlessDIN11850.setOrigin(false);
	stainlessDIN11850.insertValue(0.023);
	stainlessDIN11850.insertValue(0.029);
	stainlessDIN11850.insertValue(0.035);
	stainlessDIN11850.insertValue(0.041);
	stainlessDIN11850.insertValue(0.053);
	stainlessDIN11850.insertValue(0.07);
	stainlessDIN11850.insertValue(0.085);
	stainlessDIN11850.insertValue(0.104);
	stainlessDIN11850.insertValue(0.129);
	stainlessDIN11850.insertValue(0.154);
	standardlists.push_back(stainlessDIN11850);

	StandardList steelASMEB36_10(xg::Guid(STANDARDPIPE_STEEL_ASME_B36_10_ID), STANDARDPIPE_STEEL_ASME_B36_10_NAME);
	steelASMEB36_10.setOrigin(false);
	steelASMEB36_10.insertValue(0.0213);
	steelASMEB36_10.insertValue(0.0267);
	steelASMEB36_10.insertValue(0.0334);
	steelASMEB36_10.insertValue(0.0422);
	steelASMEB36_10.insertValue(0.0483);
	steelASMEB36_10.insertValue(0.0603);
	steelASMEB36_10.insertValue(0.0889);
	steelASMEB36_10.insertValue(0.1016);
	steelASMEB36_10.insertValue(0.1143);
	steelASMEB36_10.insertValue(0.1413);
	steelASMEB36_10.insertValue(0.1683);
	steelASMEB36_10.insertValue(0.2191);
	steelASMEB36_10.insertValue(0.273);
	steelASMEB36_10.insertValue(0.3239);
	steelASMEB36_10.insertValue(0.3556);
	steelASMEB36_10.insertValue(0.4064);
	steelASMEB36_10.insertValue(0.457);
	steelASMEB36_10.insertValue(0.508);
	steelASMEB36_10.insertValue(0.559);
	steelASMEB36_10.insertValue(0.61);
	steelASMEB36_10.insertValue(0.66);
	steelASMEB36_10.insertValue(0.711);
	steelASMEB36_10.insertValue(0.762);
	steelASMEB36_10.insertValue(0.813);
	steelASMEB36_10.insertValue(0.864);
	steelASMEB36_10.insertValue(0.914);
	steelASMEB36_10.insertValue(0.965);
	steelASMEB36_10.insertValue(1.016);
	steelASMEB36_10.insertValue(1.118);
	steelASMEB36_10.insertValue(1.168);
	steelASMEB36_10.insertValue(1.219);
	steelASMEB36_10.insertValue(1.321);
	steelASMEB36_10.insertValue(1.422);
	steelASMEB36_10.insertValue(1.524);
	steelASMEB36_10.insertValue(1.626);
	steelASMEB36_10.insertValue(1.727);
	steelASMEB36_10.insertValue(1.829);
	steelASMEB36_10.insertValue(1.93);
	steelASMEB36_10.insertValue(2.032);
	standardlists.push_back(steelASMEB36_10);

	StandardList stainlessEN_10217_7(xg::Guid(STANDARDPIPE_STAINLESS_EN_10217_7_ID), STANDARDPIPE_STAINLESS_EN_10217_7_NAME);
	stainlessEN_10217_7.setOrigin(false);
	stainlessEN_10217_7.insertValue(0.0213);
	stainlessEN_10217_7.insertValue(0.0269);
	stainlessEN_10217_7.insertValue(0.0337);
	stainlessEN_10217_7.insertValue(0.0424);
	stainlessEN_10217_7.insertValue(0.0483);
	stainlessEN_10217_7.insertValue(0.0603);
	stainlessEN_10217_7.insertValue(0.0761);
	stainlessEN_10217_7.insertValue(0.0889);
	stainlessEN_10217_7.insertValue(0.1016);
	stainlessEN_10217_7.insertValue(0.1143);
	stainlessEN_10217_7.insertValue(0.1397);
	stainlessEN_10217_7.insertValue(0.1683);
	stainlessEN_10217_7.insertValue(0.2191);
	stainlessEN_10217_7.insertValue(0.273);
	stainlessEN_10217_7.insertValue(0.3239);
	stainlessEN_10217_7.insertValue(0.3556);
	stainlessEN_10217_7.insertValue(0.4064);
	stainlessEN_10217_7.insertValue(0.4572);
	stainlessEN_10217_7.insertValue(0.508);
	stainlessEN_10217_7.insertValue(0.61);
	standardlists.push_back(stainlessEN_10217_7);

	StandardList copperEN_1057(xg::Guid(STANDARDPIPE_COPPER_EN_1057_ID), STANDARDPIPE_COPPER_EN_1057_NAME);
	copperEN_1057.setOrigin(false);
	copperEN_1057.insertValue(0.012);
	copperEN_1057.insertValue(0.014);
	copperEN_1057.insertValue(0.016);
	copperEN_1057.insertValue(0.018);
	copperEN_1057.insertValue(0.022);
	copperEN_1057.insertValue(0.028);
	copperEN_1057.insertValue(0.035);
	copperEN_1057.insertValue(0.042);
	copperEN_1057.insertValue(0.054);
	copperEN_1057.insertValue(0.064);
	copperEN_1057.insertValue(0.0761);
	copperEN_1057.insertValue(0.0889);
	copperEN_1057.insertValue(0.108);
	copperEN_1057.insertValue(0.133);
	copperEN_1057.insertValue(0.159);
	copperEN_1057.insertValue(0.219);
	copperEN_1057.insertValue(0.267);
	standardlists.push_back(copperEN_1057);

	StandardList PVC_EN_1453_1(xg::Guid(STANDARDPIPE_PVC_EN_1453_1_ID), STANDARDPIPE_PVC_EN_1453_1_NAME);
	PVC_EN_1453_1.setOrigin(false);
	PVC_EN_1453_1.insertValue(0.032);
	PVC_EN_1453_1.insertValue(0.04);
	PVC_EN_1453_1.insertValue(0.05);
	PVC_EN_1453_1.insertValue(0.063);
	PVC_EN_1453_1.insertValue(0.075);
	PVC_EN_1453_1.insertValue(0.08);
	PVC_EN_1453_1.insertValue(0.1);
	PVC_EN_1453_1.insertValue(0.11);
	PVC_EN_1453_1.insertValue(0.125);
	PVC_EN_1453_1.insertValue(0.14);
	PVC_EN_1453_1.insertValue(0.16);
	PVC_EN_1453_1.insertValue(0.2);
	PVC_EN_1453_1.insertValue(0.25);
	PVC_EN_1453_1.insertValue(0.315);
	standardlists.push_back(PVC_EN_1453_1);

	StandardList DUCT_EN_1506(xg::Guid(STANDARDPIPE_DUCT_EN_1506_ID), STANDARDPIPE_DUCT_EN_1506_NAME);
	DUCT_EN_1506.setOrigin(false);
	DUCT_EN_1506.insertValue(0.08);
	DUCT_EN_1506.insertValue(0.1);
	DUCT_EN_1506.insertValue(0.125);
	DUCT_EN_1506.insertValue(0.16);
	DUCT_EN_1506.insertValue(0.2);
	DUCT_EN_1506.insertValue(0.25);
	DUCT_EN_1506.insertValue(0.315);
	DUCT_EN_1506.insertValue(0.355);
	DUCT_EN_1506.insertValue(0.4);
	DUCT_EN_1506.insertValue(0.45);
	DUCT_EN_1506.insertValue(0.5);
	DUCT_EN_1506.insertValue(0.56);
	DUCT_EN_1506.insertValue(0.63);
	DUCT_EN_1506.insertValue(0.71);
	DUCT_EN_1506.insertValue(0.8);
	DUCT_EN_1506.insertValue(0.9);
	DUCT_EN_1506.insertValue(1.0);
	DUCT_EN_1506.insertValue(1.12);
	DUCT_EN_1506.insertValue(1.25);
	standardlists.push_back(DUCT_EN_1506);

	StandardList STEEL_EN_10216(xg::Guid(STANDARDPIPE_STEEL_EN_10216_ID), STANDARDPIPE_STEEL_EN_10216_NAME);
	STEEL_EN_10216.setOrigin(false);
	STEEL_EN_10216.insertValue(0.0213);
	STEEL_EN_10216.insertValue(0.0269);
	STEEL_EN_10216.insertValue(0.0337);
	STEEL_EN_10216.insertValue(0.0424);
	STEEL_EN_10216.insertValue(0.0483);
	STEEL_EN_10216.insertValue(0.0603);
	STEEL_EN_10216.insertValue(0.0761);
	STEEL_EN_10216.insertValue(0.0889);
	STEEL_EN_10216.insertValue(0.1016);
	STEEL_EN_10216.insertValue(0.1143);
	STEEL_EN_10216.insertValue(0.1397);
	STEEL_EN_10216.insertValue(0.1683);
	STEEL_EN_10216.insertValue(0.2191);
	STEEL_EN_10216.insertValue(0.273);
	STEEL_EN_10216.insertValue(0.3239);
	STEEL_EN_10216.insertValue(0.3556);
	STEEL_EN_10216.insertValue(0.4064);
	STEEL_EN_10216.insertValue(0.4572);
	STEEL_EN_10216.insertValue(0.508);
	STEEL_EN_10216.insertValue(0.61);
	standardlists.push_back(STEEL_EN_10216);

	StandardList STEEL_EN_10219(xg::Guid(STANDARDPIPE_STEEL_EN_10219_ID), STANDARDPIPE_STEEL_EN_10219_NAME);
	STEEL_EN_10219.setOrigin(false);
	STEEL_EN_10219.insertValue(0.0213);
	STEEL_EN_10219.insertValue(0.0269);
	STEEL_EN_10219.insertValue(0.0337);
	STEEL_EN_10219.insertValue(0.0424);
	STEEL_EN_10219.insertValue(0.0483);
	STEEL_EN_10219.insertValue(0.0603);
	STEEL_EN_10219.insertValue(0.0761);
	STEEL_EN_10219.insertValue(0.0889);
	STEEL_EN_10219.insertValue(0.1016);
	STEEL_EN_10219.insertValue(0.1143);
	STEEL_EN_10219.insertValue(0.1397);
	STEEL_EN_10219.insertValue(0.1683);
	STEEL_EN_10219.insertValue(0.2191);
	STEEL_EN_10219.insertValue(0.273);
	STEEL_EN_10219.insertValue(0.3239);
	STEEL_EN_10219.insertValue(0.3556);
	STEEL_EN_10219.insertValue(0.4064);
	STEEL_EN_10219.insertValue(0.4572);
	STEEL_EN_10219.insertValue(0.508);
	standardlists.push_back(STEEL_EN_10219);

	return standardlists;
}