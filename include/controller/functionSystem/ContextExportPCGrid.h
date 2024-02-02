#ifndef CONTEXT_EXPORT_PC_GRID_H
#define CONTEXT_EXPORT_PC_GRID_H

#include "controller/functionSystem/ProcessingContext.h"
#include "io/exports/ExportParameters.hpp"

enum FileType;
enum PrecisionType;

class ContextExportPCGrid : public ProcessingContext
{
public:
    ContextExportPCGrid(const ContextId& id);
    ~ContextExportPCGrid();
    ContextState start(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState validate(Controller& controller) override;
    bool isBackground() const;
    bool canAutoRelaunch() const;
    void stop() override;
    void kill() override;

    ContextType getType() const override;

    AContext *clone() override;

private:
    ContextState process(Controller& controller);

private:
    ClippingExportParameters m_parameters;
};

#endif // !CONTEXT_EXPORT_PC_GRID_H_