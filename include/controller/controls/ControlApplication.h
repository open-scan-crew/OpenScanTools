#ifndef CONTROL_APPLICATION_H
#define CONTROL_APPLICATION_H

#include "controller/controls/IControl.h"
#include "utils/Color32.hpp"
#include "utils/safe_ptr.h"
#include "pointCloudEngine/RenderingTypes.h"
#include "gui/UnitUsage.h"
#include "gui/LanguageType.h"
#include "models/project/ProjectTypes.h"
#include "models/3d/NavigationTypes.h"
#include <filesystem>

class CameraNode;

namespace control::application
{
    class Redo : public AControl
    {
    public:
        Redo();
        ~Redo();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class Undo : public AControl
    {
    public:
        Undo();
        ~Undo();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class Quit : public AControl
    {
    public:
        Quit();
        ~Quit();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class SetLanguage : public AControl
    {
    public:
        SetLanguage(LanguageType type);
        ~SetLanguage();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const LanguageType m_type;
    };

    class SetValueSettingsDisplay : public AControl
    {
    public:
        SetValueSettingsDisplay(const UnitUsage& valueParameters, const bool& save = true);
        ~SetValueSettingsDisplay();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;

    private:
        const UnitUsage m_parameters;
        const bool m_save;
    };

    class SetProjectsFolder : public AControl
    {
    public:
        SetProjectsFolder(const std::filesystem::path& path, const bool& save = true);
        ~SetProjectsFolder();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const std::filesystem::path m_path;
        const bool m_save;
    };

    class SetTemporaryFolder : public AControl
    {
    public:
        SetTemporaryFolder(const std::filesystem::path& path, const bool& save = true);
        ~SetTemporaryFolder();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const std::filesystem::path m_path;
        const bool m_save;
    };

    class SetFFmpegFolder : public AControl
    {
    public:
        SetFFmpegFolder(const std::filesystem::path& path, const bool& save = true);
        ~SetFFmpegFolder();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const std::filesystem::path m_path;
        const bool m_save;
    };

    class SetUserColor : public AControl
    {
    public:
        SetUserColor(const Color32& color, const uint32_t& position = 3, const bool& set = true, const bool& save = true);
        ~SetUserColor();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const Color32 m_color;
        const uint32_t m_position;
        const bool m_save;
        const bool m_set;
    };

    class SetDecimationOptions : public AControl
    {
    public:
        SetDecimationOptions(const DecimationOptions& options, const bool& set = true, const bool& save = true);
        ~SetDecimationOptions();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const DecimationOptions m_options;
        const bool m_save;
        const bool m_set;
    };

	class SetOctreePrecision : public AControl
	{
	public:
		SetOctreePrecision(const OctreePrecision precision, const bool& set = true, const bool& save = true);
		~SetOctreePrecision();
		void doFunction(Controller& controller) override;
		bool canUndo() const override;
		void undoFunction(Controller& controller) override;
		ControlType getType() const override;
	private:
		const OctreePrecision m_precision;
		const bool m_save;
		const bool m_set;
	};

    class SetRenderPointSize : public AControl
    {
    public:
        SetRenderPointSize(const uint32_t& size, SafePtr<CameraNode> camera);
        ~SetRenderPointSize();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const uint32_t m_pointSize;
        SafePtr<CameraNode> m_camera;
    };

    class SetExamineOptions : public AControl
    {
    public:
        SetExamineOptions(bool isCentering, bool keepOnPan, bool save = true);
        ~SetExamineOptions();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const bool m_isCentering;
        const bool m_keepOnPan;
        const bool m_save;
    };

    class SetIndexationMethod : public AControl
    {
    public:
        SetIndexationMethod(const IndexationMethod& indexationMethod , const bool& save = true);
        ~SetIndexationMethod();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const IndexationMethod m_indexationMethod;
        const bool m_save;
    };

    class SetFramelessMode : public AControl
    {
    public:
        SetFramelessMode(const bool& isFrameless, const bool& save = true);
        ~SetFramelessMode();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const bool m_isFrameless;
        const bool m_save;
    };

    class SetExamineDisplayMode : public AControl
    {
    public:
        SetExamineDisplayMode(bool isActive, bool save = true);
        ~SetExamineDisplayMode();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const bool m_isActive;
        const bool m_save;
    };

    class RenderModeUpdate : public AControl
    {
    public:
        RenderModeUpdate(const UiRenderMode& mode, SafePtr<CameraNode> camera);
        ~RenderModeUpdate();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;

    private:
        const UiRenderMode m_mode;
        SafePtr<CameraNode> m_camera;
        bool m_clusterColor;
    };

    class SetRecentProjects : public AControl
    {
    public:
        SetRecentProjects(const std::vector<std::pair<std::filesystem::path, time_t>>& recentProjects, const bool& save = true);
        ~SetRecentProjects();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        std::vector<std::pair<std::filesystem::path, time_t>> m_recentProjects;
        const bool m_save;
    };
        
    class SendRecentProjects : public AControl
    {
    public:
        SendRecentProjects();
        ~SendRecentProjects();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class SetAutoSaveParameters : public AControl
    {
    public:
        SetAutoSaveParameters(const bool& activate, const uint8_t& timing, const bool& save);
        ~SetAutoSaveParameters();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const bool m_save;
        const bool m_isActivate;
        const uint8_t m_timing;
    };

    class SetGizmoParameters : public AControl
    {
    public:
        SetGizmoParameters(const bool& activate, const glm::vec3& gizmo, const bool& save);
        ~SetGizmoParameters();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const bool m_save;
        const bool m_isActivate;
        const glm::vec3 m_gizmo;
    };

    class SetManipulatorSize : public AControl
    {
    public:
        SetManipulatorSize(float sizeFactor, bool save);
        ~SetManipulatorSize();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const bool m_save;
        const double m_sizeFactor;
    };

    class LoadManipulators : public AControl
    {
    public:
        LoadManipulators();
        ~LoadManipulators();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class SetNavigationParameters : public AControl
    {
    public:
        SetNavigationParameters(const NavigationParameters& navParam, const bool& save = true);
        ~SetNavigationParameters();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const NavigationParameters m_navParam;
        const bool m_save;
    };

    class SetPerspectiveZBounds : public AControl
    {
    public:
        SetPerspectiveZBounds(const PerspectiveZBounds& zBounds, const bool& save = true);
        ~SetPerspectiveZBounds();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const PerspectiveZBounds m_zBounds;
        const bool m_save;
    };

    class SetOrthographicZBounds : public AControl
    {
    public:
        SetOrthographicZBounds(const OrthographicZBounds& zBounds, const bool& save = true);
        ~SetOrthographicZBounds();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const OrthographicZBounds m_zBounds;
        const bool m_save;
    };

    class SetOrthoGridParameters : public AControl
    {
    public:
        SetOrthoGridParameters(bool active, const Color32& color, float step, uint32_t linewidth);
        ~SetOrthoGridParameters();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        bool m_active;
        Color32 m_color;
        float m_step;
        uint32_t m_linewidth;
    };

    class UnlockScanManipulation : public AControl
    {
    public:
        UnlockScanManipulation(bool unlock, bool save = true);
        ~UnlockScanManipulation();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        bool m_unlock;
        bool m_save;
    };

    class SetMultithreadedCalculation : public AControl
    {
    public:
        SetMultithreadedCalculation(bool enabled, bool save = true);
        ~SetMultithreadedCalculation();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        bool m_enabled;
        bool m_save;
    };

    class SwitchScantraConnexion : public AControl
    {
    public:
        SwitchScantraConnexion(bool start);
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        bool start_;
    };
}

#endif // !CONTROLAPPLICATION_H_
