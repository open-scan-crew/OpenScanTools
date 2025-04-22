#include "gui/style/IconObject.h"
#include "services/MarkerDefinitions.hpp"
#include <QtGui/qpainter.h>
#include <QtGui/qimage.h>
#include <QtWidgets/qgraphicseffect.h>
#include <QtWidgets/qgraphicsscene.h>
#include <QtWidgets/qgraphicsitem.h>

#include "utils/Utils.h"

void tint(QImage& src, Color32 color, qreal strength = 1.0) {
    if (src.isNull())
        return;
    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(QPixmap::fromImage(src));
    QGraphicsColorizeEffect effect;
    effect.setColor(QColor(color.r, color.g, color.b));
    effect.setStrength(strength);
    item.setGraphicsEffect(&effect);
    scene.addItem(&item);
    QPainter ptr(&src);
    scene.render(&ptr, QRectF(), src.rect());
}

QPixmap scs::IconManager::getIcon(scs::MarkerIcon icon, Color32 color)
{
    uint64_t hashIcon = getHash(icon, color);
    if (m_hashmapIcons.find(hashIcon) != m_hashmapIcons.end())
        return m_hashmapIcons.at(hashIcon);

    QPixmap pixmap = QPixmap();

    if (scs::markerStyleDefs.find(icon) != scs::markerStyleDefs.end())
    {
        scs::MarkerStyleDefinition style = scs::markerStyleDefs.at(icon);

        QImage iconImage(style.qresource);
        if (!style.showTrueColor)
            tint(iconImage, color, 1.0);
        pixmap = QPixmap::fromImage(iconImage);
    }

    if (!pixmap.isNull())
        m_hashmapIcons[hashIcon] = pixmap;

    return pixmap;
}

uint64_t scs::IconManager::getHash(scs::MarkerIcon icon, Color32 color)
{
    uint64_t hash = 0;
    std::hash<int> hash_int;

    hash = Utils::hash_combine(hash, hash_int((int)icon));
    hash = Utils::hash_combine(hash, hash_int(color.r));
    hash = Utils::hash_combine(hash, hash_int(color.b));
    hash = Utils::hash_combine(hash, hash_int(color.g));

    return hash;
}
