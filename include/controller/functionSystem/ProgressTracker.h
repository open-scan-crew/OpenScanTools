#ifndef CONTEXT_PROGRESS_TRACKER_H_
#define CONTEXT_PROGRESS_TRACKER_H_

#include "controller/Controller.h"
#include "gui/GuiData/GuiDataMessages.h"

#include <algorithm>
#include <functional>

class ProgressTracker
{
public:
    using ProgressCallback = std::function<void(size_t processed, size_t total)>;
    using StateFormatter = std::function<QString(size_t unitsDone, size_t totalUnits, int percent)>;

    ProgressTracker(Controller& controller, size_t totalUnits, StateFormatter stateFormatter)
        : controller_(controller)
        , totalUnits_(totalUnits)
        , stateFormatter_(std::move(stateFormatter))
    {}

    void update(size_t unitsDone, int percent)
    {
        if (!stateFormatter_)
            return;
        percent = std::clamp(percent, 0, 100);
        QString state = stateFormatter_(unitsDone, totalUnits_, percent);
        uint64_t progressValue = static_cast<uint64_t>(unitsDone) * 100;
        if (percent < 100)
            progressValue += static_cast<uint64_t>(percent);
        controller_.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(state, progressValue));
    }

    ProgressCallback makeCallback(size_t unitsDone, int basePercent, int spanPercent)
    {
        return [this, unitsDone, basePercent, spanPercent](size_t processed, size_t total)
        {
            if (total == 0)
                return;
            int percent = basePercent + static_cast<int>((processed * spanPercent) / total);
            percent = std::clamp(percent, basePercent, basePercent + spanPercent - 1);
            if (percent >= 100)
                percent = 99;
            update(unitsDone, percent);
        };
    }

private:
    Controller& controller_;
    size_t totalUnits_;
    StateFormatter stateFormatter_;
};

#endif // CONTEXT_PROGRESS_TRACKER_H_
