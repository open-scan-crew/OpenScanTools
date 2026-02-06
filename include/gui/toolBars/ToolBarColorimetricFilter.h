#ifndef TOOLBAR_COLORIMETRIC_FILTER_H
#define TOOLBAR_COLORIMETRIC_FILTER_H

#include "ui_toolbar_colorimetricFilter.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "models/3d/DisplayParameters.h"
#include "utils/safe_ptr.h"

#include <array>
#include <unordered_map>
#include <QSize>

class CameraNode;

class ToolBarColorimetricFilter : public QWidget, public IPanel
{
    Q_OBJECT

public:
    explicit ToolBarColorimetricFilter(IDataDispatcher& dataDispatcher, QWidget* parent = nullptr, float guiScale = 1.f);
    ~ToolBarColorimetricFilter();

private:
    void informData(IGuiData* data) override;
    void onActiveCamera(IGuiData* data);
    void onFocusViewport(IGuiData* data);
    void onRenderColorMode(IGuiData* data);
    void onPickedColor(IGuiData* data);
    void onProjectLoad(IGuiData* data);

    typedef void (ToolBarColorimetricFilter::*GuiDataFunction)(IGuiData*);
    inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
    {
        m_dataDispatcher.registerObserverOnKey(this, type);
        m_methods.insert({ type, fct });
    };

    void updateFromDisplayParameters(const DisplayParameters& params);
    void updateIntensityMode();
    void updatePreview();
    void updateColorField(int index, const glm::vec3& value, bool active);
    int firstAvailableColorIndex() const;
    void applyPickedColor(const Color32& color);
    bool parseColorText(const QString& text, glm::vec3& outColor) const;
    QString formatColorText(const glm::vec3& value) const;
    glm::vec3 rgbToLab(const glm::vec3& rgb) const;
    glm::vec3 labToRgb(const glm::vec3& lab) const;
    void reorderConvexColors(std::array<glm::vec3, 4>& colors, int count) const;

    bool eventFilter(QObject* watched, QEvent* event) override;
    void startPixelPicking();
    void stopPixelPicking();
    void resetFilterState(bool resetTolerance);
    float computeIntensity(const Color32& color) const;
    QColor toPreviewColor(const glm::vec3& value) const;

private slots:
    void slotApply();
    void slotReset();
    void slotPickColor();
    void slotColorSpaceChanged(int index);

private:
    Ui::toolbar_colorimetricFilter m_ui;
    IDataDispatcher& m_dataDispatcher;
    std::unordered_map<guiDType, GuiDataFunction> m_methods;
    SafePtr<CameraNode> m_focusCamera;
    UiRenderMode m_renderMode = UiRenderMode::RGB;
    std::array<glm::vec3, 4> m_colors = { glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f) };
    std::array<bool, 4> m_colorActive = { false, false, false, false };
    bool m_pixelPickActive = false;
    QSize m_previewSize;
};

#endif // TOOLBAR_COLORIMETRIC_FILTER_H
