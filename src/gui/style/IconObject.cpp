#include "gui/style/IconObject.h"
#include "services/MarkerSystem.h"
#include <QtGui/qimage.h>

#include "utils/Utils.h"

void tint(QImage& image, Color32 color)
{
    for (int x = 0; x < image.width(); x++)
    {
        for (int y = 0; y < image.height(); y++)
        {
            QPoint pt(x, y);
            int a = image.pixelColor(pt).alpha();
            QColor qcolor = QColor(color.r, color.g, color.b, a);
            image.setPixelColor(pt, qcolor);
        }
    }
}

QPixmap scs::IconManager::getIcon(scs::MarkerIcon icon, Color32 color)
{
    uint64_t hashIcon = getHash(icon, color);
    if (m_hashmapIcons.find(hashIcon) != m_hashmapIcons.end())
        return m_hashmapIcons.at(hashIcon);

    QPixmap pixmap = QPixmap();
    if (icon == scs::MarkerIcon::Max_Enum)
        return pixmap;

    MarkerSystem::Style style = MarkerSystem::getStyle(icon);

    QImage iconImage(style.qresource);
    if (!style.showTrueColor)
        tint(iconImage, color);
    pixmap = QPixmap::fromImage(iconImage);

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
