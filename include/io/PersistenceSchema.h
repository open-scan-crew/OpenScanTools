#ifndef PERSISTENCE_SCHEMA_H
#define PERSISTENCE_SCHEMA_H

#include "io/SerializerKeys.h"
#include "models/3d/DisplayParameters.h"

#include <nlohmannJson/json.hpp>

#include <vector>

namespace persistence
{
	enum class Scope
	{
		ProjectOnly,
		ViewpointOnly,
		SharedProjectViewpoint,
		RuntimeOnly
	};

	enum class Status
	{
		Implemented,
		Planned
	};

	struct Field
	{
		const char* id;
		Scope scope;
		Status status;
		const char* projectKey;
		const char* viewpointKey;
	};

	// Single source of truth for persistence intent.
	// This table is used as the canonical functional mapping and
	// must be updated whenever persistence scope changes.
	inline const std::vector<Field>& getFields()
	{
		static const std::vector<Field> fields =
		{
			// --- 2.a Shared project + viewpoint ---
			{ "userOrientation.mode", Scope::SharedProjectViewpoint, Status::Implemented, Key_Viewpoint_User_Orientation_Enabled, Key_Viewpoint_User_Orientation_Enabled },
			{ "userOrientation.selectedId", Scope::SharedProjectViewpoint, Status::Implemented, Key_Viewpoint_User_Orientation_Id, Key_Viewpoint_User_Orientation_Id },
			{ "orthoGrid.active", Scope::SharedProjectViewpoint, Status::Implemented, Key_Ortho_Grid_Active, Key_Ortho_Grid_Active },
			{ "orthoGrid.step", Scope::SharedProjectViewpoint, Status::Implemented, Key_Ortho_Grid_Step, Key_Ortho_Grid_Step },
			{ "orthoGrid.lineWidth", Scope::SharedProjectViewpoint, Status::Implemented, Key_Ortho_Grid_Linewidth, Key_Ortho_Grid_Linewidth },
			{ "orthoGrid.color", Scope::SharedProjectViewpoint, Status::Implemented, Key_Ortho_Grid_Color, Key_Ortho_Grid_Color },
			{ "imagegroup.useFrame", Scope::SharedProjectViewpoint, Status::Implemented, Key_Image_Group_Settings, Key_Image_Group_Settings },
			{ "imagegroup.ratioMode", Scope::SharedProjectViewpoint, Status::Implemented, Key_Image_Group_Settings, Key_Image_Group_Settings },
			{ "imagegroup.ratioChoice", Scope::SharedProjectViewpoint, Status::Implemented, Key_Image_Group_Settings, Key_Image_Group_Settings },
			{ "imagegroup.orientation", Scope::SharedProjectViewpoint, Status::Implemented, Key_Image_Group_Settings, Key_Image_Group_Settings },
			{ "imagegroup.grid", Scope::SharedProjectViewpoint, Status::Implemented, Key_Image_Group_Settings, Key_Image_Group_Settings },
			{ "imagegroup.width", Scope::SharedProjectViewpoint, Status::Implemented, Key_Image_Group_Settings, Key_Image_Group_Settings },
			{ "imagegroup.height", Scope::SharedProjectViewpoint, Status::Implemented, Key_Image_Group_Settings, Key_Image_Group_Settings },
			{ "imagegroup.alpha", Scope::SharedProjectViewpoint, Status::Implemented, Key_Image_Group_Settings, Key_Image_Group_Settings },
			{ "imagegroup.format", Scope::SharedProjectViewpoint, Status::Implemented, Key_Image_Group_Settings, Key_Image_Group_Settings },
			{ "imagegroup.antialiasing", Scope::SharedProjectViewpoint, Status::Implemented, Key_Image_Group_Settings, Key_Image_Group_Settings },
			{ "imagegroup.scaleOrDpi", Scope::SharedProjectViewpoint, Status::Implemented, Key_Image_Group_Settings, Key_Image_Group_Settings },

			// --- 2.b Project only ---
			{ "clipping.defaultMode", Scope::ProjectOnly, Status::Implemented, Key_DefaultClipMode, "" },
			{ "clipping.defaultDistances", Scope::ProjectOnly, Status::Implemented, Key_DefaultClipDistances, "" },
			{ "clipping.defaultLengthThreshold", Scope::ProjectOnly, Status::Implemented, Key_DefaultLengthThresholdClip, "" },
			{ "animation.length", Scope::ProjectOnly, Status::Planned, "", "" },
			{ "ramp.defaultDistances", Scope::ProjectOnly, Status::Implemented, Key_DefaultRampDistances, "" },
			{ "ramp.defaultSteps", Scope::ProjectOnly, Status::Implemented, Key_DefaultRampSteps, "" }
		};
		return fields;
	}

	// Centralized writer for orthographic grid fields.
	// This avoids key drift between serializer implementations.
	inline void writeOrthoGrid(nlohmann::json& json, const DisplayParameters& params)
	{
		json[Key_Ortho_Grid_Active] = params.m_orthoGridActive;
		json[Key_Ortho_Grid_Color] = { params.m_orthoGridColor.r, params.m_orthoGridColor.g, params.m_orthoGridColor.b, params.m_orthoGridColor.a };
		json[Key_Ortho_Grid_Step] = params.m_orthoGridStep;
		json[Key_Ortho_Grid_Linewidth] = params.m_orthoGridLineWidth;
	}
}

#endif // PERSISTENCE_SCHEMA_H
