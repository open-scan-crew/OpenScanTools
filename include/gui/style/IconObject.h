#ifndef ICON_OBJECT_H_
#define ICON_OBJECT_H_

#include "models/Types.hpp"
#include "models/project/Marker.h"
#include <QtGui/qpixmap.h>

class QColor;
class QPixmap;
class QImage;


namespace scs
{
	class IconManager
	{
	public:
		static IconManager& getInstance()
		{
			static IconManager instance;
			return instance;
		}

		QPixmap getIcon(ElementType type, scs::MarkerIcon icon, QColor color);

	private:
		uint64_t getHash(ElementType type, scs::MarkerIcon icon, QColor color);
		std::unordered_map<uint64_t, QPixmap> m_hashmapIcons;
	};
}
#endif