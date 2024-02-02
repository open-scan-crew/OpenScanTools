#include "gui/viewport/PickingManager.h"
#include "vulkan/VulkanManager.h"
#include "models/OpenScanToolsModelEssentials.h"

PickingManager::PickingManager()
    : m_selectionRange(2)
    , m_iterator(0)
    , m_lastMousePosition(-1, -1)
{}

PickingManager::~PickingManager()
{}

void PickingManager::resetPickingStored()
{
	m_selectedIds.clear();
	m_lastMousePosition = glm::ivec2(-m_selectionRange, -m_selectionRange);
}

uint32_t PickingManager::getPicking(uint32_t mouseX, uint32_t mouseY, TlFramebuffer framebuffer)
{
	glm::ivec2 mouse(mouseX, mouseY);
	if ((m_lastMousePosition.x - mouseX) < m_selectionRange ||
		(m_lastMousePosition.y - mouseY) < m_selectionRange)
	{
		if (m_selectedIds.empty())
			return INVALID_PICKING_ID;
		m_iterator = (m_iterator + 1) < m_selectedIds.size() ? m_iterator + 1 : 0;
		return m_selectedIds[m_iterator];
	}
	else // refresh picking set
	{
		m_iterator = 0;
		m_lastMousePosition = mouse;
		if (!m_selectionRange)
		{
			m_selectedIds.clear();
			uint32_t value(VulkanManager::getInstance().sampleIndex(framebuffer, mouseX, mouseY));
			if (value)
				m_selectedIds.push_back(value);
		}
		else
			m_selectedIds = VulkanManager::getInstance().sampleIndexList(framebuffer, mouseX, mouseY, m_selectionRange);
		return m_selectedIds.empty() ? INVALID_PICKING_ID : m_selectedIds[m_iterator];
	}
}

uint32_t PickingManager::getQuickPicking(uint32_t mouseX, uint32_t mouseY, TlFramebuffer framebuffer)
{
	uint32_t value(VulkanManager::getInstance().sampleIndex(framebuffer, mouseX, mouseY));
	return value ? value : INVALID_PICKING_ID;
}

std::unordered_set<uint32_t> PickingManager::getObjects(TlFramebuffer framebuffer, const Rect2D& rect, uint8_t sampling)
{
	std::unordered_set<uint32_t> results;
	// Discard selection outside the positive quadrant before rounding to 0
	if ((rect.c0.x < 0 && rect.c1.x < 0) || (rect.c0.y < 0 && rect.c1.y < 0))
		return results;

	// Round up to 0 if coordinates are negatives
	uint32_t x0(std::max(rect.c0.x, 0));
	uint32_t x1(std::max(rect.c1.x, 0));
	uint32_t y0(std::max(rect.c0.y, 0));
	uint32_t y1(std::max(rect.c1.y, 0));
	uint32_t xMin(std::min(x0, x1));
	uint32_t xMax(std::max(x0, x1));
	uint32_t yMin(std::min(y0, y1));
	uint32_t yMax(std::max(y0, y1));
	for (uint32_t mouseX(xMin); mouseX <= xMax; mouseX += sampling)
	{
		for (uint32_t mouseY(yMin); mouseY <= yMax; mouseY += sampling)
		{
			uint32_t value(VulkanManager::getInstance().sampleIndex(framebuffer, mouseX, mouseY));
			if (value != INVALID_PICKING_ID)
				results.insert(value);
		}
		uint32_t value(VulkanManager::getInstance().sampleIndex(framebuffer, mouseX, yMax));
		if (value != INVALID_PICKING_ID)
			results.insert(value);
	}
	for (uint32_t mouseY(yMin); mouseY <= yMax; mouseY += sampling)
	{
		uint32_t value(VulkanManager::getInstance().sampleIndex(framebuffer, xMax, mouseY));
		if (value != INVALID_PICKING_ID)
			results.insert(value);
	}
	uint32_t value(VulkanManager::getInstance().sampleIndex(framebuffer, xMax, yMax));
	if (value != INVALID_PICKING_ID)
		results.insert(value);
	return results;
}

uint32_t PickingManager::getLastPicking() const
{
	if (m_iterator > m_selectedIds.size() || m_selectedIds.empty())
		return INVALID_PICKING_ID;
	return m_selectedIds[m_iterator];
}

void PickingManager::setPickingRange(uint8_t range)
{
	m_selectionRange = range;
}

uint8_t PickingManager::getPickingRange() const
{
	return m_selectionRange;
}