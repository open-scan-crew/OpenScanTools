#ifndef CONTEXT_POINT_CLOUD_OBJECT_CREATION_H_
#define CONTEXT_POINT_CLOUD_OBJECT_CREATION_H_

#include "controller/functionSystem/AContext.h"
#include "io/exports/ExportParameters.hpp"
#include "models/data/Clipping/ClippingGeometry.h"
#include "models/graph/TransformationModule.h"

class ContextPCOCreation: public AContext
{
public:
	ContextPCOCreation(const ContextId& id);
	~ContextPCOCreation();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller);
	ContextState launch(Controller& controller);
	bool canAutoRelaunch() const;

	virtual ContextType getType() const override ;

private:
	PointCloudObjectParameters m_parameters;
	TransformationModule point_cloud_transfo_;
	ClippingAssembly clipping_assembly_;
	tls::ScanGuid m_panoramic;
};

#endif // !CONTEXT_POINT_CLOUD_OBJECT_CREATION_H_
