#ifndef CLIPPING_GEOMETRY_H
#define CLIPPING_GEOMETRY_H

#include "models/data/Clipping/ClippingTypes.h"

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>

// Notes sur le design des classes "ClippingGeometry"
// 
// ===== Besoin =====
//  * Définir au sein d’une même classe des tests géométriques variant selon la forme.
//  * Regrouper un ensemble d’instance de cette classe (par exemple dans un std::vector).
//  * Effectuer ces tests de façon rapide avec le moins d’appel de fonction possible (jusqu’à 10k par image).
//  * Copier profondément les données pour éffectuer des transformations locales sans altérer les données d’origine.
// 
// ===== Premier design =====
//  * Une classe abstraite :
//    - Déclare des méthodes communes pour effectué différents tests
//    - Défini des variables membres communes servant dans les tests
// '''
// class IClippingGeometry (qu’on pourrait renomer plus proprement AClippingGeometry) {
// public:
//     ...
//     virtual void testSphere(...) const = 0;
//     virtual void testCube(...) const = 0;
//     virtual bool testPoint(...) const = 0;
// };
// '''
//
//  * Des classes héritant de 'IClippingGeometry' :
//    - Définissent les méthodes des test propre à chaque forme (shape)
// '''
// class BoxClippingGeometry : public IClippingGeometry {};
// class SphereClippingGeometry : public IClippingGeometry {};
// class CylinderClippingGeometry : public IClippingGeometry {};
// '''
//
//  * Inconvenient du design:
//    - On est obligé de stocker des pointeurs de 'IClippingGeometry' pour bénéficier du polymorphisme.
//    - Avant chaque modification des transformations dans un espace local on doit :
//       + Soit on doit faire une copie profonde du ClippingAssembly,
//       + soit on doit reset les matrices (et stocker leur valeurs initiales)
//    - 


enum class ClippingShape
{
    box,
    cylinder,
    sphere,
    torus,
    Max_Enum
};

// Tri-state enum that indicates the result of geometric clipping
enum class ClipResult
{
    Outside,
    Partial,
    Inside,
    Max_Enum
};

// Replace the "accept/reject" system
enum class ClipAccepted
{
    Yes,
    No,
    Partially,
    Max_Enum
};


class ClippingGeometry
{
public:
    ClippingGeometry(ClippingShape shape, ClippingMode mode, const glm::dmat4& matRT_inv, const glm::vec4& params, int steps);
    ~ClippingGeometry() {};

    typedef ClipResult(*fct_clipCube)(const glm::dvec3&, double, const ClippingGeometry& geom);
    fct_clipCube f_clipCube = nullptr;

    typedef ClipResult(*fct_clipSphere)(const glm::dvec4&, double, const ClippingGeometry& geom);
    fct_clipSphere f_clipSphere = nullptr;

    typedef ClipResult(*fct_clipPoint)(const glm::dvec4&, const ClippingGeometry& geom);
    fct_clipPoint f_clipPoint = nullptr;

    const ClippingShape shape;
    const ClippingMode mode;
    const ClipAccepted cst_decisionTable[(size_t)ClipResult::Max_Enum]; // init by the ClippingMode
    glm::dmat4 matRT_inv; // rotation-1 * translation-1
    glm::vec4 params; // Cf. definition des paramètres ci-dessous
    glm::vec3 color;
    int rampSteps;
    ClippingGpuId gpuDrawId; // optional for the export

    bool isSelected = false;
};

class IClippingGeometry
{
public:
    IClippingGeometry(ClippingMode mode, const glm::dmat4& matRT_inv, const glm::vec4& params, int steps, ClippingGpuId gpuDrawId);
    /*!
     * \brief Test if a sphere defined by its _center_ and _radius_ is clipped by the geometry.
     * Return _retAccept_ to true if the sphere is entirely not clipped.
     * Return _retReject_ to true if the sphere is entirely clipped.
     * If the sphere is partially clipped return both _retAccept_ and _retReject_ to false.
     */
    virtual void testSphere(const glm::dvec4& center, double radius, bool& retAccept, bool& retReject) const = 0;

    virtual void testCube(const glm::dvec3& minCorner, double sideSize, bool& retAccept, bool& retReject) const = 0;

    /*!
     * \brief Test if a _point_ is clipped by the geometry.
     * Return true if the point is not clipped (accepted).
     */
    virtual bool testPoint(const glm::dvec4& point) const = 0;

    /*!
    *  \brief return the specialized shape of the geometry.
    */
    virtual ClippingShape getShape() const = 0;

public:
    ClippingMode mode;
    glm::dmat4 matRT_inv; // rotation-1 * translation-1
    const glm::dmat4 matRT_inv_store; // rotation-1 * translation-1
    glm::vec4 params; // Cf. definition des paramètres ci-dessous
    glm::vec3 color;
    int rampSteps;
    ClippingGpuId gpuDrawId; // optional for the export
    std::wstring clipperPhase;

    bool isSelected = false;

    // Pour être plus flexible avec le type de géométrie que l’on veut décrire la "scale" n’est pas adaptée.
    // Chaque géométrie n’a pas besoin des mêmes paramètres :
    // * Box : on remplace le booléen "limitXY" par "dX" et "dY".
    //   * dX : limite en X (fini ou infini)
    //   * dY : limite en Y (fini ou infini)
    //   * dZ : limite en Z et (épaisseur de la boite)
    //
    // * Cylinder :
    //   * x : rayon intérieur
    //   * y : rayon extérieur
    //   * z : longeur
    //   * w : rayon paroi
    //
    // * Sphere :
    //   * x : rayon intérieur
    //   * y : rayon extérieur
    //   * z : N/A
    //   * w : rayon paroi
    // 
    // * Torus (partiel) :
    //   * x : rayon principal
    //   * y : cosinus de l’angle principal
    //   * z : rayon secondaire intérieur
    //   * w : rayon secondaire extérieur
};

class BoxClippingGeometry : public IClippingGeometry
{
public:
    BoxClippingGeometry(ClippingMode mode, const glm::dmat4& matRT_inv, const glm::vec4& params, int steps);
    void testSphere(const glm::dvec4& center, double radius, bool& retAccept, bool& retReject) const;
    void testCube(const glm::dvec3& minCorner, double sideSize, bool& retAccept, bool& retReject) const;
    bool testPoint(const glm::dvec4& point) const;
    ClippingShape getShape() const override;
};

class ABasicClippingGeometry : public IClippingGeometry
{
public:
    ABasicClippingGeometry(ClippingMode mode, const glm::dmat4& matRT_inv, const glm::vec4& params, int steps, ClippingGpuId gpuDrawId);
    void testCube(const glm::dvec3& minCorner, double sideSize, bool& retAccept, bool& retReject) const override;
};

class SphereClippingGeometry : public ABasicClippingGeometry
{
public:
    SphereClippingGeometry(ClippingMode mode, const glm::dmat4& matRT_inv, const glm::vec4& scale, int steps);
    void testSphere(const glm::dvec4& center, double radius, bool& retAccept, bool& retReject) const;
    bool testPoint(const glm::dvec4& point) const;
    ClippingShape getShape() const override;
};

class CylinderClippingGeometry : public ABasicClippingGeometry
{
public:
    CylinderClippingGeometry(ClippingMode mode, const glm::dmat4& matRT_inv, const glm::vec4& params, int steps);
    void testSphere(const glm::dvec4& center, double radius, bool& retAccept, bool& retReject) const;
    bool testPoint(const glm::dvec4& point) const;
    ClippingShape getShape() const;
};

class TorusClippingGeometry : public ABasicClippingGeometry
{
public:
    TorusClippingGeometry(ClippingMode mode, const glm::dmat4& matRT_inv, const glm::vec4& params, int steps);
    void testSphere(const glm::dvec4& center, double radius, bool& retAccept, bool& retReject) const;
    bool testPoint(const glm::dvec4& point) const;
    ClippingShape getShape() const;
};

class ClippingAssembly
{
public:
    ClippingAssembly();
    ~ClippingAssembly();

    void addTransformation(const glm::dmat4& matrix);
    void testSphere(const glm::dvec4& center, double radius, bool& retAccept, bool& retReject, ClippingAssembly& retAssembly) const;
    void testCube(const glm::dvec3& minCorner, double sideSize, bool& retAccept, bool& retReject, ClippingAssembly& retAssembly) const;
    bool testPoint(const glm::dvec4& point) const;
    void clearMatrix();
    bool empty();
    bool hasPhaseClipping() const;
    ClippingAssembly resolveByPhase(const std::wstring& scanPhase) const;
public:
    std::vector<std::shared_ptr<IClippingGeometry>> clippingUnion;
    std::vector<std::shared_ptr<IClippingGeometry>> clippingIntersection;
    std::vector<std::shared_ptr<IClippingGeometry>> rampActives;
};

// Le ClippingMode peut être vu autrement maintenant que l’on utilise les clipping
// pour les rampes et la coloration.
// On peut définir 3 zones lors de l’intersection de 2 géométries { confondu, partiellement confondu, dissocié }
// 
//  * ClippingMode::showInterior:
//    - On garde l’intérieur SANS traitement GPU (passer par le shader point_clip.geom)
//    - On garde la bordure AVEC traitement GPU
//    - On ne garde pas l’exterieur

//  * ClippingMode::

struct AssemblyDrawResult
{
    std::vector<uint32_t> unionIdxToTest;
    std::vector<uint32_t> interIdxToTest;
    std::vector<uint32_t> rampIdxToTest;
    std::vector<uint32_t> rampIdxToDraw;
};

// BaseAssembly
//  - Definition des opérations logiques de base dans une système de coordonnées.
//  - Initialisé en début de parcouns d'octree
class ClippingAssembly_bis
{
public:
    ClippingAssembly_bis();
    ~ClippingAssembly_bis();

    void addTransformation(const glm::dmat4& matrix);
    void testSphere(const glm::dvec4& center, double radius, bool& retAccept, bool& retReject, AssemblyDrawResult& result) const;
    void testCube(const glm::dvec3& minCorner, double sideSize, bool& retAccept, bool& retReject, const AssemblyDrawResult& srcAssembly, AssemblyDrawResult& retAssembly) const;
    bool testPoint(const glm::dvec4& point) const;

public:
    std::vector<ClippingGeometry> clippingUnion;
    std::vector<ClippingGeometry> clippingIntersection;
    std::vector<ClippingGeometry> rampActives;
};

#endif
