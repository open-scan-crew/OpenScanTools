#pragma once
#ifndef OPENSCANTOOLS_CONTROLLER_MESSAGES_FILTER_EXECUTION_MODE_H
#define OPENSCANTOOLS_CONTROLLER_MESSAGES_FILTER_EXECUTION_MODE_H

// Shared execution-mode enums for filter contexts/messages.
// Keeping them in a dedicated header avoids cross-header coupling
// and ensures types are visible where context headers are parsed.
enum class OutlierFilterExecutionMode
{
    ApplyInProject,
    ExportFilteredAreas
};

enum class ColorBalanceFilterExecutionMode
{
    ApplyInProject,
    ExportFilteredAreas
};

#endif
