#ifndef DIALOG_ANIMATION_CONFIG_H
#define DIALOG_ANIMATION_CONFIG_H

#include "models/application/ViewPointAnimation.h"
#include "gui/IDataDispatcher.h"
#include "ui_DialogAnimationConfig.h"

#include <QtWidgets/qdialog.h>

class DialogAnimationConfig : public QDialog
{
    Q_OBJECT

public:
    DialogAnimationConfig(IDataDispatcher& dataDispatcher, QWidget* parent = nullptr);
    ~DialogAnimationConfig();

    void setKnownAnimations(const std::vector<ViewPointAnimationConfig>& animations);
    void setAvailableViewpoints(const std::vector<AnimationViewpointInfo>& viewpoints);
    void setupForNew();
    void setupForEdit(const ViewPointAnimationConfig& config);

private slots:
    void onAddViewpoint();
    void onMoveUp();
    void onMoveDown();
    void onDeleteViewpoint();
    void onCleanList();

    void onOk();
    void onUpdate();
    void onDelete();
    void onCancel();

private:
    void configureTable();
    void refreshLineColumn();
    void setCurrentConfigToUi(const ViewPointAnimationConfig& config);
    std::vector<ViewPointAnimationLine> readLinesFromUi(bool* ok = nullptr) const;
    ViewPointAnimationConfig buildConfigFromUi(bool* ok = nullptr) const;
    bool validateBeforeSave(bool creationMode) const;
    int selectedRow() const;
    void insertRowAt(int row);
    void showSplashMessage(const QString& message) const;
    AnimationViewpointInfo findViewpointInfo(const xg::Guid& id) const;
    void validatePositionRow(int row) const;
    void checkRenderingConsistency(const xg::Guid& newViewpointId) const;

private:
    IDataDispatcher& m_dataDispatcher;
    Ui::DialogAnimationConfig m_ui;

    std::vector<ViewPointAnimationConfig> m_knownAnimations;
    std::vector<AnimationViewpointInfo> m_availableViewpoints;
    viewPointAnimationId m_editId;
    bool m_isEdition;
};

#endif // DIALOG_ANIMATION_CONFIG_H
