#ifndef CONTEXT_STEP_SIMPLIFICATION_H
#define CONTEXT_STEP_SIMPLIFICATION_H

#include "controller/functionSystem/ARayTracingContext.h"
#include "io/imports/step-simplification/step-simplificationTypes.h"
#include "io/FileInputData.h"

class ContextStepSimplification : public ARayTracingContext
{
public:
	ContextStepSimplification(const ContextId& id);
	~ContextStepSimplification();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const;

private:
	StepClassification m_classification;
	double m_keepPercent;
	FileInputData m_data;

	std::filesystem::path m_outputPath;
	bool m_importAfter;
};

#endif
