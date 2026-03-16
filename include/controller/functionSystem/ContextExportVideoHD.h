#ifndef CONTEXT_EXPORT_VIDEO_HD_H_
#define CONTEXT_EXPORT_VIDEO_HD_H_

#include "controller/functionSystem/AContext.h"
#include "io/exports/ExportParameters.hpp"
#include "pointCloudEngine/RenderingTypes.h"
#include <optional>

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

	long m_totalFrames = 0;
	long m_animFrame = 0;
	uint8_t m_frameDigits = 0;
	double m_orbitalStepRad = 0.0;

	std::chrono::steady_clock::time_point m_tpStart;
	DecimationOptions m_precedentOptions;

};

#endif // !CONTEXT_POINT_H_
