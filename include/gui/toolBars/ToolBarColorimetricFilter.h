#ifndef TOOLBAR_COLORIMETRIC_FILTER_H
#define TOOLBAR_COLORIMETRIC_FILTER_H

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "pointCloudEngine/GuiRenderingTypes.h"
#include "ui_toolbar_colorimetricFilter.h"
#include "utils/safe_ptr.h"

#include <QValidator>

#include <array>
#include <unordered_map>

class CameraNode;

class ToolBarColorimetricFilter : public QWidget, public IPanel
{
    Q_OBJECT

public:
    explicit ToolBarColorimetricFilter(IDataDispatcher& dataDispatcher, QWidget* parent = nullptr, float guiScale = 1.f);
    ~ToolBarColorimetricFilter();

private:
    void informData(IGuiData* data) override;
    void onProjectLoad(IGuiData* data);
    void onActiveCamera(IGuiData* data);
    void onFocusViewport(IGuiData* data);
    void onPickValue(IGuiData* data);

    void applyFilter();
    void deactivateFilter();
    void resetFilter();
    void updatePreview();
    void updateFieldState(int index);
    void updateModeState(UiRenderMode mode);
    void applyFieldValue(int index);
    void setFieldPalette(QLineEdit* field, const Color32& color, bool active);
    int findFirstEmptyColor() const;
    bool parseRgb(const QString& value, Color32& outColor) const;
    bool parseIntensity(const QString& value, Color32& outColor) const;
    void startPicking();
    void stopPicking();

    typedef void (ToolBarColorimetricFilter::* GuiDataFunction)(IGuiData*);

    inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
    {
        m_dataDispatcher.registerObserverOnKey(this, type);
        m_methods.insert({ type, fct });
    };

private:
    std::unordered_map<guiDType, GuiDataFunction> m_methods;

    Ui::toolbar_colorimetricFilter m_ui;
    IDataDispatcher& m_dataDispatcher;
    SafePtr<CameraNode> m_focusCamera;
    ColorimetricFilterSettings m_settings;
    UiRenderMode m_currentMode = UiRenderMode::RGB;
    bool m_intensityMode = false;
    bool m_blockUpdates = false;
    bool m_isPicking = false;
    QValidator* m_rgbValidator = nullptr;
    QValidator* m_intensityValidator = nullptr;
};

#endif
