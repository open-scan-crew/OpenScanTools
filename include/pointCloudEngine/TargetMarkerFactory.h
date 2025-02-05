#ifndef TARGET_MARKER_FACTORY_H_
#define TARGET_MARKER_FACTORY_H_

#include "models/3d/MarkerDrawData.h"
#include "utils/Color32.hpp"

#include <vector>

class TargetMarkerFactory
{
public:
	TargetMarkerFactory();
	~TargetMarkerFactory();

	void createClickTarget(const glm::dvec3& position, const Color32& color);

	void freeClickMarkers();

	std::vector<MarkerDrawData> generateMarkersList() const;

protected:
	MarkerDrawData generateTargetMarkerData(const glm::dvec3& position, const Color32& color) const;

protected:
	std::vector<MarkerDrawData> m_clicks;
};

#endif // !TARGET_MARKER_FACTORY_H_

