#include "models/application/List.h"
#include "gui/texts/DefaultUserLists.hpp"
#include "models/application/Ids.hpp"
#include "utils/Utils.h"


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
void List<std::wstring>::insertVector(const std::vector<std::array<std::wstring, 2>>& values, int li)
{
    for (const std::array<std::wstring, 2>& val : values)
    {
        m_elems.insert(val[li]);
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

std::map<xg::Guid, std::array<std::wstring, 2>> userlist_names = {
    { xg::Guid(LIST_DISCIPLINE_ID),     { L"Discipline", L"Discipline" } },
    { xg::Guid(LIST_PHASE_ID),          { L"Phase", L"Phase" } },
    { xg::Guid(LIST_FREQUENCY_ID),      { L"Frequency", L"Fréquence" } },
    { xg::Guid(LIST_LOD_ID),            { L"Level of detail", L"Niveau de détail" } },
    { xg::Guid(LIST_SEVERITY_ID),       { L"Severity", L"Gravité" } },
    { xg::Guid(LIST_MODELING_TYPE_ID),  { L"Modeling types", L"Type de modélisation" } },
    { xg::Guid(LIST_MOD_ACCU_ID),       { L"Modeling accuracy", L"Précision de la modélisation" } },
    { xg::Guid(LIST_STATUS_ID),         { L"Status", L"Statut" } },
    { xg::Guid(LIST_RISKS_ID),          { L"Risks", L"Risques" } },
    { xg::Guid(LIST_SECU_EQUIP_ID),     { L"Security equipment", L"Équipement de sécurité" } },
    { xg::Guid(LIST_NON_COMPLI_ID),     { L"Non-compliance", L"Non-conformités" } }
};

List<std::wstring> init_userlist(listId id, LanguageType language)
{
    std::wstring name = userlist_names.at(id)[(int)language];
    return List<std::wstring>(id, name);
}

std::array<std::wstring, 2> field_all = { L"All", L"Tous"};
std::array<std::wstring, 2> field_NA = { L"N.A.", L"Sans object" };

std::vector<std::array<std::wstring, 2>> l_discipline = {
    field_all,
    field_NA,
    { L"Alarm",              L"Alarme" },
    { L"Architecture",       L"Architecture" },
    { L"Asbestos",           L"Bardage" },
    { L"Certification",      L"Certification" },
    { L"Civil engineering",  L"Génie civil" },
    { L"Cladding",           L"Revêtements" },
    { L"Demolition",         L"Démolition" },
    { L"Diagnostic",         L"Diagnostique" },
    { L"Dismantling",        L"Désamiantage" },
    { L"Doors and openings", L"Portes et ouvertures" },
    { L"Electricity",        L"Électricité" },
    { L"Elevator",           L"Ascenseur" },
    { L"Fire protection",    L"Protection incendie" },
    { L"Flooring",           L"Revêtements de sol" },
    { L"Framework",          L"Charpente" },
    { L"Furnitures",         L"Mobilier" },
    { L"HVAC",               L"CVC" },
    { L"Heat production",    L"Production de chaleur" },
    { L"Inspection",         L"Inspection" },
    { L"Insulation",         L"Isolation" },
    { L"Investigation",      L"Investigations" },
    { L"Joineries",          L"Menuiseries" },
    { L"Lifting",            L"Levage" },
    { L"Maintenance",        L"Maintenance" },
    { L"Measurement",        L"Mesures" },
    { L"Metalwork",          L"Métallerie" },
    { L"Misc.",              L"Divers" },
    { L"Modeling",           L"Modélisation" },
    { L"Partition",          L"Cloisons" },
    { L"Piping",             L"Tuyauterie" },
    { L"Plumbing",           L"Plomberie" },
    { L"Power generation",   L"Production d'éléctricité" },
    { L"Recycling",          L"Recyclage" },
    { L"Refrigeration",      L"Production de froid" },
    { L"Roofing",            L"Couverture" },
    { L"Scaffolds",          L"Échafaudages" },
    { L"Security",           L"Sécurité" },
    { L"Special equipment",  L"Équipements spéciaux" },
    { L"Special fluids",     L"Fluides spéciaux" },
    { L"Storage",            L"Stockage" },
    { L"Surface treatment",  L"Traitement de surface" },
    { L"Water treatment"     L"Traitement des eaux" },
};

std::vector< std::array<std::wstring, 2>> l_phase = {
    field_all,
    field_NA,
    { L"Audit",             L"Audit" },
    { L"Diagnostic",        L"Diagnostique" },
    { L"Existing",          L"Éxistant" },
    { L"Project",           L"Projet" },
    { L"Call for tenders",  L"Appel d'offres" },
    { L"Technical studies", L"Études techniques" },
    { L"Demolition",        L"Démolition" },
    { L"Dismantling",       L"Désamiantage" },
    { L"Construction",      L"Construction" },
    { L"Reception",         L"Réception" },
    { L"Warranty",          L"Warranty" },
    { L"Production",        L"Production" },
    { L"Maintenance",       L"Maintenance" },
    { L"P1",                L"P1" },
    { L"P2",                L"P2" },
    { L"P3",                L"P3" },
    { L"P4",                L"P4" },
    { L"P5",                L"P5" },
    { L"P6",                L"P6" },
    { L"P7",                L"P7" },
    { L"P8",                L"P8" },
    { L"P9",                L"P9" },
    { L"P10"                L"P10" }
};

std::vector<std::array<std::wstring, 2>> l_frequency = {
    { L"Very low",  L"Très faible" },
    { L"Low",       L"Faible" },
    { L"Medium",    L"Modéré" },
    { L"High",      L"Élevé" },
    { L"Very high", L"Très élevé" }
};

std::vector<std::array<std::wstring, 2>> l_lod = {
    field_NA,
    { L"LOD100", L"LOD100" },
    { L"LOD200", L"LOD200" },
    { L"LOD300", L"LOD300" },
    { L"LOD350", L"LOD350" },
    { L"LOD400", L"LOD400" }
};

std::vector<std::array<std::wstring, 2>> l_severity = {
    { L"Catastrophic", L"Catastrophique" },
    { L"Major",        L"Majeur" },
    { L"Minor",        L"Mineur" },
    { L"Negligible",   L"Négligeable" },
    { L"Serious",      L"Sérieux" }
};

std::vector<std::array<std::wstring, 2>> l_modeling_type = {
    { L"2D drawing",      L"Dessin 2D" },
    { L"3D Solid",        L"Solides 3D CAO" },
    { L"BIM",             L"BIM" },
    { L"Mesh",            L"Maillage" },
    { L"Model in place",  L"In situ" }
};

std::vector<std::array<std::wstring, 2>> l_modeling_accuracy = {
    { L"1mm",  L"1mm" },
    { L"2mm",  L"2mm" },
    { L"3mm",  L"3mm" },
    { L"5mm",  L"5mm" },
    { L"10mm", L"10mm" },
    { L"20mm", L"20mm" },
    { L"30mm", L"30mm" },
    { L"50mm", L"50mm" }
};

std::vector<std::array<std::wstring, 2>> l_status = {
    field_NA,
    { L"To do",       L"À faire" },
    { L"In progress", L"En cours" },
    { L"Done",        L"Terminé" },
    { L"Validated",   L"Validé" },
    { L"Paused",      L"En pause" },
    { L"Cancelled",   L"Annulé" }
};

std::vector<std::array<std::wstring, 2>> l_risks = {
    { L"Asphyxiation",              L"Asphyxie" },
    { L"Burns - scalds",            L"Brûlure" },
    { L"Collision",                 L"Collision" },
    { L"Compressed gases",          L"Gaz sous pression" },
    { L"Corrosive",                 L"Corrosif" },
    { L"Cuts",                      L"Coupant" },
    { L"Electric shock",            L"Choc électrique" },
    { L"Entrapment",                L"Piège" },
    { L"Environ.toxicity",          L"Toxique pour l'environnement" },
    { L"Explosive",                 L"Explosif" },
    { L"Eye injury",                L"Blessures oculaires" },
    { L"Falls",                     L"Chute" },
    { L"Fire",                      L"Flammes" },
    { L"Flammable",                 L"Inflammable" },
    { L"Irritant",                  L"Irritant" },
    { L"Musculoskeletal disorders", L"TMS" },
    { L"Noise",                     L"Bruit" },
    { L"Obstruction",               L"Obstruction" },
    { L"Oxidizing",                 L"Oxydant" },
    { L"Posture hazards",           L"Mauvaise posture" },
    { L"Radiation",                 L"Radiations" },
    { L"Slippery",                  L"Glissant" },
    { L"Struck",                    L"Percuté" },
    { L"Toxic"                      L"Toxique" }
};

std::vector<std::array<std::wstring, 2>> l_security_equipment = {
    { L"Aid post",                 L"Poste de secours" },
    { L"Alarm",                    L"Alarme" },
    { L"Assembly point",           L"Point de rassemblement" },
    { L"Breaker",                  L"Disjoncteur" },
    { L"CO2 extinguisher",         L"Extincteur CO2" },
    { L"Chemical extinguisher",    L"Extincteur poudre ABC" },
    { L"Containment area",         L"Zone de confinement" },
    { L"Defibrillator",            L"Défibrillateur" },
    { L"Emergency exit",           L"Sortie de secours" },
    { L"Eye wash",                 L"Rince-œil" },
    { L"Fire door",                L"Porte coupe-feu" },
    { L"Fire hose",                L"Robinet incendie armé" },
    { L"Infirmary",                L"Infirmerie" },
    { L"Phone",                    L"Téléphone" },
    { L"SCBA",                     L"ARI" },
    { L"Shower",                   L"Douche" },
    { L"Smoke vent",               L"Désenfumage" },
    { L"Sprinkler",                L"Sprinkler" },
    { L"Stop push button",         L"Arrêt coup de poing" },
    { L"Stop valve",               L"Vanne d'arrêt" },
    { L"Water spray extinguisher", L"Extincteur eau pulvérisée" }
};

std::vector<std::array<std::wstring, 2>> l_non_compliance = {
    { L"Asperity",            L"Aspérité" },
    { L"Bad performances",    L"Mauvaises performances" },
    { L"Bad welding",         L"Mauvaise soudure" },
    { L"Badly painted",       L"Mal peint" },
    { L"Bare",                L"Dénudé" },
    { L"Bent",                L"Plié" },
    { L"Breakdown",           L"En panne" },
    { L"Broken",              L"Cassé" },
    { L"Bump",                L"Bosse" },
    { L"Burnt",               L"Brûlé" },
    { L"Clogged",             L"Bouché" },
    { L"Cluttered",           L"Encombré" },
    { L"Cold",                L"Gelé" },
    { L"Condensation",        L"Condensation" },
    { L"Contact failure",     L"Faux contact" },
    { L"Crack",               L"Fissure" },
    { L"Dark",                L"Sombre" },
    { L"Dirt",                L"Sale" },
    { L"Distorted",           L"Déformé" },
    { L"Dusty",               L"Poussière" },
    { L"Erased",              L"Effacé" },
    { L"Frozen",              L"Gelé" },
    { L"Hole",                L"Trou" },
    { L"Hollow",              L"Cavité" },
    { L"Hot",                 L"Chaud" },
    { L"Humid",               L"Humidité" },
    { L"Incorrect assembly",  L"Mauvais montage" },
    { L"Infiltration",        L"Infiltration" },
    { L"Leakage",             L"Fuite" },
    { L"Level too high",      L"Niveau trop haut" },
    { L"Level too low",       L"Niveau trop bas" },
    { L"Loose",               L"Desseré" },
    { L"Low light",           L"Faible luminosité" },
    { L"Low pressure",        L"Pression basse" },
    { L"Melted",              L"Fondu" },
    { L"Misalignment",        L"Non aligné" },
    { L"Mismatch",            L"Décalage" },
    { L"Mold",                L"Moisissure" },
    { L"Noisy",               L"Bruyant" },
    { L"Not calibrated",      L"Non calibré" },
    { L"Not insulated",       L"Non isolé" },
    { L"Not leveled",         L"Non nivelé" },
    { L"Not marked out",      L"Non repéré" },
    { L"Not protected",       L"Non protégé" },
    { L"Out of tolerance",    L"Hors tolérance" },
    { L"Outdated",            L"Obsolète" },
    { L"Overloaded",          L"Surchargé" },
    { L"Peeling",             L"S'écaille" },
    { L"Poor flow",           L"Faible débit" },
    { L"Poor signage",        L"Mauvaise signalisation" },
    { L"Poor ventilation",    L"Mauvaise ventilation" },
    { L"Pressure too high",   L"Pression trop haute" },
    { L"Pressure too low",    L"Pression trop basse" },
    { L"Puddle",              L"Flaque" },
    { L"Rusty",               L"Rouillé" },
    { L"Scratch",             L"Éraflé" },
    { L"Sharp",               L"Coupant" },
    { L"Slippery",            L"Glissant" },
    { L"Smell",               L"Odeur" },
    { L"Stagnant water",      L"Eau stagnante" },
    { L"Stain",               L"Tache" },
    { L"Torn",                L"Déchiré" },
    { L"Unscrewed",           L"Dévissé" },
    { L"Vibrations",          L"Vibrations" },
    { L"Wet",                 L"Mouillé" },
    { L"Wrong color",         L"Mauvaise couleur" },
    { L"Wrong item",          L"Mauvais élément" },
    { L"Wrong location",      L"Mauvais emplacement" },
    { L"Wrong size",          L"Mauvaise taille" }
};

std::vector<UserList> generateDefaultLists(LanguageType language)
{
    assert((int)language < 2);
    std::vector<UserList> lists;

    UserList discipline = init_userlist(xg::Guid(LIST_DISCIPLINE_ID), language);
    discipline.setOrigin(true);
    discipline.insertVector(l_discipline, (int)language);
    lists.push_back(discipline);

    UserList phase = init_userlist(xg::Guid(LIST_PHASE_ID), language);
    phase.setOrigin(true);
    phase.insertVector(l_phase, (int)language);
    lists.push_back(phase);

    UserList frequency = init_userlist(xg::Guid(LIST_FREQUENCY_ID), language);
    frequency.setOrigin(true);
    frequency.insertVector(l_frequency, (int)language);
    lists.push_back(frequency);

    UserList lod = init_userlist(xg::Guid(LIST_LOD_ID), language);
    lod.setOrigin(true);
    lod.insertVector(l_lod, (int)language);
    lists.push_back(lod);

    UserList severity = init_userlist(xg::Guid(LIST_SEVERITY_ID), language);
    severity.setOrigin(true);
    severity.insertVector(l_severity, (int)language);
    lists.push_back(severity);

    UserList modelingType = init_userlist(xg::Guid(LIST_MODELING_TYPE_ID), language);
    modelingType.setOrigin(true);
    modelingType.insertVector(l_modeling_type, (int)language);
    lists.push_back(modelingType);

    UserList modelingAcc = init_userlist(xg::Guid(LIST_MOD_ACCU_ID), language);
    modelingAcc.setOrigin(true);
    modelingAcc.insertVector(l_modeling_accuracy, (int)language);
    lists.push_back(modelingAcc);

    UserList status = init_userlist(xg::Guid(LIST_STATUS_ID), language);
    status.setOrigin(true);
    status.insertVector(l_status, (int)language);
    lists.push_back(status);

    UserList risks = init_userlist(xg::Guid(LIST_RISKS_ID), language);
    risks.setOrigin(true);
    risks.insertVector(l_risks, (int)language);
    lists.push_back(risks);

    UserList secEquip = init_userlist(xg::Guid(LIST_SECU_EQUIP_ID), language);
    secEquip.setOrigin(true);
    secEquip.insertVector(l_security_equipment, (int)language);
    lists.push_back(secEquip);

    UserList nonCompliance = init_userlist(xg::Guid(LIST_NON_COMPLI_ID), language);
    nonCompliance.setOrigin(true);
    nonCompliance.insertVector(l_non_compliance, (int)language);
    lists.push_back(nonCompliance);

    return lists;
}

std::map<xg::Guid, std::wstring> stdlist_names = {
    { xg::Guid(STD_PIPE_STAINLESS_SMS_EN_10357_ID), L"Stainless SMS EN 10357" },
    { xg::Guid(STD_PIPE_STAINLESS_DIN_11850_ID),    L"Stainless DIN 11850" },
    { xg::Guid(STD_PIPE_STEEL_ASME_B36_10_ID),      L"Steel ASME B36.10" },
    { xg::Guid(STD_PIPE_STAINLESS_EN_10217_7_ID),   L"Stainless EN 10217-7" },
    { xg::Guid(STD_PIPE_COPPER_EN_1057_ID),         L"Copper EN 1057" },
    { xg::Guid(STD_PIPE_PVC_EN_1453_1_ID),          L"PVC EN 1453-1" },
    { xg::Guid(STD_PIPE_DUCT_EN_1506_ID),           L"Duct EN 1506" },
    { xg::Guid(STD_PIPE_STEEL_EN_10216_ID),         L"Steel EN 10216" },
    { xg::Guid(STD_PIPE_STEEL_EN_10219_ID),         L"Steel EN 10219" }
};


List<double> init_stdlist(listId id)
{
    std::wstring name = stdlist_names.at(id);
    return List<double>(id, name);
}
#define STANDARDPIPE_NA_NAME L"N.A."

std::vector<StandardList> generateDefaultPipeStandardList()
{
    std::vector<StandardList> standardlists;

    StandardList naStandard(xg::Guid(STD_PIPE_NA_ID), STANDARDPIPE_NA_NAME);
    naStandard.setOrigin(true);
    standardlists.push_back(naStandard);

    StandardList stainlessSMSEN10357 = init_stdlist(xg::Guid(STD_PIPE_STAINLESS_SMS_EN_10357_ID));
    stainlessSMSEN10357.setOrigin(false);
    stainlessSMSEN10357.insertValue(0.025);
    stainlessSMSEN10357.insertValue(0.038);
    stainlessSMSEN10357.insertValue(0.051);
    stainlessSMSEN10357.insertValue(0.0635);
    stainlessSMSEN10357.insertValue(0.0761);
    stainlessSMSEN10357.insertValue(0.1016);
    stainlessSMSEN10357.insertValue(0.104);
    standardlists.push_back(stainlessSMSEN10357);

    StandardList stainlessDIN11850 = init_stdlist(xg::Guid(STD_PIPE_STAINLESS_DIN_11850_ID));
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

    StandardList steelASMEB36_10 = init_stdlist(xg::Guid(STD_PIPE_STEEL_ASME_B36_10_ID));
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

    StandardList stainlessEN_10217_7 = init_stdlist(xg::Guid(STD_PIPE_STAINLESS_EN_10217_7_ID));
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

    StandardList copperEN_1057 = init_stdlist(xg::Guid(STD_PIPE_COPPER_EN_1057_ID));
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

    StandardList PVC_EN_1453_1 = init_stdlist(xg::Guid(STD_PIPE_PVC_EN_1453_1_ID));
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

    StandardList DUCT_EN_1506 = init_stdlist(xg::Guid(STD_PIPE_DUCT_EN_1506_ID));
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

    StandardList STEEL_EN_10216 = init_stdlist(xg::Guid(STD_PIPE_STEEL_EN_10216_ID));
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

    StandardList STEEL_EN_10219 = init_stdlist(xg::Guid(STD_PIPE_STEEL_EN_10219_ID));
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