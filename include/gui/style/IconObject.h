#ifndef ICON_OBJECT_H_
#define ICON_OBJECT_H_

#include "models/data/Marker.h"
#include "utils/Color32.hpp"
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

		QPixmap getIcon(scs::MarkerIcon icon, Color32 color);

	private:
		uint64_t getHash(scs::MarkerIcon icon, Color32 color);
		std::unordered_map<uint64_t, QPixmap> m_hashmapIcons;
	};
}
#endif