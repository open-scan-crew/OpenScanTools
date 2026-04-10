# OpenScanTools – Persistence Matrix (Single Source of Truth)

Ce document formalise la **source de vérité fonctionnelle** pour la persistance des réglages projet/viewpoint.

## Légende
- **Scope**
  - `ProjectOnly` : sauvegardé uniquement au niveau projet.
  - `SharedProjectViewpoint` : sauvegardé dans projet **et** viewpoint.
  - `RuntimeOnly` : non persisté.
- **Status**
  - `Implemented` : déjà implémenté dans le code.
  - `Planned` : ciblé pour une passe suivante.

---

## Matrice

| Field ID | Scope | Status | Project key | Viewpoint key | Notes |
|---|---|---|---|---|---|
| `userOrientation.mode` | SharedProjectViewpoint | Planned | _TBD_ | _TBD_ | Radio `user/project` |
| `userOrientation.selectedId` | SharedProjectViewpoint | Planned | _TBD_ | _TBD_ | Combo user orientation |
| `orthoGrid.active` | SharedProjectViewpoint | Implemented | `OrthoGridActive` | `OrthoGridActive` | |
| `orthoGrid.color` | SharedProjectViewpoint | Implemented | `OrthoGridColor` | `OrthoGridColor` | |
| `orthoGrid.step` | SharedProjectViewpoint | Implemented | `OrthoGridStep` | `OrthoGridStep` | |
| `orthoGrid.lineWidth` | SharedProjectViewpoint | Implemented | `OrthoGridLinewidth` | `OrthoGridLinewidth` | Bug de clé corrigé en passe 1 |
| `imagegroup.useFrame` | SharedProjectViewpoint | Planned | _TBD_ | _TBD_ | |
| `imagegroup.ratioMode` | SharedProjectViewpoint | Planned | _TBD_ | _TBD_ | ratio image vs ratio print |
| `imagegroup.ratioChoice` | SharedProjectViewpoint | Planned | _TBD_ | _TBD_ | combo ratio |
| `imagegroup.orientation` | SharedProjectViewpoint | Planned | _TBD_ | _TBD_ | portrait/landscape |
| `imagegroup.grid` | SharedProjectViewpoint | Planned | _TBD_ | _TBD_ | |
| `imagegroup.width` | SharedProjectViewpoint | Planned | _TBD_ | _TBD_ | |
| `imagegroup.height` | SharedProjectViewpoint | Planned | _TBD_ | _TBD_ | |
| `imagegroup.alpha` | SharedProjectViewpoint | Planned | _TBD_ | _TBD_ | |
| `imagegroup.format` | SharedProjectViewpoint | Planned | _TBD_ | _TBD_ | |
| `imagegroup.antialiasing` | SharedProjectViewpoint | Planned | _TBD_ | _TBD_ | |
| `imagegroup.scaleOrDpi` | SharedProjectViewpoint | Planned | _TBD_ | _TBD_ | mode selon projection |
| `clipping.defaultMode` | ProjectOnly | Implemented | `DefaultClipMode` | — | |
| `clipping.defaultDistances` | ProjectOnly | Implemented | `DefaultClipDistances` | — | |
| `clipping.defaultLengthThreshold` | ProjectOnly | Implemented | `DefaultLengthThresholdClip` | — | |
| `animation.length` | ProjectOnly | Planned | _TBD_ | — | `lengthSpinBox` |
| `ramp.defaultDistances` | ProjectOnly | Implemented | `DefaultRampDistances` | — | |
| `ramp.defaultSteps` | ProjectOnly | Implemented | `DefaultRampSteps` | — | |

---

## Convention opérationnelle

1. Toute évolution de scope/clé doit être modifiée dans :
   - ce document
   - `include/io/PersistenceSchema.h`
2. Toute nouvelle persistance doit indiquer explicitement :
   - fallback legacy (si nécessaire)
   - clé projet et clé viewpoint (ou absence explicite)
3. Les sérialisations du même concept doivent passer par un helper partagé pour éviter les dérives de clé.

