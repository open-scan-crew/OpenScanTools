#ifndef CONTEXT_EXPORT_VIDEO_HD_H_
#define CONTEXT_EXPORT_VIDEO_HD_H_

#include "controller/functionSystem/AContext.h"
#include "controller/messages/VideoExportParametersMessage.h"
#include "pointCloudEngine/RenderingTypes.h"

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
	VideoExportParameters m_parameters;
	std::filesystem::path m_exportPath;
	int m_exportState = 0;

	long m_totalFrames;
	long m_animFrame = 0;

	glm::dvec3 m_addPosition = glm::dvec3(0.);
	double m_addTheta = 0.;
	double m_addPhi = 0.;

	std::chrono::steady_clock::time_point m_tpStart;
	DecimationOptions m_precedentOptions;

};

#endif // !CONTEXT_POINT_H_
