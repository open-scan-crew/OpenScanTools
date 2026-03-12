#ifndef VIEWPOINT_ANIMATION_H_
#define VIEWPOINT_ANIMATION_H_

#include "crossguid/guid.hpp"
#include "pointCloudEngine/RenderingTypes.h"

#include <QtCore/qstring.h>

#include <vector>

using viewPointAnimationId = xg::Guid;

enum class ViewPointAnimationMode
{
    PositionAsTime,
    ConstantSpeed,
    ConstantIntervals
};

struct ViewPointAnimationLine
{
    xg::Guid viewpointId;
    QString viewpointName;
    double position = 0.0;
};

class ViewPointAnimationConfig
{
public:
    ViewPointAnimationConfig();
    explicit ViewPointAnimationConfig(viewPointAnimationId id);

    viewPointAnimationId getId() const;
    const QString& getName() const;
    uint32_t getOrder() const;
    ViewPointAnimationMode getMode() const;
    const std::vector<ViewPointAnimationLine>& getLines() const;

    void setId(const viewPointAnimationId& id);
    void setName(const QString& name);
    void setOrder(uint32_t order);
    void setMode(ViewPointAnimationMode mode);
    void setLines(const std::vector<ViewPointAnimationLine>& lines);

private:
    viewPointAnimationId m_id;
    QString m_name;
    uint32_t m_order;
    ViewPointAnimationMode m_mode;
    std::vector<ViewPointAnimationLine> m_lines;
};

struct AnimationViewpointInfo
{
    xg::Guid id;
    QString name;
    ProjectionMode projectionMode = ProjectionMode::Perspective;
    UiRenderMode renderMode = UiRenderMode::RGB;
    BlendMode blendMode = BlendMode::Opaque;
    bool normals = false;
    bool blendColor = false;
    bool edgeAwareBlur = false;
    bool depthLining = false;
    bool depthLiningStrongMode = false;
};

#endif // VIEWPOINT_ANIMATION_H_
