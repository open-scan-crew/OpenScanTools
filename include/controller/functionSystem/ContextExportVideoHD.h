#ifndef CONTEXT_EXPORT_VIDEO_HD_H_
#define CONTEXT_EXPORT_VIDEO_HD_H_

#include "controller/functionSystem/AContext.h"
#include "io/exports/ExportParameters.hpp"
#include "pointCloudEngine/RenderingTypes.h"
#include <optional>
#include <vector>

class ViewPointNode;

class ContextExportVideoHD : public AContext
{
public:
	ContextExportVideoHD(const ContextId& id);
	~ContextExportVideoHD();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	virtual ContextState launch(Controller& controller) override;
	virtual ContextState abort(Controller& controller) override;
	virtual ContextState validate(Controller& controller) override;

	bool canAutoRelaunch() const;


	std::filesystem::path getNextFramePath();

	virtual ContextType getType() const override;

private:
	bool encodeVideo();
	std::optional<std::filesystem::path> firstFrameFilepath() const;
	void cleanupFrames();

	VideoExportParameters m_parameters;
	std::filesystem::path m_exportPath;
	std::filesystem::path m_videoFilePath;
	int m_exportState = 0;

	long m_totalFrames;
	long m_animFrame = 0;
	uint8_t m_frameDigits = 0;
	std::vector<SafePtr<ViewPointNode>> m_viewpoints;
	std::vector<double> m_viewpointControlTimes;
	double m_orbitalTotalAngleRad = 0.0;
	double m_orbitalLastAppliedRad = 0.0;
	double m_orbitalRealAppliedRad = 0.0;
	double m_orbitalDirectionSign = 1.0;
	bool m_orbitalUsesExamine = false;
	bool m_orbitalVertical = false;

	std::chrono::steady_clock::time_point m_tpStart;
	DecimationOptions m_precedentOptions;

};

#endif // !CONTEXT_POINT_H_
