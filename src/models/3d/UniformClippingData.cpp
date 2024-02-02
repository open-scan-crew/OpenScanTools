#include "models/3d/UniformClippingData.h"
#include "models/data/Clipping/ClippingGeometry.h"

constexpr uint32_t SHAPE_SHIFT = 12;
constexpr uint32_t TYPE_SHIFT = 14;

void generateUniformData(ClippingAssembly& _ca, std::vector<UniformClippingData>& _shaderData)
{
    // NOTE - Extra Datas
    //  * The extra data is not used by the shaders.
    //  * The data must be filled anyways to keep a good alignment (ramp, colored)

    for (std::shared_ptr<IClippingGeometry>& cg : _ca.clippingUnion)
    {
        cg->gpuDrawId = 0;
        cg->gpuDrawId += (uint16_t)_shaderData.size();  // Il y a de la place pour 4095 matrices
        cg->gpuDrawId += (uint16_t)cg->getShape() << SHAPE_SHIFT;
        cg->gpuDrawId += ((cg->mode == ClippingMode::showInterior) ? 0 : 1) << TYPE_SHIFT;

        _shaderData.emplace_back(UniformClippingData{ cg->matRT_inv, cg->params, cg->color, cg->rampSteps });
    }
    for (std::shared_ptr<IClippingGeometry>& cg : _ca.clippingIntersection)
    {
        cg->gpuDrawId = 0;
        cg->gpuDrawId += (uint16_t)_shaderData.size();  // Il y a de la place pour 4095 matrices
        cg->gpuDrawId += (uint16_t)cg->getShape() << SHAPE_SHIFT;
        cg->gpuDrawId += ((cg->mode == ClippingMode::showInterior) ? 0 : 1) << TYPE_SHIFT;

        _shaderData.emplace_back(UniformClippingData{ cg->matRT_inv, cg->params, cg->color, cg->rampSteps });
    }

    for (std::shared_ptr<IClippingGeometry>& cg : _ca.rampActives)
    {
        cg->gpuDrawId = 0;
        cg->gpuDrawId += (uint16_t)_shaderData.size();
        cg->gpuDrawId += (uint16_t)cg->getShape() << SHAPE_SHIFT;
        cg->gpuDrawId += (cg->rampSteps == 1 ? 2 : 3) << TYPE_SHIFT;

        _shaderData.emplace_back(UniformClippingData{ cg->matRT_inv, cg->params, cg->color, cg->rampSteps });
    }
}