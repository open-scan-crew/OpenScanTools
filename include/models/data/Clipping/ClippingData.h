#ifndef CLIPPING_DATA_H
#define CLIPPING_DATA_H

#include "models/data/Clipping/ClippingTypes.h"

enum class ElementType;
class ControllerContext;

class ClippingData
{
public:
	ClippingData(const ClippingData& data);
	ClippingData();
	~ClippingData();

	void copyClippingData(const ClippingData& data);

	virtual void setClippingMode(ClippingMode mode);
	void setClippingActive(bool active);
	void setMinClipDist(float minDist);
	void setMaxClipDist(float maxDist);

	void setRampActive(bool active);
	void setRampMin(float min);
	void setRampMax(float max);
	void setRampSteps(int steps);
	void setRampClamped(bool clamped);

	void setDefaultData(const ControllerContext& context);

	ClippingMode getClippingMode() const;
	bool isClippingActive() const;
	float getMinClipDist() const;
	float getMaxClipDist() const;

	bool isRampActive() const;
	float getRampMin() const;
	float getRampMax() const;
	int getRampSteps() const;
	bool isRampClamped() const;

protected:
	ClippingMode m_clippingMode = ClippingMode::showInterior;
	bool m_clippingActive = false;
	float m_minClipDist = 0.f;
	float m_maxClipDist = 1.f;

	bool m_rampActive = false;
	float m_rampMin = 0.f;
	float m_rampMax = 1.f;
	int m_rampSteps = 8; // max = 240
	bool m_rampClamped = false; // only for the box
};

#endif
