#include "pointCloudEngine/RenderingEcoSystem.h"

uint64_t hash_dmat4(const glm::dmat4& mat)
{
    uint64_t result = 0;
    std::hash<double> hash_fn_d;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result += hash_fn_d(mat[i][j]);
        }
    }
    return result;
}

uint64_t hash_dvec3(const glm::dvec3& vec)
{
    uint64_t result = 0;
    std::hash<double> hash_fn_d;
    for (int i = 0; i < 3; i++)
    {
        result += hash_fn_d(vec[i]);
    }
    return result;
}

uint64_t hash_vec4(const glm::vec4& vec)
{
    uint64_t result = 0;
    std::hash<float> hash_fn_f;
    for (int i = 0; i < 4; i++)
    {
        result += hash_fn_f(vec[i]);
    }
    return result;
}

uint64_t hash_vec3(const glm::vec3& vec)
{
    uint64_t result = 0;
    std::hash<float> hash_fn_f;
    for (int i = 0; i < 3; i++)
    {
        result += hash_fn_f(vec[i]);
    }
    return result;
}

uint64_t HashFrame::hashRenderingData(VkExtent2D viewportExtent, const glm::dmat4& VP, const ClippingAssembly& clipAssembly, const std::vector<PointCloudDrawData>& m_pcDrawData, const DisplayParameters& display)
{
    uint64_t hash = 0;

    std::hash<uint32_t> hash_fn_32;
    std::hash<uint64_t> hash_fn_64;
    std::hash<bool> hash_fn_b;
    std::hash<float> hash_fn_f;
    std::hash<double> hash_fn_d;
    std::hash<int> hash_fn_i;
    std::hash<std::wstring> hash_fn_w;

    hash += hash_fn_32(viewportExtent.width);
    hash += hash_fn_32(viewportExtent.height);
    hash += hash_dmat4(VP);

    // ClippingAssembly
    for (auto clipping : clipAssembly.clippingUnion)
    {
        hash += hash_fn_32((uint32_t)clipping->mode);
        hash += hash_dmat4(clipping->matRT_inv);
        hash += hash_vec4(clipping->params);
        hash += hash_fn_i(clipping->rampSteps);
        hash += hash_fn_w(clipping->clipperPhase);
    }

    for (auto clipping : clipAssembly.clippingIntersection)
    {
        hash += hash_fn_32((uint32_t)clipping->mode);
        hash += hash_dmat4(clipping->matRT_inv);
        hash += hash_vec4(clipping->params);
        hash += hash_fn_i(clipping->rampSteps);
        hash += hash_fn_w(clipping->clipperPhase);
    }

    for (auto clipping : clipAssembly.rampActives)
    {
        hash += hash_fn_32((uint32_t)clipping->mode);
        hash += hash_dmat4(clipping->matRT_inv);
        hash += hash_vec4(clipping->params);
        hash += hash_vec3(clipping->color);
        hash += hash_fn_i(clipping->rampSteps);
    }

    // PointCloudDrawData
    hash += hash_fn_64(m_pcDrawData.size());
    for (const PointCloudDrawData& pcdd : m_pcDrawData)
    {
        hash += hash_dmat4(pcdd.transfo);
        hash += hash_fn_32(*reinterpret_cast<uint32_t const*>(&pcdd.color));
        hash += hash_fn_32(pcdd.clippable);
        hash += hash_fn_w(pcdd.phase);
    }

    // DisplayParameters
    hash += hash_fn_32((uint32_t)display.m_mode);
    hash += hash_fn_32(*reinterpret_cast<uint32_t const*>(&display.m_backgroundColor));

    hash += hash_fn_f(display.m_pointSize);
    hash += hash_fn_i(display.m_texelThreshold);
    hash += hash_fn_f(display.m_deltaFilling);
    hash += hash_fn_f(display.m_contrast);    
    hash += hash_fn_f(display.m_brightness);
    hash += hash_fn_f(display.m_saturation);
    hash += hash_fn_f(display.m_luminance);
    hash += hash_fn_f(display.m_hue);
    hash += hash_vec3(display.m_flatColor);
    hash += hash_fn_f(display.m_distRampMin);
    hash += hash_fn_f(display.m_distRampMax);
    hash += hash_fn_i(display.m_distRampSteps);

    hash += hash_fn_32((uint32_t)display.m_blendMode);
    hash += hash_fn_32((uint32_t)display.m_negativeEffect);
    hash += hash_fn_32((uint32_t)display.m_reduceFlash);
    hash += hash_fn_32((uint32_t)display.m_flashAdvanced);
    hash += hash_fn_f(display.m_flashControl);
    hash += hash_fn_f(display.m_transparency);

    hash += hash_fn_b(display.m_postRenderingNormals.show);
    hash += hash_fn_b(display.m_postRenderingNormals.inverseTone);
    hash += hash_fn_b(display.m_postRenderingNormals.blendColor);
    hash += hash_fn_f(display.m_postRenderingNormals.normalStrength);
    hash += hash_fn_f(display.m_postRenderingNormals.gloss);
    hash += hash_fn_b(display.m_postRenderingAmbientOcclusion.enabled);
    hash += hash_fn_f(display.m_postRenderingAmbientOcclusion.radius);
    hash += hash_fn_f(display.m_postRenderingAmbientOcclusion.intensity);
    hash += hash_fn_b(display.m_edgeAwareBlur.enabled);
    hash += hash_fn_f(display.m_edgeAwareBlur.radius);
    hash += hash_fn_f(display.m_edgeAwareBlur.depthThreshold);
    hash += hash_fn_f(display.m_edgeAwareBlur.blendStrength);
    hash += hash_fn_f(display.m_edgeAwareBlur.resolutionScale);
    hash += hash_fn_b(display.m_depthLining.enabled);
    hash += hash_fn_f(display.m_depthLining.strength);
    hash += hash_fn_f(display.m_depthLining.threshold);
    hash += hash_fn_f(display.m_depthLining.sensitivity);
    hash += hash_fn_b(display.m_depthLining.strongMode);
    hash += hash_fn_b(display.m_colorimetricFilter.enabled);
    hash += hash_fn_b(display.m_colorimetricFilter.showColors);
    hash += hash_fn_f(display.m_colorimetricFilter.tolerance);
    for (size_t i = 0; i < display.m_colorimetricFilter.colors.size(); ++i)
    {
        hash += hash_fn_32(*reinterpret_cast<uint32_t const*>(&display.m_colorimetricFilter.colors[i]));
        hash += hash_fn_b(display.m_colorimetricFilter.colorsEnabled[i]);
    }

    hash += hash_fn_b(display.m_polygonalSelector.enabled);
    hash += hash_fn_b(display.m_polygonalSelector.showSelected);
    hash += hash_fn_b(display.m_polygonalSelector.active);
    hash += hash_fn_b(display.m_polygonalSelector.pendingApply);
    hash += hash_fn_32(display.m_polygonalSelector.appliedPolygonCount);
    hash += hash_fn_64(static_cast<uint64_t>(display.m_polygonalSelector.polygons.size()));
    for (const PolygonalSelectorPolygon& polygon : display.m_polygonalSelector.polygons)
    {
        hash += hash_fn_64(static_cast<uint64_t>(polygon.normalizedVertices.size()));
        for (const glm::vec2& vertex : polygon.normalizedVertices)
        {
            hash += hash_fn_f(vertex.x);
            hash += hash_fn_f(vertex.y);
        }

        for (int c = 0; c < 4; ++c)
            for (int r = 0; r < 4; ++r)
            {
                hash += hash_fn_d(polygon.camera.view[c][r]);
                hash += hash_fn_d(polygon.camera.proj[c][r]);
            }
        hash += hash_fn_32(polygon.camera.viewportWidth);
        hash += hash_fn_32(polygon.camera.viewportHeight);
        hash += hash_fn_b(polygon.camera.perspective);

        hash += hash_fn_64(static_cast<uint64_t>(polygon.snapshotUnion.size()));
        for (const PolygonalSelectorPolygon::SnapshotClip& clip : polygon.snapshotUnion)
        {
            hash += hash_fn_32(static_cast<uint32_t>(clip.shape));
            hash += hash_fn_32(static_cast<uint32_t>(clip.mode));
            hash += hash_fn_f(clip.params.x);
            hash += hash_fn_f(clip.params.y);
            hash += hash_fn_f(clip.params.z);
            hash += hash_fn_f(clip.params.w);
            for (int c = 0; c < 4; ++c)
                for (int r = 0; r < 4; ++r)
                    hash += hash_fn_d(clip.matRTInv[c][r]);
        }

        hash += hash_fn_64(static_cast<uint64_t>(polygon.snapshotIntersection.size()));
        for (const PolygonalSelectorPolygon::SnapshotClip& clip : polygon.snapshotIntersection)
        {
            hash += hash_fn_32(static_cast<uint32_t>(clip.shape));
            hash += hash_fn_32(static_cast<uint32_t>(clip.mode));
            hash += hash_fn_f(clip.params.x);
            hash += hash_fn_f(clip.params.y);
            hash += hash_fn_f(clip.params.z);
            hash += hash_fn_f(clip.params.w);
            for (int c = 0; c < 4; ++c)
                for (int r = 0; r < 4; ++r)
                    hash += hash_fn_d(clip.matRTInv[c][r]);
        }
    }

    //hash += hash_fn_f(display.m_alphaObject);             // Do not affect the point cloud

    return hash;
}

uint64_t HashFrame::hashRenderingData_v2(const glm::dmat4& VP, const ClippingAssembly& clipAssembly, const std::vector<PointCloudDrawData>& m_pcDrawData, const DisplayParameters& display)
{
    uint64_t hash = 0;

    size_t binarySize = sizeof(glm::dmat4);
    binarySize += (clipAssembly.clippingUnion.size() + clipAssembly.clippingIntersection.size()) * (sizeof(uint32_t) + sizeof(glm::dmat4) + sizeof(glm::dvec3));
    binarySize += m_pcDrawData.size() * (sizeof(glm::dmat4) + 2 * sizeof(uint32_t));
    binarySize += sizeof(DisplayParameters);

    char* clearData = new char[binarySize];


    

    delete[] clearData;
    return hash;
}
