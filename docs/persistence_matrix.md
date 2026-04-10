# OpenScanTools – Persistence Matrix (Single Source of Truth)

This document defines the **functional source of truth** for project/viewpoint persistence settings.

## Legend
- **Scope**
  - `ProjectOnly`: saved only at project level.
  - `SharedProjectViewpoint`: saved in both project **and** viewpoint.
  - `RuntimeOnly`: not persisted.
- **Status**
  - `Implemented`: already implemented in code.
  - `Planned`: targeted for a future pass.

---

## Matrix

| Field ID | Scope | Status | Project key | Viewpoint key | Notes |
|---|---|---|---|---|---|
| `userOrientation.mode` | SharedProjectViewpoint | Implemented | `ViewpointUserOrientationEnabled` | `ViewpointUserOrientationEnabled` | User/project radio mode |
| `userOrientation.selectedId` | SharedProjectViewpoint | Implemented | `ViewpointUserOrientationId` | `ViewpointUserOrientationId` | User orientation combo |
| `orthoGrid.active` | SharedProjectViewpoint | Implemented | `OrthoGridActive` | `OrthoGridActive` | |
| `orthoGrid.color` | SharedProjectViewpoint | Implemented | `OrthoGridColor` | `OrthoGridColor` | |
| `orthoGrid.step` | SharedProjectViewpoint | Implemented | `OrthoGridStep` | `OrthoGridStep` | |
| `orthoGrid.lineWidth` | SharedProjectViewpoint | Implemented | `OrthoGridLinewidth` | `OrthoGridLinewidth` | Key typo fixed in pass 1 |
| `imagegroup.useFrame` | SharedProjectViewpoint | Implemented | `ImageGroupSettings` | `ImageGroupSettings` | |
| `imagegroup.ratioMode` | SharedProjectViewpoint | Implemented | `ImageGroupSettings` | `ImageGroupSettings` | image ratio vs print ratio |
| `imagegroup.ratioChoice` | SharedProjectViewpoint | Implemented | `ImageGroupSettings` | `ImageGroupSettings` | ratio combo |
| `imagegroup.orientation` | SharedProjectViewpoint | Implemented | `ImageGroupSettings` | `ImageGroupSettings` | portrait/landscape |
| `imagegroup.grid` | SharedProjectViewpoint | Implemented | `ImageGroupSettings` | `ImageGroupSettings` | |
| `imagegroup.width` | SharedProjectViewpoint | Implemented | `ImageGroupSettings` | `ImageGroupSettings` | |
| `imagegroup.height` | SharedProjectViewpoint | Implemented | `ImageGroupSettings` | `ImageGroupSettings` | |
| `imagegroup.alpha` | SharedProjectViewpoint | Implemented | `ImageGroupSettings` | `ImageGroupSettings` | |
| `imagegroup.format` | SharedProjectViewpoint | Implemented | `ImageGroupSettings` | `ImageGroupSettings` | |
| `imagegroup.antialiasing` | SharedProjectViewpoint | Implemented | `ImageGroupSettings` | `ImageGroupSettings` | |
| `imagegroup.scaleOrDpi` | SharedProjectViewpoint | Implemented | `ImageGroupSettings` | `ImageGroupSettings` | mode depends on projection |
| `clipping.defaultMode` | ProjectOnly | Implemented | `DefaultClipMode` | — | |
| `clipping.defaultDistances` | ProjectOnly | Implemented | `DefaultClipDistances` | — | |
| `clipping.defaultLengthThreshold` | ProjectOnly | Implemented | `DefaultLengthThresholdClip` | — | |
| `animation.length` | ProjectOnly | Planned | _TBD_ | — | `lengthSpinBox` |
| `ramp.defaultDistances` | ProjectOnly | Implemented | `DefaultRampDistances` | — | |
| `ramp.defaultSteps` | ProjectOnly | Implemented | `DefaultRampSteps` | — | |

---

## Operational convention

1. Any scope/key change must be updated in:
   - this document
   - `include/io/PersistenceSchema.h`
2. Any new persistence field must explicitly define:
   - legacy fallback behavior (if needed)
   - project key and viewpoint key (or explicit absence)
3. Serializing the same concept in multiple places must use a shared helper to prevent key drift.
