#ifndef VIEWPORT_ORGANIZER_H
#define VIEWPORT_ORGANIZER_H

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include "utils/safe_ptr.h"

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/QShortcut.h>
#include <glm/glm.hpp>

class IDataDispatcher;
class IRenderingEngine;
class IGuiData;
class VulkanViewport;
class QuickBarNavigation;
class ShortcutSystem;
class CameraNode;

enum class AlignView;
enum class ProjectionMode;

//Note (Aurélien) testing mouse 3d
#include "utils/3DConnexion/3DConnexion.h"

struct MetaViewport
{
    QWidget* mainWidget = nullptr;
    VulkanViewport* vulkanViewport = nullptr;
    QuickBarNavigation* quickBar = nullptr;
};

class ViewportOrganizer : public QWidget, public IPanel
{
    Q_OBJECT
public:
    ViewportOrganizer(QWidget* parent, IDataDispatcher& dataDispatcher, IRenderingEngine& engine, ShortcutSystem& shortSys, float guiScale);
    ~ViewportOrganizer();

    void addViewport(int row, int column, int rowSpan, int columnSpan);

    void mouse3DUpdate(const glm::dmat4& status) const;
    glm::dmat4 getCurrentViewMatrix() const;
    SafePtr<CameraNode> getActiveCameraNode();

public slots:
    void onActiveViewport(const VulkanViewport* viewport);
    void onInitializedCamera(CameraNode* camera, const VulkanViewport* viewport);
    void onUpdateCameraPosition(double x, double y, double z, const VulkanViewport* viewport);
    void onUpdatePickingPosition(double x, double y, double z,  const VulkanViewport* viewport);
    void onToggleFullScreen();
    void onEnableFullScreen();
    void onDisableFullScreen();

    void onCancel();

signals:
    void cameraPos(double x, double y, double z);
    void picking(double x, double y, double z);

private:
    typedef void (ViewportOrganizer::* ViewportOrganiserFunction)();

    QShortcut* createShortcut(QKeySequence key, QWidget* parent, Qt::ShortcutContext context);
    void addShortcut(QKeySequence key, QWidget* parent, Qt::ShortcutContext context, std::function<void()> action);
    void addShortcut(QKeySequence key, QWidget* parent, Qt::ShortcutContext context, ViewportOrganiserFunction action);

    void informData(IGuiData* guiData) override;
    typedef void (ViewportOrganizer::* GuiDataFunction)(IGuiData*);
    inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
    {
        m_dataDispatcher.registerObserverOnKey(this, type);
        m_functions.insert({ type, fct });
    };

    void onContextRequestActiveCamera(IGuiData* data);
    void onCursorChange(IGuiData* data);

    void onAdjustZoomToScene();
    void onAlignView(AlignView align);
    void onRotation90();
    void onProjectionMode(ProjectionMode proj);
    void onMousePointAction(bool examineAction);

private:
    IDataDispatcher& m_dataDispatcher;
    ShortcutSystem& m_shortSys;
    std::unordered_map<guiDType, GuiDataFunction> m_functions;
    IRenderingEngine& m_renderingEngine;
    float m_guiScale;

    QGridLayout* m_layout;
    SafePtr<CameraNode> m_activeViewport;
    SafePtr<CameraNode> m_fullScreenViewport;
    std::unordered_map<SafePtr<CameraNode>, MetaViewport> m_viewports;

    // Store the previous position of the fullscreen viewport in the layout
    int row, col, rowSpan, colSpan;

    Mouse_3DConnexion m_mouse;
};

#endif