# REngine 2.0 — Engine Systems Reference

keep in mind that this file might not always be up to date. While general design of described system will probably stay the same - some minor changes or bug fixes can probably stay unmentioned.

> Language: **C++17**, Graphics: **OpenGL (ES-compatible)**, Window/Input: **SDL2**, Math: **GLM**, Physics: **JoltPhysics**, Navigation: **Recast/Detour**, Sound: **OpenAL + FMOD Studio**, UI: **Custom + RmlUi**.

---

## Table of Contents

1. [Engine Core (EngineMain)](#1-engine-core-enginemain)
2. [Level System](#2-level-system)
3. [Entity System](#3-entity-system)
4. [Renderer](#4-renderer)
5. [Physics](#5-physics)
6. [Navigation](#6-navigation)
7. [AI Perception](#7-ai-perception)
8. [Behaviour Tree](#8-behaviour-tree)
9. [Sound System](#9-sound-system)
10. [Spatial Sound](#10-spatial-sound)
11. [Particle System](#11-particle-system)
12. [UI System (Custom)](#12-ui-system-custom)
13. [RmlUi Integration](#13-rmlui-integration)
14. [Camera](#14-camera)
15. [Input](#15-input)
16. [Shader & ShaderManager](#16-shader--shadermanager)
17. [Asset Registry](#17-asset-registry)
18. [File System](#18-file-system)
19. [Animation & Skeletal Mesh](#19-animation--skeletal-mesh)
20. [Static Mesh](#20-static-mesh)
21. [Map Loading (Brush Maps & BSP)](#21-map-loading-brush-maps--bsp)
22. [Save System](#22-save-system)
23. [Level Traversal System](#23-level-traversal-system)
24. [Character Controller](#24-character-controller)
25. [Light Manager](#25-light-manager)
26. [Fog Manager](#26-fog-manager)
27. [Debug Draw](#27-debug-draw)
28. [Console](#28-console)
29. [Time](#29-time)
30. [Thread Pool](#30-thread-pool)
31. [Loading Screen](#31-loading-screen)
32. [Video System](#32-video-system)
33. [HTTP Client](#33-http-client)
34. [Resource Statistics](#34-resource-statistics)
35. [LevelObject Factory](#35-levelobject-factory)
36. [Compression / Zip VFS](#36-compression--zip-vfs)
37. [Helper Utilities](#37-helper-utilities)
38. [Model Loading (ModelLoader / Assimp pipeline)](#38-model-loading-modelloader--assimp-pipeline)
39. [GPU Buffer Abstractions](#39-gpu-buffer-abstractions)
40. [Texture System](#40-texture-system)
41. [Platform Startup, SDL2 & OpenGL Context](#41-platform-startup-sdl2--opengl-context)
42. [ImGui Integration](#42-imgui-integration)
43. [MeshUtils](#43-meshutils)

---

## 1. Engine Core (EngineMain)

**Files:** `EngineMain.h`, `EngineMain.cpp`

`EngineMain` is the central orchestrator that ties every other system together. A single global instance is stored in `EngineMain::MainInstance`. It owns the SDL window, the main `Renderer`, the `ThreadPool`, and the `RmlUiContext`.

### Initialization sequence (`Init`)

1. Parse command-line arguments via `ParseCommands` (supports `-key value`, `--key=value`, grouped short flags).
2. Register entity types via `LevelObjectFactory::registerDefaults()` and Behaviour Tree node types.
3. Start the `ThreadPool` and initialize `SoundManager`, `Time`, the developer `Console`, and `Physics`.
4. Mount the `FileSystem`, create the `Renderer` and `UiRenderer`.
5. Initialize `RmlUiContext` (HTML-based UI layer) and `ParticleEmitter` shared VAO.
6. Register default input actions and create an empty `Level`.

### Main loop (`MainLoop`)

Each frame executes in this order:

1. **Frame 5 bootstrap** — `initGame()` creates the `"gamestart"` entity which the game code subclasses to drive level loading.
2. **ImGui** — `ImStartFrame()` only when `DebugUiEnabled` (stripped in `DISTRIBUTION` builds).
3. **Pending level load** — `Level::LoadPendingLevel()` defers level changes safely to the frame boundary.
4. **Touch routing** — touch events are routed to the correct `UiViewport` hit element.
5. **Asset streaming** — `Level::Current->LoadAssets()` calls `LoadAssetsIfNeeded()` on any entity that has not yet loaded.
6. **Time-skip simulation** — `SimulateGameTicksForTime` can fast-forward the game state (e.g. after loading a save) by running thousands of physics + logic ticks with variable precision: low-precision phase (5 ticks/sec) for the bulk, high-precision phase (20 ticks/sec) for the final 10 simulated seconds. A progress bar is drawn via `LoadingScreenSystem` during this process.
7. **Async or sync game update** — `GameUpdate()` runs either on a background `std::future` or on the main thread depending on `asyncGameUpdate`. Inside it: `NavigationSystem::Update`, `Physics::Simulate`, `Level::UpdatePhysics`, `AiPerceptionSystem::Update`, `Level::Update/AsyncUpdate/LateUpdate`, `SoundManager::Update`.
8. **Render** — Runs concurrently with the async game update. Renders the UI to an off-screen RGBA16F render target, calls `MainRenderer->RenderLevel`, composites the UI over the scene with premultiplied-alpha blending, then renders the `RmlUi` overlay.
9. **Finish frame** — After waiting for the async update future: removes destroyed entities, updates camera matrices, finalizes the visible/shadow render lists.
10. **Buffer swap** — `SDL_GL_SwapWindow` + `glFinish/glFlush`.

### Key design decisions

- `asyncGameUpdate = true` allows game logic to overlap with GPU rendering, improving CPU utilization.
- The `LoadingFrames` counter holds the engine in "loading mode" for several frames after a level opens so that assets render correctly before normal culling kicks in.
- The game-tick simulator moves the player far away (`y = −1,000,000`) during low-precision simulation to avoid expensive AI visibility tests.

---

## 2. Level System

**Files:** `Level.hpp`, `Level.cpp`

`Level` is a thread-safe container for all `LevelObject` instances (entities, brushes, etc.) in a running map. The single current level is accessed via `Level::Current`.

### Lifetime

- `OpenLevel(path)` — synchronously loads a `.map` (brush format) or `.bsp` (Quake 3 BSP) file, builds the navmesh, opens FMOD sound banks, starts all entities, and optionally restores a pending save. The static flag `ChangingLevel` is set throughout to let other systems know a transition is in progress.
- `CloseLevel()` — disposes all entities, destroys physics bodies, clears navigation obstacles, flushes physics contact history, and unloads RmlUi documents.
- `LoadLevelFromFile(path)` — deferred version that sets `pendingLoadLevelPath`; the change is applied at the next frame boundary.

### Entity management

- `AddEntity` — thread-safe addition to a pending list (`pendingAddLevelObjects`) with mutex protection. Auto-generates a unique ID of the form `$ClassName_N` and registers the entity in `entityIdMap` and `entityNameMap` hash maps for O(1) lookup.
- `RemoveEntity` — moves to `PendingRemoveLevelObjects`. On `SaveGame` entities, tracks deleted names/IDs for save-state fidelity.
- `RemovePendingEntities` / `MemoryCleanPendingEntities` — two-phase removal: first erase from the array (locked), then delete memory (double-delete guard via `DeletedLevelObjectAdresses`).

### Update model

Each frame calls four distinct stages on all alive `LevelObject` instances:

| Stage | Method | Threading |
|---|---|---|
| Physics sync | `UpdatePhysics()` | Main thread, sequential |
| Gameplay logic | `Update()` | Main thread, sequential |
| Parallel update | `AsyncUpdate()` | Worker thread pool (one job per object) |
| Post-frame | `LateUpdate()` | Main thread, sequential |

`UpdateWhenPaused` / `LateUpdateWhenPaused` flags let individual objects opt into running while the game is paused.

### Render list finalization (`FinalizeFrame`)

Every frame, `FinalizeFrame` walks all `LevelObject::GetDrawMeshes()` and partitions them into opaque (sorted front-to-back) and transparent (sorted back-to-front) lists, with separate shadow and detail-shadow caster lists. It also calls `LightManager::Update()`.

---

## 3. Entity System

**Files:** `Entity.h`, `Entity.cpp`, `LevelObject.hpp`, `EObject.hpp`

### Hierarchy

```
EObject            — base reference-counted object with Dispose/OnDispose lifecycle
  └─ LevelObject   — adds Update/LateUpdate/AsyncUpdate/Finalize/DevUiUpdate virtual hooks
       └─ Entity   — game object with Position/Rotation/Scale, physics bodies, drawables
```

### Entity fields

| Field | Purpose |
|---|---|
| `Position / Rotation / Scale` | World transform |
| `Drawables` | List of `IDrawMesh*` rendered each frame |
| `LeadBody / Bodies` | JoltPhysics bodies; `UpdatePhysics()` syncs position from `LeadBody` each tick |
| `Health / MaxHealth` | Generic damage tracking |
| `Name / Id / ClassName` | Runtime identification; name is set from the map's `targetname` property |
| `Tags` | Arbitrary string tags for filtering |
| `SaveGame / Unique` | Controls save-state serialization behavior |
| `destroyDelay` | Schedules delayed self-destruction |
| `DefaultBrushGroup / DefaultBrushCollisionMask` | Collision filtering for brush entities |

### Lifecycle callbacks

- `FromData(EntityData)` — called during map load to populate properties from the `.map` format.
- `Start()` — records `SpawnTime`; overridden in subclasses for one-time initialization.
- `Update() / AsyncUpdate() / LateUpdate()` — per-frame hooks.
- `Finalize()` — called after the update phase but before rendering begins.
- `OnDamage / OnPointDamage` — hit/damage events with optional causer and weapon references.
- `OnBodyEntered / OnBodyExited` — physics contact events.
- `OnAction(string)` — message-passing system; the static helper `CallActionOnEveryEntityWithName` broadcasts to all entities with a given name.
- `Destroy()` — marks entity for removal.
- `Serialize / Deserialize` — JSON round-trip via nlohmann/json macros.

### Static helpers

- `Entity::Spawn(name)` — factory shortcut.
- `Entity::PreloadEntityType(name)` — forces asset loading without adding to the level.

---

## 4. Renderer

**Files:** `Renderer/Renderer.h`, `Renderer/Renderer.cpp`, `Renderer/Abstractions/FrameBuffer.*`, `Renderer/Abstractions/RenderTexture.*`

The renderer is an OpenGL forward renderer with a depth prepass, MSAA support, cascaded shadow maps, optional motion blur, post-processing, and a custom-ID pass for editor picking.

### Frame pipeline (`RenderLevel`)

```
1. RenderDirectionalLightShadows  (shadow casters → DirectionalShadowMapFBO)
2. PreDraw()                      (per-mesh CPU-side setup)
3. RenderCameraForward            (full scene forward pass)
4. Post-process / compose to screen
```

### RenderCameraForward detail

**A — Depth-only prepass**
- Color writes disabled, depth writes enabled.
- All meshes draw their depth via `DrawDepth(view, projection)`.
- Resolves MSAA depth to single-sample FBO.

**B — Opaque color pass**
- Color writes re-enabled, depth writes disabled.
- `glDepthFunc(GL_LEQUAL)` — only fragments that pass the prepass depth are shaded.
- Each mesh calls `DrawForward(view, projection)`.

**C — Transparent pass**
- BSP transparent faces rendered first (`BspData.RenderTransparentFaces()`).
- Transparent entities sorted back-to-front, drawn with standard blending.
- `DebugDraw::Draw()` runs here.

**D — Custom ID pass**
- Draws each mesh with `DrawCustomId` to an 8-bit RGB off-screen texture.
- Used for mouse-picking in the editor/debug overlay.

### Shadow maps

Two `Framebuffer` objects each hold a single `Depth32F` texture subdivided into a 2×2 grid, giving **4 cascade levels**. `LightManager` calculates the view-projection matrices for each cascade based on camera position and configurable distances/radii. Cascade matrices and shadow map textures are injected into every surface shader via `SetSurfaceShaderUniforms`.

### Motion blur (optional)

A temporal accumulation blur using `motionBlur` and `motionBlur_apply` shaders. Currently disabled (`bulurEnabled = false`) but the full pipeline exists.

### Post-processing

The resolved HDR (`RGBA16F`) color buffer is read by the `postprocessing` pixel shader (tonemapping, gamma correction, etc.) and output to the native screen resolution framebuffer. A separate `fxaa_simple` shader composites the UI render texture with premultiplied-alpha blending.

### Resolution scaling

`ResolutionScale` multiplies the internal render resolution while the output always matches the native window size, enabling dynamic resolution scaling.

### RenderTexture abstraction

`RenderTexture` wraps an OpenGL texture with type (`Texture2D`, `Texture2DMultisample`), format (`RGBA8`, `RGBA16F`, `Depth24`, `Depth32F`, etc.), dynamic resize, sample count changes, and named tracking for `ResourceStatistics`. `Framebuffer` wraps an FBO with color and depth attachments plus a `resolve()` helper for MSAA blit.

---

## 5. Physics

**Files:** `Physics.h`, `Physics.cpp`, `PhysicsConverter.h`, `PhysicsHelper.h`

The physics layer is a thin but complete wrapper around **Jolt Physics**. All JoltPhysics types are used directly; the wrapper handles initialization, body management, simulation stepping, raycasting, and contact event dispatch.

### Collision layers

Three object layers: `NON_MOVING` (static world), `MOVING` (dynamic objects), `TRACE_ONLY` (no-simulate trace bodies). Non-moving only collides with moving; moving collides with everything. `TRACE_ONLY` is invisible to simulation but participates in raycasts.

### Body type bitmask (`BodyType`)

A 32-bit bitmask enum (not the Jolt layer system) used for custom filtering in raycasts and triggers:

- `WorldOpaque`, `WorldTransparent`, `WorldSkybox` — static geometry layers.
- `MainBody` — primary entity body.
- `HitBox` — hitbox bodies (linked to skeletal mesh bones).
- `CharacterCapsule` — character controller capsule.
- `Liquid`, `NoRayTest` — special-purpose types.
- `Area1..5` — game-logic trigger zones.
- `CustomType1..5` — project-specific extension slots.
- Composite masks: `World`, `GroupAll`, `GroupHitTest`, `GroupCollisionTest`.

`BodyData` (stored as Jolt user data) holds the group/mask, owning `Entity*`, hitbox bone name, and owning `SkeletalMesh*` pointer.

### Contact listener (`MyContactListener`)

Tracks pairs of contacting entities across frames using two `std::set` snapshots (`previousContacts`, `currentContacts`). After each simulation step, it fires `Entity::OnBodyEntered` / `OnBodyExited` for newly added and removed contact pairs. An ignore-pair system (`ignoredPairs` unordered set) allows entities to suppress contact events with specific bodies.

### Raycasting and shape casting

The physics API exposes:
- `RayCast(origin, direction, maxDist, BodyType mask)` — single-hit ray test with body-type filtering.
- `SphereTrace(from, to, radius, mask)` — swept sphere hit test, used by particle collision and the character controller.
- `BoxTrace` / `CapsuleTrace` — additional swept shape variants.

### Simulation

`Physics::Simulate()` advances the JoltPhysics world by `Time::DeltaTimeF`, using a `TempAllocator` and `JobSystemThreadPool` sized by `ThreadPool::GetNumThreadsForPhysics()`. `Physics::ResetSimulation()` resets accumulator state after level load.

---

## 6. Navigation

**Files:** `Navigation/Navigation.hpp`, `Navigation/Navigation.cpp`, `Navigation/CrowdAgent.*`, `Navigation/PathFollowQuery.*`, `Navigation/NavigationFileHelper.*`, `Navigation/NavigationGenerationHelpers.hpp`, plus full bundled Recast and Detour source.

The navigation system builds and queries a tile-based navigation mesh using the **Recast** library for mesh generation and **Detour** (+ DetourCrowd) for path queries and agent steering.

### NavMesh generation (`GenerateNavData`)

1. Collects static nav-obstacle meshes from all `LevelObject::GetDrawMeshes()` where `StaticNavigation == true`.
2. Attempts to load a pre-baked navmesh file (`<mapname>.nav`) for fast startup. If absent, runs the full Recast pipeline: rasterize, filter, build compact heightfield, region, contour, poly mesh, and detail mesh.
3. Initializes a `dtTileCache` with `LinearAllocator` + LZ4 compressor for dynamic obstacle support.
4. Initializes a `dtCrowd` with up to 128 agents.
5. Saves the generated data back to disk.

### Path queries

- `FindSimplePath(start, target, acceptanceRadius)` — finds a path via `dtNavMeshQuery::findPath` + `findStraightPath`. Returns a vector of world-space waypoints.
- `FindFleePath / FindFleePathSimple` — samples candidate flee positions on the navmesh and picks the best one based on a flee heuristic that accounts for NPC speed vs player speed and distance advantage.

### Dynamic obstacles

`CreateObstacleBox(min, max)` / `RemoveObstacle(ref)` — adds/removes axis-aligned box obstacles to the tile cache at runtime. This triggers tile cache updates each `Update()` call (`tileCache->update`).

### Crowd simulation

`CrowdAgent` wraps a `dtCrowdAgent` index and exposes `SetTarget`, `GetPosition`, `GetVelocity`. The crowd integrates obstacle avoidance and local steering for all registered agents each frame.

### Path follow query

`PathFollowQuery` computes the steering velocity needed to follow a waypoint path, handling re-pathing, acceptance radius, and path staleness.

---

## 7. AI Perception

**Files:** `AiPerception/AiPerceptionSystem.h`, `AiPerception/AiPerceptionSystem.cpp`, `AiPerception/Observer.h/.cpp`, `AiPerception/ObservationTarget.h`

A lightweight sense system that decouples AI sight computation from entity logic.

### Concepts

- **ObservationTarget** — a world-space position with an owner ID and a list of tags (e.g. `"player"`, `"enemy"`). Any entity that wants to be perceivable creates a target and updates its position each frame.
- **Observer** — a world-space position + forward direction + FOV in degrees. Stores a list of currently visible targets.

### Update loop

Each frame `AiPerceptionSystem::Update()` iterates all observer–target pairs. For each pair it checks:
1. **Distance** — target must be within the observer's perception radius.
2. **FOV** — the angle between `observer.forward` and the direction to the target must be within `fovDeg / 2`.
3. **Line of sight** — a physics raycast against `BodyType::WorldOpaque` must not be blocked.

Results are stored on each `Observer` as a list of visible `ObservationTarget` shared pointers.

### Query API

- `GetObserversInRadius(position, radius)` — spatial query.
- `GetTargetsInRadiusWithTagOrdered(position, radius, tag)` — returns all targets with a specific tag within radius, ordered by distance.

---

## 8. Behaviour Tree

**Files:** `BehaviourTree/BehaviorTree.h/.cpp`, `BehaviourTree/TreeNode.*`, `BehaviourTree/Blackboard.*`, `BehaviourTree/CompositeNodes.*`, `BehaviourTree/DecoratorNodes.*`, `BehaviourTree/TaskNodes.*`, `BehaviourTree/CustomTask.*`, `BehaviourTree/HoldTask.*`, `BehaviourTree/NodeFactory.*`, `BehaviourTree/BTVariable.*`, `BehaviourTree/BehaviorTreeEditor.*`

A full behaviour tree framework used by AI entities.

### Architecture

```
BehaviorTree
  ├─ root : TreeNode
  ├─ Blackboard           (key-value store shared by all nodes)
  ├─ BehaviorTreeContext  (runtime context passed to nodes)
  ├─ activeNodes[]        (nodes currently returning Running)
  └─ pendingAborts[]      (abort queue for Conditional aborts)
```

### Node types

| Type | Nodes |
|---|---|
| **Composite** | `Sequence` (all must succeed), `Selector` (first success), `Parallel` |
| **Decorator** | `Inverter`, `Repeater`, `Cooldown`, `Conditional` (abort support) |
| **Task** | `Wait`, `Log`, `SetBlackboard` |
| **Custom** | `CustomTask` (game-specific scripted tasks), `HoldTask` (wait on external condition) |

All nodes return one of three statuses: `Success`, `Failure`, `Running`.

### Blackboard (`BTVariable`)

Type-erased key-value map. Values can be `bool`, `float`, `int`, `string`, `vec3`, or any serializable type. The blackboard is serialized as part of the tree's save state.

### Abort system

Conditional decorators can request subtree aborts via `BehaviorTree::RequestAbort(node)`. The abort queue is processed at the top of each `Update` call before normal ticking.

### Serialization

`ToJson` / `FromJson` serialize the full tree structure including node types, parameters, and connectivity. `SaveState` / `LoadState` serialize only runtime state (blackboard values, current node progress). Both can be written to disk via `SaveToFile` / `LoadFromFile`.

### Editor

`BehaviorTreeEditor` provides an ImGui-based visual tree editor for inspecting and modifying behaviour trees at runtime.

---

## 9. Sound System

**Files:** `SoundSystem/SoundManager.hpp/.cpp`, `SoundSystem/SoundInstance.*`, `SoundSystem/FmodEventInstance.*`, `SoundSystem/SoundBufferData.h`, `SoundSystem/SoundInstanceBase.h`

The sound system uses a **dual backend** architecture: **OpenAL** for simple file-based playback and **FMOD Studio** for event-driven, bank-based audio.

### OpenAL path

`SoundManager` maintains two `ALCcontext` instances: mono (for spatial audio) and stereo (for music/UI sounds). `SoundBufferData` wraps a loaded PCM buffer. `SoundInstance` wraps an `ALuint` source, offering play/pause/stop/volume/pitch/loop/3D position controls.

`LoadOrGetSoundFileBuffer(path)` loads and caches WAV/OGG data from the file system. `GetSoundFromPath(path)` returns a ready-to-play `shared_ptr<SoundInstance>`.

### FMOD Studio path

`studioSystem` (FMOD::Studio::System) and `coreSystem` (FMOD::System) are initialized once. Banks are loaded via `LoadBankFromPath` with optional `loadSampleData`. `FmodEventInstance` wraps a single `FMOD::Studio::EventInstance` for event-driven sounds with parameter control.

### Update

`SoundManager::Update()` calls `UpdateFmod()` (advances FMOD internals) and synchronizes the OpenAL listener position/orientation with `Camera::finalizedPosition` and `Camera::finalizedRotation`. Global volume, SFX volume, and music volume are applied each update.

### Volume controls

`GlobalVolume`, `SfxVolume`, and `MusicVolume` static floats on `SoundManager` control output levels. Volume is applied via OpenAL gain or FMOD bus volume depending on the instance type.

---

## 10. Spatial Sound

**Files:** `SpatialSound/SpatialSoundManager.*`, `SpatialSound/BspVoxelWorldBuilder.*`, `SpatialSound/VoxelWorld/FixedVoxelWorld.*`, `SpatialSound/VoxelWorld/Raycast.*`, `SpatialSound/VoxelWorld/MaterialProps.h`, `SpatialSound/Helpers.*`

A voxel-based spatial audio subsystem for computing per-source **occlusion** and **reverb** parameters.

### Voxel world

On level load, `SpatialSoundManager::BuildWorld()` voxelizes the static geometry into a `FixedVoxelWorld` — a flat `uint8_t` array indexed by `(x, y, z)` voxel coordinates. Each voxel stores a material ID. The world is cached as a `.svd` binary file next to the map file to avoid rebuilding.

`BspVoxelWorldBuilder` walks the BSP geometry and marks solid voxels. Voxel size is 0.5 units.

### Audio queries

`ComputeAudioSource(position, maxDistance)` returns an `AudioSourceSpatialResult` containing:
- `occlusionGain` — computed by casting rays between the source and listener through the voxel world. Rays that pass through solid (non-transparent) voxels accumulate absorption based on `MaterialProps`.
- `reverb` (ReverbStats) — computed via stochastic ray bouncing in the voxel world to estimate average distance, travel distance, and echo density.

> **Note:** The spatial sound system is currently stubbed out (`return {};` at query start), indicating it is infrastructure-complete but not yet activated in production.

---

## 11. Particle System

**Files:** `Particle/ParticleEmitter.h/.cpp`, `Particle/RibbonEmitter.h/.cpp`, `Particle/ParticleSystem.hpp`, `Particle/GlobalParticleSystem.hpp`

### ParticleEmitter

`ParticleEmitter` implements `IDrawMesh` so it slots directly into the renderer's visible list. Each emitter owns a `std::vector<Particle>` protected by a `std::recursive_mutex`.

**Particle struct** fields: position, velocity, lifetime/deathTime, collision radius, bounce power, size, rotation, transparency, color (vec4), four float user values, and a flag for world-space vs local-space rotation.

**Update** — iterates particles, advances by `velocity * deltaTime`, optionally performs a `Physics::SphereTrace` against `BodyType::World` for collision response (reflection off normals). Spawns new particles at `SpawnRate` per second up to the pool size.

**Draw** — uploads particle transforms to a GPU instance buffer and renders billboarded quads via a shared static VAO (`InitBilboardVaoIfNeeded`). Particle shader receives per-instance color, size, rotation, and opacity.

### RibbonEmitter

Generates a continuous triangle-strip ribbon following particle path history. Useful for trails, beams, and cable simulations.

### ParticleSystem

An `Entity` subclass that owns a list of `ParticleEmitter*` and acts as a prefab container. Exposes `EmitterCount`, global position/rotation/scale delegation to all child emitters.

### GlobalParticleSystem

A singleton-per-name pattern built on top of `ParticleSystem`. `GlobalParticleSystem::SpawnParticleAt(name, position, rotation, scale)` lazily creates one instance per named particle prefab and dispatches spawn calls to it. Avoids duplicating particle system entities.

---

## 12. UI System (Custom)

**Files:** `UI/UiElement.*`, `UI/UiCanvas.hpp`, `UI/UiButton.hpp`, `UI/UiText.hpp`, `UI/UiImage.hpp`, `UI/UiProgressBar.hpp`, `UI/UiHorizontalBox.hpp`, `UI/UiVerticalBox.hpp`, `UI/UiContentBox.hpp`, `UI/UiCustomShaderImage.hpp`, `UI/UiVideo.hpp`, `UI/UiViewport.hpp`, `UI/UiManager.h`, `UI/UiRenderer.*`, `UI/WorldSpace/UiBilboard.*`

A custom immediate-mode-adjacent UI framework for in-game HUD and menus.

### UiElement (base)

All widgets derive from `UiElement : std::enable_shared_from_this<UiElement>`. Core properties:

- `position`, `size` — normalized [0, 1] coordinates within the parent.
- `origin`, `pivot` — alignment anchors.
- `rotation` — element rotation in degrees.
- `color`, `inheritParentColor` — tint with optional parent color inheritance.
- `HitCheck` — if true, participates in touch/mouse hit testing via `GetHitElementUnderPosition`.
- `PixelShader` — optional custom OpenGL fragment shader for the element.
- `children / finalizedChildren` — tree structure; `FinalizeChildren` snapshots children for thread-safe rendering.

The update loop calls `UpdateChildrenOffsetRecursive()` to propagate absolute screen offsets from the viewport root down, then `FinalizeChildren()` to produce safe render snapshots.

### Widget types

| Widget | Description |
|---|---|
| `UiCanvas` | Root container, defines coordinate space |
| `UiButton` | Clickable element with hover/press callbacks |
| `UiText` | SDL_ttf rendered text with font/size/wrap |
| `UiImage` | Textured quad |
| `UiCustomShaderImage` | Image rendered with arbitrary GLSL pixel shader |
| `UiProgressBar` | Fill-based progress display |
| `UiHorizontalBox` | Horizontal auto-layout |
| `UiVerticalBox` | Vertical auto-layout |
| `UiContentBox` | Scrollable content container |
| `UiVideo` | Renders a `Video` stream as a texture |
| `UiViewport` | Top-level viewport, handles touch routing |

### World-space billboard

`UiBilboard` renders a `UiCanvas` attached to a 3D world position, always facing the camera. Used for health bars, name labels, and interactive 3D UI.

### UiRenderer

`UiRenderer` is the backend that translates `UiElement` draw calls into OpenGL geometry (quads with texture + color + transform), batched and sorted by shader program to minimize state changes.

### Rendering integration

The engine renders the UI into a separate off-screen `RenderTexture` (RGBA16F) at a logical resolution derived from `UiManager::GetScaledUiHeight()`, then composites it over the 3D scene using premultiplied-alpha blending in the main `Render()` call.

---

## 13. RmlUi Integration

**Files:** `UI/RmlUi/RmlUiContext.*`, `UI/RmlUi/RmlUiEvents.h`, `UI/RmlUi/Backends/RmlUi_Backend_SDL_GL3.*`, `UI/RmlUi/Backends/RmlUi_Platform_SDL.*`, `UI/RmlUi/Backends/RmlUi_Renderer_GL2.*`, `UI/RmlUi/Backends/RmlUi_Renderer_GL3.*`, `UI/RmlUi/Backends/MyFileInterface.h`

Provides HTML/CSS-based UI on top of the **RmlUi** library, used for menus, HUDs, and complex data-driven layouts. GL3 and GL2 rendering backends are both included for platform flexibility.

### RmlUiContext

Wraps a single `Rml::Context*` and owns:

- **Modal stack** — `PushModal(doc)` / `PopModal()` push/pop document modals. `IsUIBlockingInput()` returns true when any modal is open, allowing the game to pause input processing.
- **Document management** — `LoadDocument`, `ShowDocument`, `HideDocument`, `UnloadDocument`, `UnloadAllDocuments`.
- **Event forwarding** — `ProcessEvent(SDL_Event&)` routes SDL events to RmlUi's SDL platform backend.
- **Resize** — `OnResize(w, h)` updates the context dimensions.
- **Callbacks** — `GameUpdateCallback` and `PreRenderCallback` hooks allow the game to inject logic tied to modal state.

### File interface

`MyFileInterface` derives from `Rml::FileInterface` and routes all RmlUi asset reads through the engine's `FileSystemEngine` API, enabling .zip-virtual-filesystem-hosted UI assets.

---

## 14. Camera

**Files:** `Camera.h`, `Camera.cpp`

A fully static class (all members are `static`) representing the single world camera.

### Matrices

| Matrix | Description |
|---|---|
| `view` | World-to-camera |
| `projection` | Perspective projection (standard FOV) |
| `projectionViewmodel` | Separate perspective with `ViewmodelFOV` for weapon models |
| `finalizedView/Projection/ProjectionViewmodel` | Copies snapped at frame end for thread-safe renderer access |

`finalizedPosition` and `finalizedRotation` are the corresponding snapped world-space values.

### Update

`Camera::Update(deltaTime)` applies all active `CameraShake` effects to `position` and `rotation`, cleans up finished shakes, then rebuilds the view and projection matrices. It also updates the `Frustum` for culling.

### Camera shake

`CameraShake` structs support two modes: `SingleWave` (sine-based) and `PerlinNoise` (continuous). Each shake has position and rotation amplitudes/frequencies, a falloff curve, a blend-in duration, and an intensity scalar. Multiple shakes are summed. `AddCameraShake(shake)` pushes a new shake; finished shakes are pruned each frame.

### Frustum culling

`FrustrumCull.hpp` provides a `Frustum` struct with six planes extracted from the view-projection matrix. `IDrawMesh::IsCameraVisible()` calls `IsInFrustrum(Camera::frustum)` on each mesh's bounding box or sphere.

---

## 15. Input

**Files:** `Input.h`, `Input.cpp`

SDL2-based input system exposing an **action-based** API with support for keyboard, mouse, gamepad, and touch.

### InputAction

Each action has optional bindings to: `SDL_Scancode`, left/right mouse buttons, scroll, gamepad button (`GamepadButton` enum with Emscripten remapping), and gamepad analog axes (left/right sticks + triggers).

Query methods:
- `Pressed()` — true for one frame when input first activates.
- `Released()` — true for one frame when input deactivates.
- `Down()` — true every frame while held.
- `GetAxis()` — analog value [-1, 1] for sticks/triggers.

### Mouse

`Input::MouseDelta` tracks raw mouse movement. `MouseMoveCalculator` can be subclassed to override delta computation (e.g. gyroscope input). `UpdateMouse()` runs after input processing to compute delta from the previous frame's position.

### Touch

`TouchEvent` structs carry touch id, pressed/released flags, position, and delta. Events are stored in `Input::TouchActions` and dispatched to the correct `UiElement` hit region by `EngineMain::MainLoop`.

### Gamepad

Full SDL2 gamepad support with analog deadzone handling. The `GamepadButton` enum has separate Emscripten mappings to handle browser gamepad API differences.

---

## 16. Shader & ShaderManager

**Files:** `Shader.hpp`, `Shader.cpp`, `ShaderManager.h`

### Shader

Wraps a single GLSL shader stage (`GL_VERTEX_SHADER` or `GL_FRAGMENT_SHADER`). Key methods:
- `Shader::FromCode(code, type)` — compile from a string.
- `Shader::FromFile(path, type)` — load via `FileSystemEngine`, supports `#include` directives resolved recursively.
- `CompileShader()` — calls `glShaderSource` + `glCompileShader` with error reporting.

Shaders track which `ShaderProgram` instances they're attached to, enabling hot-reload: recompiling a shader invalidates and re-links all programs using it.

### ShaderProgram

Wraps a linked GLSL program. Provides fluent API:
- `AttachShader(shader)` → `LinkProgram()` → ready to use.
- `UseProgram()`.
- `SetUniform(name, value)` — overloaded for `float`, `int`, `vec2`, `vec3`, `vec4`, `mat4`, `ivec2`.
- `SetTexture(name, textureId)` — auto-increments texture unit, caches uniform locations via `hashed_string`.

### ShaderManager

A static map from `"vertexName" + "pixelName"` key to `ShaderProgram`. `GetShaderProgram(vert, pixel)` returns a cached program, creating and linking it on first access via `AssetRegistry::GetShaderByName`. This is the primary shader access point throughout the engine.

---

## 17. Asset Registry

**Files:** `AssetRegistry.h`, `AssetRegistry.cpp`

Centralized cache and lifecycle manager for all GPU and CPU assets.

### Caches

| Type | Key | Notes |
|---|---|---|
| `Shader*` | filename | Vertex and pixel caches separate |
| `Texture*` | path | Loaded via stb_image |
| `CubemapTexture*` | path | Six-face cubemap |
| `roj::SkinnedModel*` | path | Full mesh + skin + bones |
| `roj::SkinnedModel*` (animations) | path | Animation-only model (`GetSkinnedAnimationFromFile`) |
| `Video*` | path | MPEG-1 video stream |
| `CachedFont` | path + fontSize | `TTF_Font*` via SDL_ttf, retains font buffer to keep memory alive |

### Level loading boundary

`BeginLevelLoad()` starts tracking newly accessed assets. `EndLevelLoad()` marks them as "used during this level." `ClearUnusedMemory()` evicts cached assets not in the used set, enabling level-scoped asset management. `ClearMemory()` does a full eviction.

### Shader hot reload

`ReloadShaders()` iterates the shader cache and recompiles each one, then marks all linked `ShaderProgram` instances as dirty for re-link. Typically bound to a console command.

---

## 18. File System

**Files:** `FileSystem/FileSystem.h`, `FileSystem/FileSystem.cpp`, `FileSystem/ZipVFS.h`, `FileSystem/ZipVFS.cpp`

Namespace `FileSystemEngine` abstracts file access across:
- **Physical filesystem** — for `GameData/` and `SaveData/` directories.
- **ZipVFS** — a virtual filesystem backed by `.zip` archives, useful for shipping assets in a single or multiple zip files. Zip files can be nested.

### API

```cpp
std::string ReadFile(path)                         // Text read
std::vector<uint8_t> ReadFileBinary(path)          // Binary read
bool WriteFile(path, content)                      // Text write
bool WriteFileBinary(path, data)                   // Binary write
std::vector<std::string> GetFilesInPath(path)      // Directory listing
uint32_t GetFileModificationTime(path)             // Unix timestamp
```

Reads attempt the physical path first; if absent, they fall through to the ZipVFS. This allows patching game data without rebuilding archives.

---

## 19. Animation & Skeletal Mesh

**Files:** `SkeletalMesh.hpp`, `SkeletalMesh.cpp`, `Animation.h`, `Animator.h`, `animator.cpp`, `model_animator.hpp`, `model_animator.cpp`, `skinned_model.hpp`, `skinned_model.cpp`

### AnimationPose

A map from `hashed_string` bone name → `mat4` local-space transform. Serializable via `NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT`.

**Static operations:**

`Lerp(a, b, t)` — full skeleton blend. Fast-paths at t < 0.005 (return A, insert B-only bones) and t > 0.995 (return B). For each shared bone it decomposes both matrices into translation/scale/quaternion, slerps the quaternion and mixes T and S, then rebuilds the matrix. The "root" bone always comes from B.

`LayeredLerp(startBoneName, rootNode, poseA, poseB, useWorldSpaceRotation, t)` — blends only the subtree descending from `startBoneName`; bones outside the subtree stay at A. Uses a process-lifetime cache (`s_traversalMap`, `s_indexMap`, `s_parentIndexMap`, `s_childrenMap`) keyed by `BoneNode*` and protected by `std::shared_mutex` for concurrent reads. The start bone supports an optional world-space rotation blend (blends global quaternions, then converts back to local space via parent inverse). All descendant bones use simple local slerp. Root always from B.

`ApplyFABRIK(chainRootName, endEffectorName, rootNode, inputPose, targetPosition, targetRotation, snapRotation, maxIterations, tolerance)` — FABRIK two-pass IK solver. Collects the chain of bones from `endEffector` up to `chainRoot`, runs forward (end→root) and backward (root→end) passes in global space, then converts results back to local-space matrices. `snapRotation = true` orients the end effector to `targetRotation`.

`GetModelSpaceTransform(boneName, rootNode, pose)` — walks the parent map to accumulate a global model-space matrix for any named bone.

**Bone transform helpers on the struct:**
```cpp
void SetBoneTransform(hashed_string bone, mat4)
void SetBoneTransform(hashed_string bone, MathHelper::Transform)
void SetBoneTransformEuler(hashed_string bone, MathHelper::Transform)
MathHelper::Transform GetBoneTransform(string bone)
```

### roj::Animator (low-level sampler)

`model_animator.hpp/.cpp` — samples a `roj::SkinnedModel`'s animation data into a flat `std::vector<glm::mat4> m_boneMatrices` (up to `MAX_SKINNED_BONES = 128`) suitable for uploading to a shader uniform.

**Precaching** — `set(animationName)` calls `precacheAnimation()` which builds flat `cachedFrameBoneTransforms` and `cachedBoneInfos` vectors indexed by `BoneNode::id` (a `uint16_t` assigned during precache DFS). This avoids hash map lookups per bone per frame.

**Per-frame** — `update(dt)` advances `m_currTime` by `ticksPerSec * dt`, then calls `calcBoneTransform(rootBone, identity, stopAfterRoot)`. Each bone:
1. Calls `interpolatePosition`, `interpolateRotation`, `interpolateScaling` on its `FrameBoneTransform` (linear/slerp between adjacent keyframes using `getKeyTransformIdx` + `getScaleFactor`).
2. Multiplies TRS matrices → local pose stored in `currentPose[node.name]`.
3. Multiplies parent offset → `m_boneMatrices[boneInfo.id] = offset * boneInfo.offset`.

**Interpolation flags:** `InterpolateRotation`, `InterpolatePosition`, `InterpolateScale` (disable for discrete key snapping).

**Loop wrap:** when looping is enabled and playhead is in the last segment, it blends last keyframe → first keyframe across the animation boundary.

**Precomputed frames:** `UsePrecomputedFrames = true` switches to `useBakedFrame(time)` which reads from `BakedFrameData` (30 fps baked at load time into `Animation::bakedFrames`), copying `boneTransforms` and `m_boneMatrices` directly.

**Root motion:** `updateRootMotion()` accumulates `totalRootMotionPosition` and `totalRootMotionRotation` by comparing the current and previous `rootBoneTransform` (decomposed from the "root" bone).

**Pose injection:** `ApplyBonePoseArray(pose)` / `ApplyLocalSpacePoseArray(pose, overridePose)` walk the bone tree applying a caller-supplied pose map instead of the animation curves, allowing game code or the `AnimationPose` system to drive the skeleton directly.

### SkeletalMesh

Extends `StaticMesh`. Owns a `roj::Animator` and a `std::vector<glm::mat4> boneTransforms` (current) + `finalizedBoneTransforms` (snapshotted for the render thread via `FinalizeFrameData()`).

**Loading:** `LoadFromFile(path)` calls `StaticMesh::LoadFromFile`, resizes `boneTransforms` to `model->boneInfoMap.size()`, creates a new `roj::Animator(model)`, and calls `LoadMetaFromFile()` to read the `.json` sidecar (`SkeletalMeshMetaData` with `HitboxData` and `AnimationData` arrays).

**Playback:** `PlayAnimation(name, loop, interpIn)` calls `animator.set(name)`, `animator.play()`, and records `blendStartPose` + blend time window. During `Update()`, the animator is ticked, the result pose is blended with `blendStartPose` via `AnimationPose::Lerp` at the current blend fraction (`GetBlendInProgress()`), then pasted back via `PasteAnimationPose`.

**Shader upload:** `ApplyAdditionalShaderParams(shader_program)` uploads `finalizedBoneTransforms` as the `finalBonesMatrices` uniform array (up to 128 mat4s).

**Ragdoll:** `StartRagdoll()` / `StopRagdoll()` toggle `InRagdoll`. While active, `UpdateHitboxes()` reads hitbox body transforms from `Physics` and feeds them back into the animator pose.

**Hitboxes:** `CreateHitboxes(owner)` iterates `metaData.hitboxes` and calls `Physics::CreateBox` for each bone, storing bodies in `hitboxBodies`. `TwoBodyConstraint` objects with twist/limit parameters are created for bone-to-bone connections and stored in `hitboxConstraints`. Access is protected by `std::recursive_mutex hitboxMutex`.

**Animation events:** `UpdateAnimationEvents()` scans `currentAnimationData->animationEvents` for events whose time falls between `oldAnimationEventTime` and the current animation time, queueing them into `pendingAnimationEvents`. Callers drain via `PullAnimationEvents()`.

**State serialization:** `GetAnimationState()` / `SetAnimationState(state)` pack/unpack the full `AnimationState` struct (name, time, loop, ragdoll hitbox transforms/velocities) for save/restore.

### Animation class

`Animation.h` — trivially extends `SkeletalMesh` with `skipMeshLoad = true` (only bones and animations are loaded, no mesh geometry). All draw methods (`DrawForward`, `DrawDepth`, `DrawShadow`) are no-ops. `IsCameraVisible()` always returns false. Used as a pose-only asset that drives other `SkeletalMesh` instances.

### High-level Animator (gameplay layer)

`Animator.h` / `animator.cpp` — gameplay-facing wrapper that owns a list of `Animation*` objects and blends them into a single `AnimationPose`.

```cpp
Animator(Entity* owner);
Animation* AddAnimation(string path, string name = "", bool loop = true);
void Update();                        // ticks all owned Animation instances
AnimationPose GetResultPose();        // calls ProcessResultPose() → override in subclass
void LoadAssetsIfNeeded();
float Speed;
bool UpdatePose;
bool UsePrecomputedFrames;
```

Game code subclasses `Animator`, overrides `LoadAssets()` to call `AddAnimation(...)` for each clip, and overrides `ProcessResultPose()` to blend them. Each `Animation` object propagates `UpdatePose` and `UsePrecomputedFrames` before ticking. Animation events from all child animations are aggregated into `pendingAnimationEvents`.

### SkinnedModel data structures

**`roj::SkinnedMesh`** — one submesh: `VertexBuffer* vertices`, `IndexBuffer* indices`, `VertexArrayObject* VAO`, texture list, `materialName`, `name`, cached `Texture*` pointers for base color and emissive, `transparentTexture` flag. `ProcessDefaultTextures()` resolves embedded texture pointers from the asset registry. `DestroyBuffers()` deletes all three GPU objects.

**`roj::SkinnedModel`** — complete loaded model: `vector<SkinnedMesh> meshes`, `unordered_map<hashed_string, BoneInfo> boneInfoMap` (bone name → id + offset matrix), `unordered_map<hashed_string, Animation> animations`, `BoneNode defaultRoot`, `boneNodesMap`, `parentMap`, `BoundingSphere`, `BoundingBox`, `sceneCamera` (if present in the file), `globalInversed`.

**`roj::Animation`** — one clip: `duration`, `ticksPerSec`, `frameTime`, `BoneNode rootBone` (full hierarchy snapshot), `unordered_map<hashed_string, FrameBoneTransform> animationFrames`, `vector<BakedFrameData> bakedFrames`.

**`roj::FrameBoneTransform`** — three parallel arrays of timestamps + values for position/rotation/scale keyframes.

**`roj::BakedFrameData`** — pre-computed frame: `unordered_map<hashed_string, mat4> boneTransforms`, `totalRootMotionPosition/Rotation`, `vector<mat4> modelTransform` (ready-to-upload bone matrices).

---

## 20. Static Mesh

**Files:** `StaticMesh.h`, `StaticMesh.cpp`, `IDrawMesh.h`, `VertexData.h`

`StaticMesh` is the base renderable unit. It implements `IDrawMesh` which the renderer calls during each render pass.

### IDrawMesh interface

```cpp
void DrawForward(mat4 view, mat4 projection)   // Color/lighting pass
void DrawDepth(mat4 view, mat4 projection)      // Depth prepass
void DrawShadow(mat4 view, mat4 projection)     // Shadow map pass
void DrawCustomId(mat4 view, mat4 projection)   // Editor pick ID buffer
void PreDraw()                                  // CPU-side prep
void FinalizeFrameData()                        // Snapshot state for render thread
bool IsCameraVisible()                          // BSP PVS + frustum check
bool IsShadowVisible()                          // Shadow frustum check
float GetDistanceToCamera()
```

### DrawForward

Uses `ShaderManager::GetShaderProgram("default_vertex", PixelShader)` (cached by name pair). Per-draw GL state:
- `glDepthMask(GL_TRUE/FALSE)` — controlled by `DepthWrite` and `Transparent` flag.
- `glEnable/Disable(GL_CULL_FACE)` — controlled by `TwoSided`.

Uniforms set: `masked`, surface uniforms from `Renderer::SetSurfaceShaderUniforms`, `light_color` / `direct_light_color` / `direct_light_dir` from `GetLightVolData()`, `view`, `projection`, `viewmodelScaleFactor`, `world` (from `finalizedWorld`), `isViewmodel`, `customId`. Point lights applied via `LightManager::ApplyPointLightToShader` using the mesh's `BoundingBox`.

Texture binding: if no `ColorTexture` is set, looks up `mesh.cachedBaseColor` and `mesh.cachedEmissiveColor` from `AssetRegistry::GetTextureFromFile(TexturesLocation + textureName)`. For transparent submeshes, `glDepthMask(GL_FALSE)` if `mesh.transparentTexture`. Binds as `u_texture` and `u_textureEmissive`.

Draw call per submesh:
```cpp
mesh.VAO->Bind();
if (mesh.VAO->IsInstanced())
    glDrawElementsInstanced(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0, instanceCount);
else if (numInstances < 0)
    glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0);
VertexArrayObject::Unbind();
```

### DrawDepth

Uses `"default_vertex"` + `"empty_pixel"` (no fragment output) or `"mask_pixel"` if `Transparent || Masked`. Sets `view`, `projection`, `world`, `viewmodelScaleFactor`, `isViewmodel`. For masked meshes, also binds `u_texture`. Skips entirely if `DepthPrePath == false`.

### DrawShadow

Same shader selection as `DrawDepth`. Does not set `isViewmodel` to the actual flag — always passes `false` so viewmodel geometry casts shadows as normal world geometry. Sets `view`, `projection`, `world`.

### DrawCustomId

Uses `"default_vertex"` + `"customId"` shader. Sets `customId` (int packed into RGB), `view`, `projection`, `world`, `isViewmodel`. Returns immediately if `CustomId == 0`.

### StaticMesh key fields

`model` (`roj::SkinnedModel*`) — the loaded model (shared via `AssetRegistry`). `TexturesLocation` — path prefix for texture lookup. `PixelShader` — fragment shader name (default `"default_pixel"`). `MeshHideList` — names of submeshes to skip during rendering (snapshotted to `finalMeshHideList` at `FinalizeFrameData`).

`CastShadows`, `CastHiddenShadows` (cast shadow but invisible in forward pass), `CastDetailShadows`. `Masked` enables alpha-test. `TwoSided` disables culling. `Transparent` marks the mesh for the transparent render pass. `DepthWrite` / `DepthPrePath` control depth buffer participation.

`IsViewmodel` — routes to `Camera::finalizedProjectionViewmodel` (narrower FOV, no depth conflict with world geometry). `ViewmodelScaleFactor` adjusts the projection further.

`CustomId` — integer for editor mouse-pick. `ColorTextureId` / `EmissiveTextureId` — raw GL texture IDs usable as fallbacks when no `Texture*` pointer is set.

`numInstances` — when ≥ 0 the draw is skipped (`glDrawElements` is not called), used as a hook for external instancing logic.

### Visibility testing

`IsCameraVisible()` first checks the Quake 3 BSP PVS if a BSP is loaded (`BspData.IsClusterVisible`), then performs bounding-sphere frustum culling (`IsInFrustrum(Camera::frustum)`). For viewmodel meshes, BSP PVS is skipped.

`IsInFrustrum(frustum)` transforms `model->boundingSphere` by position/rotation/scale and calls `frustum.IsSphereVisible(offset, radius)`.

`GetBoundingBox()` returns `model->boundingBox.Transform(finalizedWorld)`.

### FinalizeFrameData

Snapshots `finalizedWorld`, `finalizedPosition`, `finalizedRotation`, `finalizedScale`, and `finalMeshHideList` (copied from `MeshHideList`) so the render thread sees a consistent frame while the game thread may already be updating the next frame.

---

## 21. Map Loading (Brush Maps & BSP)

**Files:** `MapParser.h`, `MapParser.cpp`, `MapData.h`, `MapData.cpp`, `BSP/Quake3Bsp.h`, `BSP/Quake3Bsp.cpp`, `Entities/Brushes/`, `BrushFaceMesh.hpp`

The engine supports two map formats:

### Custom `.map` format (Quake/TrenchBroom style)

`MapParser::ParseMap(path)` reads text-based `.map` files. Each entity block contains key-value properties and optional brush definitions (sets of planes). The parser:

1. Extracts entity key-value pairs (including `classname`, `origin`, `targetname`, etc.).
2. For brush entities, computes vertices by finding the intersection points of all plane triples (`GetPlaneContacts`).
3. Generates triangulated `BrushFaceMesh` geometry with UV coordinates.
4. Returns a `MapData` object whose `LoadToLevel()` instantiates entities via `LevelObjectFactory`.

### Quake 3 BSP (`.bsp`)

`CQuake3BSP` reads the standard Quake 3 BSP binary format:
- **Lumps**: vertices, faces, textures, lightmaps, nodes, leaves, models.
- `BuildVBO()` — uploads face geometry to GPU vertex/index buffers.
- `GenerateTexture()` — loads face textures via `AssetRegistry`.
- `GenerateLightmap()` — builds a composite lightmap atlas.
- `BuildMergedModels()` — merges brush models into `StaticMesh` instances.
- `BuildStaticOpaqueObstacles()` — creates Jolt physics bodies for solid BSP faces.
- `RenderTransparentFaces()` — called inside the transparent render pass for BSP faces flagged as translucent.
- `PreloadFaces()` — warms asset caches for all referenced textures.

### Brush entity types

- `TriggerBase` / `TriggerOnce` — proximity triggers that fire `OnAction` on named targets.
- `MovebleBrush` — a brush that can be moved/rotated at runtime (platforms, doors).
- `DestructibleBrush` — brush that breaks when damaged.
- `AreaBase` — zone volumes using the `Area1..5` body type layers for soft area detection.
- `vis_through` — visual-only brush with no collision.

---

## 22. Save System

**Files:** `SaveSystem/GameSaveSystem.h/.cpp`, `SaveSystem/LevelSaveSystem.h/.cpp`, `SaveSystem/LevelSaveData.h`, `SaveSystem/EntitySaveData.h`

Two-tier save architecture:

### LevelSaveSystem (per-level)

`LevelSaveData` contains:
- A list of `EntitySaveData` (serialized entity states as JSON).
- Lists of deleted entity names and IDs (so respawned entities can be suppressed on load).

`LevelSaveSystem::SaveLevelToData()` iterates `Level::Current->LevelObjects`, calls `Entity::Serialize(json)` on every entity with `SaveGame = true`, and packs results into a `LevelSaveData`.

`LevelSaveSystem::LoadLevelFromData(data)` applies the saved state: restores serialized entities, destroys entities whose IDs appear in the deleted lists, and calls `Entity::Deserialize` on existing entities.

A `pendingSave` static is set before a level load so the data is applied immediately after `OpenLevel` completes.

### GameSaveSystem (whole-game)

`GameSaveData` contains the current `LevelSaveData` plus a `LevelMemory` map (level path → saved state for all previously visited levels).

`SaveGameToFile(name)` / `LoadGameFromFile(name)` serialize to/from JSON files under `SaveData/`. `LoadGameFromData` feeds the correct level save into `LevelSaveSystem::pendingSave` and triggers a level load via `LevelTraversalSystem`.

---

## 23. Level Traversal System

**Files:** `LevelTraversalSystem.h`, `LevelTraversalSystem.cpp`

Manages seamless player travel between levels while preserving state.

### Travel flow

`TravelToLevel(levelPath, playerEntity, spawnPointName)`:
1. Serializes the player entity to `PlayerSerializedData` JSON.
2. Stores the current level's save state into `LevelMemory[currentLevelPath]`.
3. Sets `DesiredSpawnPointName` and `Traveling = true`.
4. Requests a level load via `Level::LoadLevelFromFile`.

`FinishTransition()` (called inside `Level::OpenLevel` after entities start):
1. Finds the entity named `DesiredSpawnPointName` (a `transformPoint`) and teleports the player there.
2. Applies `PlayerSerializedData` back to the player entity via `Deserialize`.
3. Checks `LevelMemory` for a stored state of the new level and applies it if present.

### Time skip

`TimeSkip` (float seconds) triggers `SimulateGameTicksForTime` after a level load, used to let NPCs settle and pathfind to their initial positions before the player sees the world.

---

## 24. Character Controller

**Files:** `Character/CharacterController.h`, `Character/CharacterController.cpp`

A virtual character controller built on JoltPhysics's `CharacterVirtual`.

### Core behavior

`Init(owner, position, radius, height)` creates a capsule-shaped body and registers it with physics.

`Update(deltaTime)` each tick:
1. Applies gravity to velocity if not on ground.
2. Calls `CharacterVirtual::Update` with the current velocity.
3. Runs `UpdateGroundCheck` — a series of downward sweeps to determine ground contact, slope angle, whether the surface is walkable (below `groundMaxAngle`), and the body being stood on.
4. Handles step-up smoothing via `heightSmoothOffset` lerped at `stepSmoothingSpeed`.
5. Detects platform riding: when standing on a moving body, the relative velocity of the platform is added to the character's translation.

### Crouch

`Crouch()` shrinks the capsule to `crouchHeight`, lowers `targetCameraHeight` to `cameraHeightCrouching`. `UnCrouch()` calls `CanStandUp()` (sphere overlap check above the character) before re-expanding.

### Movement quality

`CharacterControllerMovementQuality` switches between `Player` (high-quality, precise step handling) and `NpcGeneric` (cheaper, less precise) sweep modes.

### Smooth position

`GetSmoothPosition()` returns a version of the position with step-height offsets interpolated out, preventing visible pops when the character steps up geometry.

---

## 25. Light Manager

**Files:** `LightSystem/LightManager.h`, `LightSystem/LightManager.cpp`

Manages the directional (sun) light and dynamic point lights.

### Directional light

`LightDirection` (normalized vec3) is the global sun direction. Each frame, `Update()` calls `CalculateLightMatrices` four times with increasing `lightDistance` values to produce four cascaded shadow projection matrices. The distance values (`LightDistance1..4`) and radii (`LightRadius1..4`) are configurable. `LightDistanceMultiplier` scales all distances uniformly.

### Point lights

`UpdateLightSource(PointLightInfo)` adds a point or spot light to the per-frame `m_lights` list. `ApplyPointLightToShader(shader, boundsMin, boundsMax)` selects the N most influential lights within a mesh's bounding volume and uploads them as shader uniforms. `FinishPointLights` swaps the per-frame list into `m_finalLights` after the update phase.

`PointLightInfo` fields: `position`, `color`, `radius`, `direction`, `innerConeAngleDegrees`, `outerConeAngleDegrees` (zero cone = omnidirectional).

---

## 26. Fog Manager

**Files:** `FogManager.h`

A simple static class for linear depth-based fog parameters shared across all surface shaders.

| Parameter | Default | Description |
|---|---|---|
| `StartDistance` | -10 | Fog starts at this world depth |
| `EndDistance` | 70 | Fog reaches full opacity here |
| `Opacity` | 0 | Master fog opacity (0 = off) |
| `Color` | (0.8, 0.8, 0.8) | Fog color in linear space |

These values are pushed into every surface shader via `Renderer::SetSurfaceShaderUniforms`.

---

## 27. Debug Draw

**Files:** `DebugDraw.hpp`, `DebugDraw.cpp`

Thread-safe immediate-mode debug geometry rendering. All draw calls are safe to issue from any thread.

### Architecture

Commands are stored as `std::unique_ptr<DebugDrawCommand>` in a mutex-protected `commands` list. Each `DebugDrawCommand` has a `Delay` timer; commands expire after their duration. At the start of each render frame, `Finalize()` snapshots the live command list into `finalizedCommands` (raw pointers, no ownership). `Draw()` iterates `finalizedCommands` and calls `command->Draw()`. `ClearCommands()` wipes both lists.

### Available draw calls

- `DebugDraw::Line(start, end, duration, thickness)` — renders a thin box-mesh oriented along the line.
- Additional geometric primitives (sphere, box, text) can be added as `DebugDrawCommand` subclasses.

---

## 28. Console

**Files:** `Console/Console.h`, `Console/Console.cpp`, `Console/ConsoleCommand.h`, `Console/ConsoleRegister.h`, `Console/ConsoleDefaultCommands.*`

A developer console rendered via **ImGui**.

### Command system

`ConsoleCommand` stores a name, description, argument spec, and a `std::function<void(vector<string>)>` handler. `Console::RegisterCommand(cmd)` adds it to the lookup map. `Execute(input)` tokenizes the input, resolves the command name (with error on unknown), and calls the handler. Argument helpers `ArgInt`, `ArgFloat`, `ArgString`, `ArgBool` provide safe parsed access to `args[i]`.

### Default commands (ConsoleDefaultCommands)

Pre-registered system commands include: `reload_shaders`, `set_fov`, `set_timescale`, `nav_debug`, `toggle_shadows`, `quit`, and others. Each is registered in `RegisterAll()` at engine startup.

### UI features

- **Auto-complete** — `UpdateSuggestions()` filters registered command names against the current input token. Suggestions appear as a dropdown list.
- **History** — Up/Down arrows cycle through previously entered commands (capped at `m_maxHistory`).
- **Timestamps** — Optionally prepend log entries with elapsed time.
- **Color-coded output** — Each `LogItem` carries an `ImVec4` color for info/warning/error distinction.
- **Input callback** — Uses `ImGuiInputTextCallbackData` for dynamic suggestion and history navigation.

---

## 29. Time

**Files:** `Time.hpp`, `Time.cpp`

Static class providing all timing values.

### Core values

| Field | Type | Description |
|---|---|---|
| `DeltaTime` | `double` | Frame duration in seconds |
| `DeltaTimeF` | `float` | Same as float |
| `DeltaTimeNoTimeScale` | `double` | DeltaTime ignoring TimeScale |
| `DeltaTimeFNoTimeScale` | `float` | Same as float |
| `GameTime` | `double` | Accumulated game time (paused when game is paused) |
| `GameTimeNoPause` | `double` | Accumulated time even while paused |
| `TimeScale` | `float` | Current composite time scale |

### Time scale effects

`AddTimeScaleEffect(duration, scale, affectSound, key, blendIn, blendOut)` adds a `TimeScaleEffect` to a stack. Each frame `GetFinalTimeScale()` evaluates all active effects, applying blend-in and blend-out curves, and composites them multiplicatively. `GetSoundFinalTimeScale()` applies only sound-affecting effects. The `key` field allows cancelling or replacing named effects.

### Frame rate limiter

`LimitFrameRate()` uses a high-resolution SDL counter to busy-wait until `TargetFrameTime` is reached, providing accurate frame pacing without OS sleep jitter.

---

## 30. Thread Pool

**Files:** `ThreadPool.h`, `ThreadPool.cpp`

A work-stealing-style thread pool used for async entity updates, physics jobs, and batch processing.

### API

- `Start(numThreads)` — spawns N worker threads.
- `QueueJob(fn)` / `QueueJobs(vector<fn>)` — push one or many jobs atomically.
- `WaitForFinish()` — block until all queued jobs complete.
- `Stop()` — drain queue and join all threads.
- `Enqueue<F>(f, args...)` — returns `std::future<result>` for typed async work.

### Thread counts

Static helpers determine optimal thread counts:
- `GetNumThreadsForPhysics()` — Jolt job pool.
- `GetNumThreadsForAsyncUpdate()` — entity `AsyncUpdate` pool.
- `GetNumThreadsForThreadPool()` — main background pool.

### Emscripten compatibility

When `DISABLE_THREADPOOL` is defined (no pthreads, or Emscripten without shared memory), all job queuing runs synchronously on the calling thread. `ThreadPool::Supported()` lets callers check availability.

---

## 31. Loading Screen

**Files:** `LoadingScreen/LoadingScreenSystem.h/.cpp`, `LoadingScreen/UiLoadingScreenBase.h`

A progress-based loading screen overlay that renders during level loads and time-skip simulations.

`UiLoadingScreenBase` is an abstract `UiCanvas` subclass. Games override it to provide a custom loading screen widget tree. `SetLoadingCanvas(canvas)` registers the active implementation.

`LoadingScreenSystem::Update(progress)` sets `LoadingScreenSystem::Progress` [0..1] and calls `Draw()` immediately — this renders to the screen synchronously by binding the loading screen's `UiViewport`, calling `Viewport.Draw()`, and swapping the buffer. This allows progress updates during what would otherwise be a frozen frame.

---

## 32. Video System

**Files:** `Video/Video.h`, `Video/Video.cpp`, `Video/VideoInstance.h/.cpp`, `Video/pl_mpeg.h`

MPEG-1 video playback using the bundled **pl_mpeg** single-header library.

`Video::FromMemory(bytes, length)` / `FromVector(vec)` loads an MPEG-1 stream from a memory buffer (typically pre-loaded via `FileSystemEngine`).

`GetFrameAtTime(seconds, out_rgb, out_w, out_h, timeout)` decodes the frame at the requested timestamp. The YCbCr planes are converted to RGB24 using the inline `yuv_to_rgb_pixel` function. A timeout parameter guards against stalls during seek.

`VideoInstance` wraps a `Video*` and manages a per-frame OpenGL texture upload. It exposes `Play`, `Pause`, `Stop`, `SetLooping`, and an `OnFrameReady` callback. `UiVideo` uses `VideoInstance` to drive an in-UI video element.

---

## 33. HTTP Client

**Files:** `http/http_client.h`, `http/http_client_httplib.cpp`, `http/http_client_emscripten.cpp`

A platform-abstracted async HTTP client with two backends:
- **Native** — uses the `yhirose/cpp-httplib` header-only library.
- **Emscripten** — uses `emscripten_fetch` for browser-compatible requests.

### API

```cpp
RequestId id = http_client::start_request({ Method::GET, "https://..." });
// each frame:
if (http_client::is_done(id)) {
    Response r;
    http_client::get_response(id, r);
    // r.status_code, r.body, r.headers
}
```

Requests are fire-and-forget: the caller polls `is_done()` and fetches the response when ready. `cancel_request(id)` aborts a pending request. Methods supported: GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS.

---

## 34. Resource Statistics

**Files:** `Profiling/ResourceStatistics.hpp`

A singleton `ResourceStatistics` that tracks all GPU resource allocations and renders a breakdown table via **ImGui**.

### Registration

`registerResource(type, id, sizeBytes, name)` adds or updates a resource entry. `unregisterResource(type, id)` removes it. Thread-safe via `std::atomic` counters.

Resource types tracked: `Texture`, `TextureCube`, `RenderTexture`, `VertexBuffer`, `IndexBuffer`, `InstanceBuffer`.

### ImGui display

`renderImGui()` shows:
- Total memory by type (formatted as KB/MB).
- Per-resource detail table with name, size, and age.
- Overall GPU memory total.

Displayed when `DebugUiEnabled && Paused` in the main render loop.

---

## 35. LevelObject Factory

**Files:** `LevelObjectFactory.h`, `LevelObjectFactory.cpp`, `RegisterLevelObject.h`

A registry-based entity factory. Maps `string technicalName → CreateEntityFn` (function pointer returning `Entity*`).

`registerDefaults()` registers all built-in entity types: brushes, lights, spawners, sound players, transform points, BSP entities, etc.

Game code registers custom types with the macro:
```cpp
REGISTER_ENTITY_INLINE("my_npc", MyNpcClass);
// expands to: LevelObjectFactory::instance().registerType("my_npc", []()->Entity*{ return new MyNpcClass(); });
```

`create(name)` returns `nullptr` if the type is unknown, allowing graceful fallback. `isRegistered(name)` allows preload checks.

---

## 36. Compression / Zip VFS

**Files:** `Compression/miniz.h/.cpp`, `Compression/zip/zip.h/.cpp`

Bundled **miniz** provides zlib-compatible deflate/inflate. The `zip` wrapper adds standard ZIP archive read/write on top. `ZipVFS` in the file system layer uses these to mount `.zip` files as virtual directories.

---

## 37. Helper Utilities

### MathHelper (`MathHelper.hpp`)
- `GetRotationMatrix(yawPitchRoll)` — Euler angles to mat4.
- `FindLookAtRotation(from, to)` — returns yaw/pitch Euler angles.
- `ToYawPitchRoll(quat)` — quaternion to Euler.
- `Transform` struct — position/rotation/scale with `ToMatrix()` / `ToMatrixEuler()` / `DecomposeMatrix()`.
- Lerp, Clamp, Remap, and other scalar/vector utilities.

### RandomHelper (`RandomHelper.h/.cpp`)
Seeded pseudo-random float/int generators with range clamping.

### Logger (`Logger.hpp`)
Static `Logger::Log(string)` with timestamp; routes to `Console::AddLog` and `printf`.

### Delay (`Delay.hpp`)
A timer struct used throughout the engine. `Wait()` returns false when the countdown expires. `AddDelay(seconds)` resets it. Used for `destroyDelay`, `DebugDrawCommand` lifetimes, and cooldown decorators in behaviour trees.

### BoundingBox / BoundingSphere (`BoundingBox.hpp`, `BoundingSphere.hpp`)
AABB and sphere structs used for frustum culling tests.

### JsonHelper / JsonMacros (`Helpers/JsonHelper.hpp`, `Helpers/JsonMacros.hpp`)
Wrappers around nlohmann/json for engine-specific types (vec3, quat, etc.) and `NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT` macro helpers.

### StringHelper (`Helpers/StringHelper.h`)
Split, trim, toLower, and other string utilities.

### CSVParser (`Helpers/CSVParser.h/.cpp`)
Lightweight CSV reader for data tables (weapon stats, localization, etc.).

### ByteCompressor (`Helpers/ByteCompressor.h/.cpp`)
LZ4-based byte array compression/decompression used by the navigation system tile cache.

### hashed_string (`utility/hashed_string.hpp`)
A FNV-1a compile-time and runtime string hash wrapper. Stored in entity ID/name maps for O(1) average lookup. Allows `string_view`-based comparisons without heap allocation.

### PhysicsConverter (`PhysicsConverter.h`)
Inline conversion functions between GLM and JoltPhysics vector/quaternion types (`ToPhysics(vec3)`, `FromPhysics(RVec3)`, etc.).

---

*Last analyzed: source/Engine commit as of Engine.zip upload.*

---

## 38. Model Loading (ModelLoader / Assimp pipeline)

**Files:** `model.hpp`, `model.cpp`, `skinned_model.hpp`, `skinned_model.cpp`, `utils.hpp`, `utils.cpp`

The engine uses a templated `ModelLoader<mesh_t>` class (namespace `roj`) with two explicit specializations: `ModelLoader<Mesh>` for static meshes and `ModelLoader<SkinnedMesh>` for animated meshes.

### Template structure

```cpp
template<typename mesh_t>
class ModelLoader {
public:
    using model_t = ModelType<mesh_t>::type;   // vector<Mesh> or SkinnedModel
    bool SkipVisual = false;
    bool load(const string& path);
    model_t& get();
    const string& getInfoLog();
};
```

`ModelType<Mesh>::type = std::vector<Mesh>`. `ModelType<SkinnedMesh>::type = SkinnedModel`.

### Static mesh loading (`ModelLoader<Mesh>::load`)

1. Calls `m_import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace)`.
2. Recursively walks `processNode` → `processMesh` for each `aiMesh`.
3. `processMesh` extracts vertices into `std::vector<VertexData>` (position, normal, UV, tangent, bitangent) and indices.
4. Creates `VertexBuffer(vertices, VertexData::Declaration())` (GL_STATIC_DRAW).
5. After all meshes are processed, creates `VertexArrayObject(*mesh.vertexBuffer, *mesh.indexBuffer)` for each.
6. Index buffer creation is **absent** in `processMesh` for the static path — indices are assembled but the `IndexBuffer` is not constructed (this is a known gap in the static loader; the skinned loader is the production path).

### Skinned mesh loading (`ModelLoader<SkinnedMesh>::load`)

Reads file via `FileSystemEngine::ReadFileBinary` (works with both physical filesystem and Zip VFS) and passes memory to `m_import.ReadFileFromMemory` with flags:
```
aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights |
aiProcess_JoinIdenticalVertices | aiProcess_GlobalScale
```
Caches the `aiScene*` and path (`m_lastLoadedPath`, `m_cachedScene`) — reloading the same path reuses the cached scene without re-parsing.

**Mesh extraction** (`processMesh<SkinnedMesh>`):

For each vertex: position (`× LoaderGlobalParams::Size`), normal, UV (first channel), tangent, bitangent, vertex color (first non-null color channel). Blend indices and weights are zero-initialized then filled by `extractBoneVertexData`.

`extractBoneVertexData` iterates `aiMesh::mBones`: registers unknown bones into `model.boneInfoMap` with a new sequential id and the Assimp offset matrix (converted via `utils::assimp::toGlmMat4`). Then distributes bone weights into vertex `BlendWeights[0..3]` / `BlendIndices[0..3]` (first free slot, up to `MAX_BONE_INFLUENCE = 4`).

Creates `VertexBuffer` and `IndexBuffer`, records them in `ResourceStatistics` with the path + mesh name. After all meshes, creates `VertexArrayObject` for each.

**Animation extraction** (`extractAnimations`):

For each `aiAnimation`: creates `roj::Animation`, calls `extractBoneNode` (DFS copy of the full node hierarchy into `BoneNode` tree), sets `ticksPerSec`, `duration`, `frameTime`. For each channel, copies position/rotation/scale keyframes (time + value) into `FrameBoneTransform`. Bones referenced by animation channels but absent from `boneInfoMap` are added with globally-computed offset matrices.

**Post-processing:**

- `extractBoneNodeAndBuildMap` builds `model.boneNodesMap` and `model.parentMap` for runtime parent-chain lookups.
- Computes `BoundingSphere::FromPoints` and `BoundingBox::FromPoints` over all vertex positions. For rigged models the sphere radius is enlarged (`× 2.5`) and offset scaled (`× 1.5`) to account for deformation range.
- Submeshes sorted so transparent ones come last (preserves correct render order).
- Creates an `roj::Animator` temporarily to bake precomputed frames at `BAKED_FRAME_RATE = 30` fps for each animation.
- Iterates `aiScene::mTextures` and calls `LoadTextureFromScene` for each embedded texture.
- Calls `mesh.ProcessDefaultTextures()` to resolve textures from the asset registry.

### Embedded texture loading (`LoadTextureFromScene`)

Two paths based on `texture->mHeight`:
- **Compressed** (mHeight == 0): raw bytes at `pcData`, size = `mWidth`. Constructs `Texture(data, size)` (stb_image decompresses). Registers in `ResourceStatistics` and `AssetRegistry` under `lastLoadedPath + "/" + filename.ext`.
- **Uncompressed** (raw BGRA32): constructs `Texture(data, width, height)`. Registers similarly.

### Assimp utility conversions (`utils::assimp`)

```cpp
glm::mat4 toGlmMat4(aiMatrix4x4)  // transposes during copy (Assimp row-major → GLM column-major)
glm::vec3 toGlmVec3(aiVector3D)
glm::quat toGlmQuat(aiQuaternion)
```

### Texture reference resolution

`getMeshTextures` reads `aiTextureType_BASE_COLOR`, `EMISSIVE`, `EMISSION_COLOR`, `SPECULAR`, `NORMALS`, `HEIGHT`. For each, it extracts the embedded texture index from the `*N` string, retrieves filename and format hint from `scene->mTextures[id]`, and builds a `MeshTexture { type, filename.ext }`. Actual GPU upload happens in `LoadTextureFromScene` or lazily in `SkinnedMesh::ProcessDefaultTextures`.

---

## 39. GPU Buffer Abstractions

**File:** `VertexData.h`

All GPU buffer types extend `EObject` (giving them an owner entity context).

### VertexDeclaration

Describes the attribute layout for a vertex or instance buffer as a list of `Element` structs:

```cpp
struct Element {
    GLuint index;         // attribute location
    GLint componentCount;
    GLenum type;          // GL_FLOAT, GL_INT, GL_UNSIGNED_INT, etc.
    GLboolean normalized;
    GLsizei stride;
    const void* offset;   // byte offset within struct
    GLuint divisor;       // 0 = per-vertex, 1 = per-instance
};
```

### VertexBuffer

```cpp
template<typename T>
VertexBuffer(const vector<T>& vertices, const VertexDeclaration& decl, GLenum usage = GL_STATIC_DRAW);
```

Constructor: `glGenBuffers` + `glBindBuffer(GL_ARRAY_BUFFER)` + `glBufferData`. Registers with `ResourceStatistics`.

`UpdateData(data, offset, usage)` — if vertex count changes, calls `glBufferData` to reallocate; otherwise `glBufferSubData` for a partial update.

`Bind()` → `glBindBuffer(GL_ARRAY_BUFFER, m_id)`. Static `Unbind()` → binds 0.

Destructor: `glDeleteBuffers` + unregisters from `ResourceStatistics`.

### IndexBuffer

Stores `uint32_t` indices in `GL_ELEMENT_ARRAY_BUFFER`. Same create/bind/update/delete pattern. `UpdateData` always reallocates (`glBufferData` with `GL_STREAM_DRAW` by default).

### VertexArrayObject (desktop, non-GLES2)

```cpp
VertexArrayObject(VertexBuffer& vb, IndexBuffer& ib, VertexBuffer* instanceBuf = nullptr);
```

Constructor: `glGenVertexArrays` + `glBindVertexArray`. Binds `vb`, `ib`, then calls `glEnableVertexAttribArray` + `glVertexAttribIPointer` (for integer types without normalization) or `glVertexAttribPointer` + `glVertexAttribDivisor` for each element. Optional instance buffer is set up with `divisor > 0`. Finishes with `glBindVertexArray(0)`.

`Bind()` → `glBindVertexArray`. Static `Unbind()` → `glBindVertexArray(0)`. Destructor: `glDeleteVertexArrays`.

`IsInstanced()` → `instanceBuffer != nullptr`. `GetInstanceCount()` → `instanceBuffer->GetVertexCount()`.

### VertexArrayObject (GLES2 emulation, `#ifdef GL_ES_2`)

No real VAO. `Bind()` replays the full attribute setup manually: binds vb + ib, calls `glEnableVertexAttribArray` + `glVertexAttribPointer` for each element (integer types are cast to `GL_FLOAT`). Tracks enabled attribute indices in `m_enabledAttributes`. Static `Unbind()` calls `glDisableVertexAttribArray` for each tracked index, then unbinds vb + ib.

### VertexData struct (vertex layout)

```
attr 0: vec3  Position
attr 1: vec3  Normal
attr 2: vec2  TextureCoordinate
attr 3: vec3  Tangent
attr 4: vec3  BiTangent
attr 5: int[4] BlendIndices  (GL_INT, not normalized)
attr 6: vec4  BlendWeights
attr 7: vec3  SmoothNormal
attr 8: vec4  Color
attr 9: vec2  ShadowMapCoords
```

`VertexData::Declaration()` returns the matching `VertexDeclaration`.

### InstanceData struct

```
attr 10–13: mat4 ModelMatrix  (4 × vec4, divisor = 1)
attr 14:    vec4 Color        (divisor = 1)
```

Used for hardware instancing — bind an `InstanceData` `VertexBuffer` as the instance buffer of a VAO to draw many transforms in one draw call.

---

## 40. Texture System

**Files:** `Texture.hpp`, `TextureCube.hpp`

### Texture class

Single 2D OpenGL texture. All constructors ultimately call `setupTexture(width, height, format, pixels, generateMipmaps)`.

**Constructors:**
```cpp
Texture(const string& filename, bool generateMipmaps = true);          // file path → FileSystem → stb_image
Texture(const uchar* data, size_t size, bool generateMipmaps = true);  // compressed blob (PNG/JPEG) → stb_image
Texture(const uchar* data, int w, int h, GLenum format, bool generateMipmaps = true); // raw pixels
```

**`setupTexture`** (all OpenGL calls):
1. `glGenTextures(1, &textureID)` + `glBindTexture(GL_TEXTURE_2D, textureID)`.
2. `glPixelStorei(GL_UNPACK_ALIGNMENT, 1)`.
3. `glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, pixels)`.
4. Checks `glGetError()` immediately after upload (logs any ANGLE/WebGL errors).
5. `glGenerateMipmap(GL_TEXTURE_2D)` if mipmaps enabled and texture is not NPOT.
6. Wrap: `GL_REPEAT` for normal textures, `GL_CLAMP_TO_EDGE` for NPOT.
7. Filter: `GL_LINEAR_MIPMAP_LINEAR` if mipmaps, else `GL_NEAREST` (min) and `GL_LINEAR` (mag).
8. Anisotropic filtering: queries `GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT`, sets `GL_TEXTURE_MAX_ANISOTROPY_EXT` to maximum.
9. Registers texture size (+ ~⅓ for mips) and format string in `ResourceStatistics`.

**`loadFromFile`**: reads binary data via `FileSystemEngine::ReadFileBinary`, calls `loadFromMemoryCompressed`, then sets a human-readable filename in `ResourceStatistics`.

**`loadFromMemoryCompressed`**: `stbi_load_from_memory(data, size, &w, &h, &channels, 4)` (forces RGBA), uploads, frees with `stbi_image_free`.

**Destructor**: `glDeleteTextures(1, &textureID)` + unregisters from `ResourceStatistics`.

`bind()` → `glBindTexture(GL_TEXTURE_2D, textureID)`. `getID()` returns `GLuint`.

### CubemapTexture class

Six-face OpenGL cubemap.

**Constructors:**
```cpp
CubemapTexture(const vector<string>& faces, bool generateMipmaps = false); // explicit 6 paths
CubemapTexture(const string& base, bool generateMipmaps = false);          // derives 6 paths from base
```

The single-path constructor generates face names by replacing the extension dot with `_lf.`, `_rt.`, `_up.`, `_dn.`, `_ft.`, `_bk.`.

**`loadFromFiles`**:
1. `glGenTextures` + `glBindTexture(GL_TEXTURE_CUBE_MAP)` + `glPixelStorei(GL_UNPACK_ALIGNMENT, 1)`.
2. For each of 6 faces: reads binary via `FileSystemEngine`, `stbi_load_from_memory(..., STBI_rgb_alpha)`.
3. Faces +Y (index 2) and −Y (index 3) are rotated 90° CW and CCW respectively via `rotate90_rgba` (manual pixel shuffle, allocates with `malloc`, caller frees with `stbi_image_free`). Width and height are swapped after rotation.
4. `glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, ...)` for each face.
5. Sets wrap `GL_CLAMP_TO_EDGE` on S/T/R, filter `GL_LINEAR` (mag), `GL_LINEAR_MIPMAP_LINEAR` or `GL_LINEAR` (min).
6. `glGenerateMipmap(GL_TEXTURE_CUBE_MAP)` if requested.
7. Derives a common prefix from all face paths as the resource name.
8. Registers total size in `ResourceStatistics`.

**Destructor**: `glDeleteTextures` + unregisters `ResourceType::TextureCube`.

`bind()` → `glBindTexture(GL_TEXTURE_CUBE_MAP, textureID)`.

---

## 41. Platform Startup, SDL2 & OpenGL Context

**Files:** `main.cpp`, `PlatformMains/windows_main.hpp`, `PlatformMains/linux_main.hpp`, `PlatformMains/emscripten_main.hpp`, `PlatformMains/PlatformWindowData.h`

`main.cpp` selects the correct platform header at compile time via `#if defined(__EMSCRIPTEN__) / _WIN32 / __linux__`. Each header contains a complete `main()` function.

### Common startup sequence

All three platforms share this pattern:

```
SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK)
SDL_GL_SetAttribute(context profile, major/minor version, depth 24, MSAA 2×)
SDL_CreateWindow("Image", centered, 800×600, SDL_WINDOW_OPENGL | SHOWN | HIGHDPI)
TTF_Init()
SDL_GL_CreateContext(window)
[glewInit() on desktop]
InitImGui()
SDL_SetRelativeMouseMode(SDL_TRUE)
glClearColor(0,0,0,1)
SDL_GL_SetSwapInterval(0)   // vsync off
new EngineMain(window)
engine->Init(args)
[run loop]
SDL_GL_DeleteContext / SDL_DestroyWindow / SDL_Quit
```

### Windows (`windows_main.hpp`)

- OpenGL **Core 4.5** (or ES 3.0 if `GL_ES_PROFILE` defined).
- MSAA: `SDL_GL_MULTISAMPLEBUFFERS=1, MULTISAMPLESAMPLES=2`.
- Debug builds: `SDL_GL_CONTEXT_DEBUG_FLAG` + `glEnable(GL_DEBUG_OUTPUT) + glDebugMessageCallback(openglDebugCallback)`. Debug callback filters by `g_DebugSeverityLevel` (0=all, 1=warn+err, 2=err only), logs to `std::cerr`.
- Mouse delta from **DirectInput** (`dinput8.lib`): `InitDirectInput` retrieves HWND via `SDL_GetWindowWMInfo`, creates `IDirectInput8` + `IDirectInputDevice8` (GUID_SysMouse, `c_dfDIMouse2`, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND). Per-frame: `GetDeviceState(sizeof(DIMOUSESTATE2))`, re-acquires on `DIERR_INPUTLOST`. Linux/web read mouse delta from `SDL_MOUSEMOTION::xrel/yrel` instead.
- **Crash handler**: `SetUnhandledExceptionFilter(EngineUnhandledExceptionFilter)`. On crash: writes a timestamped `.dmp` minidump via `MiniDumpWriteDump`, symbolizes stack trace with `DbgHelp` (`SymFromAddr`, `SymGetLineFromAddrW64`), shows `MessageBoxW` with the full call stack and dump path.
- `NvOptimusEnablement = 0x00000001` export forces NVIDIA GPU selection on Optimus laptops.
- Working directory can be overridden with `-working_dir <path>` command line argument (`_chdir`).

### Linux (`linux_main.hpp`)

- OpenGL **Core 4.2**, otherwise identical to Windows but without DirectInput or crash handler.
- Mouse delta accumulated from `SDL_MOUSEMOTION` events inside the SDL event loop.

### Emscripten (`emscripten_main.hpp`)

- OpenGL **ES 3.0** (`SDL_GL_CONTEXT_PROFILE_ES`, major 3, minor 0).
- Sets `DISTRIBUTION` define (strips debug UI).
- No `glewInit` — GLES3 functions loaded by the browser.
- Main loop: `emscripten_set_main_loop(emscripten_render_loop, 0, 1)` (yields to browser).
- Mouse delta from `SDL_MOUSEMOTION::xrel/yrel`, scaled by `screenSizeDifferenceFactor` to compensate for canvas resolution differences.
- Two `emscripten_sleep(300)` calls before the loop to allow JS to deliver the correct initial canvas size.
- `asyncGameUpdate` disabled unless `__EMSCRIPTEN_PTHREADS__` is defined.
- Focus/blur hooks (currently commented out): `on_canvas_focus` → unpause, `on_canvas_blur` → pause.

### `gl.h` abstraction

```cpp
#ifndef __EMSCRIPTEN__
#include <GL/glew.h>          // desktop: GLEW provides all GL 4.x entry points
#include <SDL2/SDL_opengl.h>
#else
#include <SDL2/SDL_opengles2.h>
#include <GLES3/gl31.h>
#define glTexImage2DMultisample glTexStorage2DMultisample  // GLES3 doesn't have the MS variant
#define GL_ES_PROFILE           // triggers VertexArrayObject GLES2 emulation path
#endif
```

All engine files include `gl.h` and use standard OpenGL names; the `#define`s in the emscripten path remap any desktop-only calls to their ES equivalents.

---

## 42. ImGui Integration

**File:** `ImGuiEngineImpl.h`, `imgui/imgui_impl_opengl3.h`, `imgui/imgui_impl_sdl2.h`

Two inline functions wrap the per-frame ImGui lifecycle:

**`ImStartFrame()`** — called at the start of each frame (only when `DebugUiEnabled`):
1. Sets `ImGuiConfigFlags_DockingEnable`.
2. If `Input::LockCursor` is true, adds `ImGuiConfigFlags_NoMouse | NoKeyboard` (hides debug UI from gameplay); otherwise removes those flags.
3. `ImGui_ImplOpenGL3_NewFrame()` + `ImGui_ImplSDL2_NewFrame()` + `ImGui::NewFrame()`.
4. Creates a transparent fullscreen dockspace (`ImGui::DockSpaceOverViewport`) with zero-alpha window and docking-background colors for frameless editor layouts.

**`RenderImGui()`** — called after all other rendering:
1. `ImGui::Render()`.
2. `glUseProgram(0)` (ensures no stale shader binding interferes with ImGui's own shader).
3. `ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData())`.

ImGui is initialized in each platform main via:
```cpp
ImGui::CreateContext();
ImGui::StyleColorsDark();
ImGui_ImplSDL2_InitForOpenGL(window, glContext);
ImGui_ImplOpenGL3_Init();
```
SDL events are forwarded with `ImGui_ImplSDL2_ProcessEvent(&event)` in the platform event loops.

Font Awesome 6 Free Solid icon font (`FA6FreeSolidFontData.h`) is included on Windows for editor icon rendering.

---

## 43. MeshUtils

**Files:** `MeshUtils.hpp`, `MeshUtils.cpp`

Static utility class for CPU-side mesh operations. Works with two geometry types:
- `PositionVerticesIndices { vector<vec3> vertices; vector<uint32_t> indices; }`
- `VerticesIndices { vector<VertexData> vertices; vector<uint32_t> indices; }`

**`MergeMeshes(meshes)`** — concatenates multiple meshes into one by appending vertex arrays and offsetting each mesh's indices by the running vertex count. Available for both geometry types.

**`RemoveDegenerates(mesh, distanceThreshold, areaEpsilon)`** — cleans a position-only mesh:
1. Spatially sorts vertices by X, then uses a sweep to union-find weld vertices within `distanceThreshold`.
2. Computes merged vertex positions as the average of each weld group.
3. Remaps triangle indices; discards triangles with duplicate vertices or area < `areaEpsilon` (degenerate/sliver triangles). Returns a cleaned `PositionVerticesIndices`.

**`ExpandHorizontalTriangles(mesh, radius)`** — expands each horizontal triangle outward by `radius` (declared in header, implemented in `.cpp`). Used for navigation mesh obstacle inflation.

**`MergeCoplanarRegions`** — flood-fills coplanar adjacent triangle patches and re-triangulates each planar region (full implementation in `.cpp`). Used to reduce overdraw in BSP-derived geometry.

**`Edge` / `EdgeHash` / `EdgeEq`** — normalized undirected edge (a ≤ b always), hash and equality for use in `unordered_map<Edge, ...>` adjacency structures within the coplanar merge algorithm.

