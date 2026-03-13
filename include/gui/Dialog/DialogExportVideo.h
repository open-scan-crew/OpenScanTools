#ifndef DIALOG_EXPORT_VIDEO_H
#define DIALOG_EXPORT_VIDEO_H

#include "ui_DialogExportVideo.h"
#include "gui/Dialog/ADialog.h"
#include "io/exports/ExportParameters.hpp"
#include "models/application/ViewPointAnimation.h"
#include <optional>
#include <utility>

class DialogExportVideo : public ADialog
{
    Q_OBJECT

public:
    DialogExportVideo(IDataDispatcher& dataDispatcher, QWidget *parent, float guiScale);
    ~DialogExportVideo();


    void informData(IGuiData *data) override;
    void closeEvent(QCloseEvent* event);

private:
    void onSelectOutFolder();
	void onSelectOutFile();

    void onOutputTypeChanged();
    bool checkResolutionForMp4() const;
    bool isX265Available() const;
    std::optional<std::pair<uint32_t, uint32_t>> currentImageResolution() const;

    void startGeneration();
    void cancelGeneration();

public:
    void setAnimationMode(VideoAnimationMode mode);
    void setLength(int length);
    void setOrbitalDegrees(int degrees);
    void setSelectedAnimation(const viewPointAnimationId& animationId, bool hasSelection);
    void setInterpolateRenderings(bool interpolate);

private:
    Ui::DialogExportVideo m_ui;
    QString m_openPath;

	VideoExportParameters m_parameters;
	VideoAnimationMode m_animationMode = VideoAnimationMode::BETWEENVIEWPOINTS;
	int m_length = 30;
	int m_orbitalDegrees = 360;
	viewPointAnimationId m_selectedAnimationId;
	bool m_hasSelectedAnimation = false;
	bool m_interpolateRenderings = false;

    static constexpr uint64_t MAX_MP4_PIXELS = 8294400;
};

#endif
