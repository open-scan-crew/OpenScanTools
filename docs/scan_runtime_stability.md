# Technical Note — Scan Runtime Stability (Large Load/Import Workloads)

## 1) Initial context

The original issue affected projects with very large numbers of scans, with a recurring threshold around 507/508 scans:
- TLS open failures,
- invalid or null UUIDs,
- missing scans,
- unstable behavior during massive imports.

Historically, the product was expected to handle very large projects (including >1000 scans), so the fix had to be systemic, not local.

---

## 2) Global resolution strategy

The remediation was intentionally incremental to minimize regressions:

1. **Initial 3-pass plan**
   - Pass 1: instrumentation/measurement,
   - Pass 2: main technical correction,
   - Pass 3: final hardening/validation.

2. **Pass 2 split**
   - 2.1: GUID lookup vs runtime activation separation,
   - 2.2: deferred activation + stabilization,
   - 2.3: finishing (logs/documentation).

3. **Pass 2.2 split**
   - 2.2.A: GUID->path registry,
   - 2.2.B: lazy runtime activation,
   - 2.2.C: save/persistence robustness.

4. **2.2.C split into blocks**
   - Block A = 2.2.C (save/path fallback),
   - Block B = massive import (drag&drop/import button).

5. **Block B split**
   - B2.1: immediate file-capacity hotfix,
   - B2.2: structural budget/eviction policy,
   - B2.3: logging clarification + finalization.

---

## 3) Work performed

### 3.1 Pass 1 — Instrumentation

Goal: prove the root cause under real workload.

Added runtime metrics around GUID resolution:
- total calls,
- success/failure counts,
- cache hits,
- active insertions,
- active peak.

Outcome: confirmed saturation behavior under heavy load.

---

### 3.2 Pass 2.1 — Lookup/activation decoupling

Goal: avoid activating scans just to read a GUID.

Changes:
- introduced **lightweight lookup** (`tlLookupScanGuid`) that reads TLS header data without creating a runtime `EmbeddedScan`,
- adjusted reload/import code paths to use lightweight lookup when possible.

Outcome: reduced massive activation during load; further runtime policy work still required.

---

### 3.3 Pass 2.2.A — GUID->path registry

Goal: preserve a path source of truth even when scans are not active.

Changes:
- added persistent `GUID -> path` registry,
- synchronized registry on key operations (lookup/copy/etc.).

Outcome: foundation for lazy activation and robust persistence.

---

### 3.4 Pass 2.2.B — Lazy runtime activation

Goal: activate scans only when runtime access actually needs them.

Changes:
- added lazy activation helper (`ensureScanActive_locked`),
- integrated activation on key runtime entry points (view/info/path/free) as required.

Outcome: restored user-visible behavior with on-demand activation.

---

### 3.5 Pass 2.2.C (Block A) — Save/persistence robustness

Goal: prevent empty/invalid paths during save when scans are not active.

Changes:
- added fallback path behavior using registry/backup path in serialization-related flows.

Outcome: stable save/reopen behavior (no path corruption due to runtime inactive state).

---

### 3.6 Block B (massive import)

#### B2.1 — File-capacity hotfix (Windows)

Goal: remove immediate ~507/508 import wall.

Changes:
- raised CRT stream limit (`_setmaxstdio`) at startup (Windows).

Outcome: 512-scan import succeeded; systematic >507 failures disappeared.

#### B2.2 — Structural budget/eviction policy

Goal: prevent unbounded growth of active scans.

Changes:
- active scan budget,
- eviction of deletable active scans before new activation,
- throttled warning logs when budget blocks activation.

Outcome: improved stability for long sessions and large workloads.

#### B2.3 — Log clarification

Goal: make diagnostics reliable and unambiguous.

Changes:
- explicit cause-oriented logs (open failure, invalid GUID header, inactive scan, budget limit, unresolved path),
- noise reduction via throttling,
- operator-friendly messages for support/QA.

Outcome: significantly improved incident readability.

---

## 4) Technical invariants

1. **GUID lookup is not runtime activation**.
2. **GUID->path registry** is the path source of truth when a scan is not active.
3. Runtime activation is **lazy** and now **bounded** (budget/eviction).
4. Persistence must not rely only on active runtime state.
5. Errors must be logged with explicit causes.

---

## 5) Current functional status (summary)

User validation across:
- small and large projects,
- massive import (drag&drop + import button),
- deletion with/without physical file deletion,
- save/reopen,
- legacy project opening,

is considered successful.

---

## 6) Known limitation / follow-up

Known point to address later:
- when scans are physically moved outside the `Scans` folder, logs correctly report resolution failures but the `?` indicator is not always consistently displayed in the project tree.

This is isolated and should be handled in a dedicated UI/model consistency task.

---

## 7) Operational recommendation

Keep the following as non-regression baseline:
- clarified logs,
- load tests at 50/512/650 scans,
- delete/save/reopen scenarios,

for future scan-runtime evolutions.
