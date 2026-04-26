# Note technique — Stabilité runtime scans (chargement/import gros volumes)

## 1) Contexte initial

Le problème d’origine concernait les projets volumineux (plusieurs centaines de scans), avec un seuil récurrent autour de 507/508 scans :
- échecs d’ouverture TLS,
- UUID invalides,
- scans absents ou marqués comme manquants,
- instabilités lors d’imports massifs.

Le comportement historique attendu (projets >1000 scans possibles) imposait de traiter le problème de manière systémique, pas uniquement locale.

---

## 2) Stratégie globale appliquée

La résolution a été menée de façon incrémentale pour limiter les régressions :

1. **Plan initial en 3 passes**
   - Pass 1 : instrumentation/mesure,
   - Pass 2 : correction technique principale,
   - Pass 3 : validation/robustesse finale.

2. **Pass 2 subdivisée**
   - 2.1 : séparation lookup GUID vs activation runtime,
   - 2.2 : activation différée + stabilisation,
   - 2.3 : finition (logs/documentation).

3. **Pass 2.2 subdivisée**
   - 2.2.A : registre GUID->path,
   - 2.2.B : activation lazy à la demande,
   - 2.2.C : robustesse persistance/save.

4. **Pass 2.2.C ensuite scindée en blocs**
   - Bloc A = 2.2.C (save/path fallback),
   - Bloc B = import/drag&drop massif.

5. **Bloc B subdivisé**
   - B2.1 : hotfix immédiat capacité fichiers,
   - B2.2 : politique structurelle budget/éviction,
   - B2.3 : clarification logs + finalisation.

---

## 3) Détail des travaux réalisés

### 3.1 Pass 1 — Instrumentation

Objectif : prouver la cause en conditions réelles.

Mise en place de métriques d’activité autour de la résolution GUID :
- nombre d’appels,
- succès/échecs,
- cache hit,
- insertions actives,
- pic de scans actifs.

Résultat : confirmation d’un comportement de saturation sous gros volume.

---

### 3.2 Pass 2.1 — Découplage lookup/activation

Objectif : éviter d’activer des scans juste pour lire un GUID.

Changements :
- ajout d’un **lookup léger** (`tlLookupScanGuid`) qui lit l’en-tête TLS sans activer un `EmbeddedScan` runtime,
- adaptation des chemins reload/import pour exploiter ce lookup quand possible.

Effet : baisse de l’activation massive au chargement, mais besoin d’une vraie politique runtime pour l’affichage et opérations associées.

---

### 3.3 Pass 2.2.A — Registre GUID->path

Objectif : conserver une source de vérité chemin même si le scan n’est pas actif.

Changements :
- ajout d’un registre persistant `GUID -> path`,
- synchronisation lors des opérations clés (lookup/copy/etc.).

Effet : base nécessaire pour activation lazy et persistance robuste.

---

### 3.4 Pass 2.2.B — Activation lazy runtime

Objectif : activer un scan uniquement lorsqu’un besoin runtime l’exige.

Changements :
- helper d’activation lazy (`ensureScanActive_locked`),
- intégration dans les points d’entrée runtime (vue, infos, path, free, etc.) selon le besoin.

Effet : restauration du comportement utilisateur (rendu/opérations) avec activation à la demande.

---

### 3.5 Pass 2.2.C (Bloc A) — Robustesse save/persistance

Objectif : éviter des chemins vides/invalides en sauvegarde lorsque des scans ne sont pas actifs.

Changements :
- fallback path basé registre/backup dans les chemins de sérialisation.

Effet : stabilisation save/reopen (pas de corruption de chemins liée à l’état runtime).

---

### 3.6 Bloc B (import/drag&drop massif)

#### B2.1 — Hotfix capacité fichiers (Windows)

Objectif : supprimer le mur immédiat autour de 507/508 sur batch import.

Changements :
- augmentation de la limite CRT (`_setmaxstdio`) au démarrage (Windows).

Effet : import 512 scans passe, disparition des échecs systématiques >507 observés initialement.

#### B2.2 — Politique structurelle budget/éviction

Objectif : empêcher la croissance non bornée des scans actifs.

Changements :
- budget de scans actifs,
- éviction de scans supprimables avant nouvelle activation,
- logs throttlés quand le budget bloque une activation.

Effet : meilleure tenue dans les longues sessions et gros volumes.

#### B2.3 — Clarification des logs

Objectif : rendre le diagnostic terrain fiable et non ambigu.

Changements :
- différenciation explicite des causes (open fail, guid invalide, scan non actif, budget, path unresolved),
- réduction du bruit (throttling),
- messages opérationnels pour support/QA.

Effet : lecture des incidents nettement améliorée.

---

## 4) Invariants techniques à retenir

1. **Lookup GUID ≠ activation runtime**.
2. Le registre **GUID->path** est la base de résolution quand le scan n’est pas actif.
3. L’activation runtime est **lazy** et désormais **bornée** (budget/éviction).
4. La persistance ne doit pas dépendre uniquement d’un scan actif.
5. Les erreurs doivent être loggées avec une cause explicite (diagnostic orienté action).

---

## 5) Résultat fonctionnel actuel (synthèse)

Les campagnes de tests utilisateur menées sur :
- projets petits et volumineux,
- import massif (drag&drop + import button),
- suppression avec/sans suppression physique,
- save/reopen,
- projets anciens,

sont jugées probantes.

---

## 6) Limites connues / suite

Point identifié à traiter ultérieurement :
- lorsqu’un scan est physiquement déplacé hors du dossier `Scans`, le log signale l’échec de résolution mais l’indicateur visuel `?` n’est pas toujours affiché de manière cohérente dans l’arborescence.

Ce point est isolé et peut être traité dans une tâche dédiée de cohérence UI/état modèle.

---

## 7) Recommandation d’exploitation

Conserver :
- les logs clarifiés,
- les tests de charge 50/512/650 scans,
- les scénarios suppression/save/reopen,

comme base de non-régression pour les prochaines évolutions du moteur scan.
