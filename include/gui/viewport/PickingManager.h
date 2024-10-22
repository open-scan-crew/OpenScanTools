#ifndef PICKING_MANAGER_H
#define PICKING_MANAGER_H

#include <vector>
#include <unordered_set>
#include "glm/glm.hpp"
#include "vulkan/TlFramebuffer.h"
#include "models/OpenScanToolsModelEssentials.h"

class PickingManager
{
public:
	PickingManager();
	~PickingManager();
	void resetPickingStored();
	uint32_t getPicking(uint32_t mouseX, uint32_t mouseY, TlFramebuffer framebuffer);
	uint32_t getQuickPicking(uint32_t mouseX, uint32_t mouseY, TlFramebuffer framebuffer);
	uint32_t getLastPicking() const;

	static std::unordered_set<uint32_t> getObjects(TlFramebuffer framebuffer, const Rect2D& rect, uint8_t sampling = 3);

	void setPickingRange(uint8_t range);
	uint8_t getPickingRange() const;

protected:
	uint8_t					m_selectionRange;
	std::vector<uint32_t>	m_selectedIds;
	uint32_t				m_iterator;
	glm::ivec2				m_lastMousePosition;
};

#endif // !PICKING_MANAGER_H_