#ifndef RENDERING_ECO_SYSTEM_H
#define RENDERING_ECO_SYSTEM_H

// Robin Kervadec - 16/06/2023
// 
// +++++ Problématique +++++
//  * L'affichage du nuage de point par le GPU est très couteux en temps et en énergie.
//  * On souhaite réduire au maximum les opérations qui aboutirait au même rendu que celui précédent.
//

// +++++ Analyse +++++
// Voici la liste détaillée des paramètres ayant une influence sur le rendu du nuage de point
//  * Viewport : taille, 
//  * Mode & Paramètres de rendu : taille de point, transparence, BCLS, normals, couleur pure, rampe
//  * Camera : position, fov, near / far
//  * Scan / PCO : position, couleur,
//  * Clipping{active} : position, scale / size, user scale
//  * Polyline{clipping::active} : nombre de mesure
//  * Objet{ ramp } : position,
//  * Suppression d'un objet : Scan, PCO, Clipping {active}, Objet {ramp}
//
// +++++ Système centralisé +++++
//  * Avant la refonte du modèle de donnée, tout changement aux données était centralisé par le
//    graph de scene. Ainsi, on pouvait déterminer si le renomage d'un tag, le déplacement d'un
//    cylindre ou le changement de couleur d'un scan était susceptible de modifier le rendu final.
//  * En outre, les paramètres d'affichage de lié à la vue (mode de rendu, taille de point,
//    transparence, résolution, position de camera, etc) étaient stockés par le viewport, le
//    rendering engine et la caméra. Leur changement était traqué par leur intermédiaire.
//
// +++++ Système individuel +++++
//  * Traquer la modification de tous ces paramètres individuellement est très difficile et
//    potentiellement très couteuse. Il faut noter que nous n'avons plus comme avant d'observateur
//    central pour filtrer les changement.
//  * Un système ou chaque objet tient compte de ses propres changement est complexe à développer, 
//    prompt aux erreurs et très difficile à maintenir.
//  * Un tel système n'est pas adapté en particulier pour évaluer l'effet d'une suppression d'objet 
//    puisque l'objet n'est plus là pour attester de sa modification.

#include "models/data/clipping/ClippingGeometry.h"
#include "models/3d/PointCloudDrawData.h"
#include "models/3d/DisplayParameters.h"
#include "vulkan/vulkan.h"

class HashFrame
{
public:
    static uint64_t hashRenderingData(VkExtent2D viewportExtent, const glm::dmat4& VP, const ClippingAssembly& clipAssembly, const std::vector<PointCloudDrawData>& m_pcDrawData, const DisplayParameters& displayParams);

    static uint64_t hashRenderingData_v2(const glm::dmat4& VP, const ClippingAssembly& clipAssembly, const std::vector<PointCloudDrawData>& m_pcDrawData, const DisplayParameters& displayParams);
};


#endif