export declare const VERSION: '1.2.1';

/** Returns the dB/m transmission value that reduces energy to 0.1% after travelling metersToBlock meters through a material */
export declare function transmissionForThickness(metersToBlock: number): number;

export declare const MaterialType: {
    readonly Air: 0;
    readonly Brick: 1;
    readonly Cloth: 2;
    readonly Concrete: 3;
    readonly ConcretePolished: 4;
    readonly Dirt: 5;
    readonly Glass: 6;
    readonly Grass: 7;
    readonly Gravel: 8;
    readonly Gyprock: 9;
    readonly Ice: 10;
    readonly Leaf: 11;
    readonly Marble: 12;
    readonly Metal: 13;
    readonly Mud: 14;
    readonly Rock: 15;
    readonly Sand: 16;
    readonly Snow: 17;
    readonly Tile: 18;
    readonly Tree: 19;
    readonly Water: 20;
    readonly WoodIndoor: 21;
    readonly WoodOutdoor: 22;
};
export type MaterialType = typeof MaterialType[keyof typeof MaterialType];

export interface Vector { x: number; y: number; z: number; }

export interface Matrix {
    m11: number; m12: number; m13: number; m14: number;
    m21: number; m22: number; m23: number; m24: number;
    m31: number; m32: number; m33: number; m34: number;
    m41: number; m42: number; m43: number; m44: number;
}

export interface MatrixStatic {
    readonly Identity: Matrix;
    CreateTranslation(x: number, y: number, z: number): Matrix;
    CreateRotationX(rads: number): Matrix;
    CreateRotationY(rads: number): Matrix;
    CreateRotationZ(rads: number): Matrix;
    CreateScale(x: number, y?: number, z?: number): Matrix;
    CreateFromQuaternion(x: number, y: number, z: number, w: number): Matrix;
    Multiply(a: Matrix, b: Matrix): Matrix;
}

export interface LowPassFilter {
    readonly id: number;
    destroy(): void;
    /** Low-frequency gain in the range 0.0 to 1.0 */
    gainLF: number;
    /** High-frequency gain in the range 0.0 to 1.0 */
    gainHF: number;
}

export interface AirAbsorptionSettings {
    readonly id: number;
    destroy(): void;
    /** Relative humidity as a percentage (0–1) */
    humidity: number;
    /** Air temperature in degrees Celsius */
    temperature: number;
    /** Atmospheric pressure in Pascals */
    pressure: number;
}

export interface MaterialProperties {
    readonly id: number;
    destroy(): void;
    /** Percentage of low-frequency energy lost on each bounce (0.0 to 1.0) */
    absorptionLF: number;
    /** Percentage of high-frequency energy lost on each bounce (0.0 to 1.0) */
    absorptionHF: number;
    /** Scattering strength (0.0 to 1.0), where 0.0 has no scattering and 1.0 scatters up to 90 degrees from the reflected direction */
    scattering: number;
    /** Low-frequency transmission in dB/m (0.0 or greater) */
    transmissionLF: number;
    /** High-frequency transmission in dB/m (0.0 or greater) */
    transmissionHF: number;
    /** Percentage of low-frequency energy lost when a ray passes through a flat plane, disk, triangle, or non-watertight mesh (0.0 to 1.0) */
    planeTransmissionLF: number;
    /** Percentage of high-frequency energy lost when a ray passes through a flat plane, disk, triangle, or non-watertight mesh (0.0 to 1.0) */
    planeTransmissionHF: number;
}

export interface VisualisationData {
    readonly id: number;
    destroy(): void;
    /** The world-space position of the ray bounce */
    position: Vector;
    /** The world-space normal of the ray bounce. Guaranteed to be normalised */
    normal: Vector;
}

export interface TargetFilter {
    gainLF: number;
    gainHF: number;
}

export interface Trail {
    origin: Vector;
    direction: Vector;
    doReverb: boolean;
    doOcclusion: boolean;
    doPermeation: boolean;
    doAmbientOcclusion: boolean;
    doAmbientPermeation: boolean;
    hits: TrailHit[];
}

export interface TrailHit {
    position: Vector;
    accumulatedDistance: number;
    hasListenerLOS: boolean;
    outside: boolean;
    hasTargetLOS: boolean[];
}

export interface ProcessedReverb {
    /** Percentage of energy that returned back to the emitter */
    readonly returnedPercent: number;
    /** Percentage of energy that escaped outside the world edge */
    readonly outsidePercent: number;
    /** Low-frequency reverberation decay time measured from the echogram, in seconds */
    readonly measuredDecayTimeLF: number;
    /** High-frequency reverberation decay time measured from the echogram, in seconds */
    readonly measuredDecayTimeHF: number;
    /** Average roughness/scattering of all surfaces that all rays hit */
    readonly materialRoughness: number;
    /** Inverse of materialRoughness */
    readonly materialSmoothness: number;
    /** Average low-frequency absorption of all surfaces that all rays hit */
    readonly materialAbsorptionLF: number;
    /** Average high-frequency absorption of all surfaces that all rays hit */
    readonly materialAbsorptionHF: number;
    /** Average absorption of all surfaces that all rays hit */
    readonly materialAbsorption: number;
}

export interface EAXReverb {
    /** Delay before early reflections are heard, in seconds (0–0.3) */
    readonly reflectionsDelay: number;
    /** Modal density of the late reverberation (0–1) */
    readonly density: number;
    /** Echo diffusion of the late reverberation (0–1) */
    readonly diffusion: number;
    /** Low-frequency gain of the reverb (0–1) */
    readonly gainLF: number;
    /** High-frequency gain of the reverb (0–1) */
    readonly gainHF: number;
    /** Overall linear gain of the reverb (0–1) */
    readonly gain: number;
    /** Reverberation decay time at mid frequencies, in seconds (0.1–20) */
    readonly decayTime: number;
    /** Ratio of low-frequency decay time to mid-frequency decay time (0.1–2) */
    readonly decayLFRatio: number;
    /** Ratio of high-frequency decay time to mid-frequency decay time (0.1–2) */
    readonly decayHFRatio: number;
    /** Linear gain of early reflections (0–3.16) */
    readonly reflectionsGain: number;
    /** Linear gain of late reverberation (0–10) */
    readonly lateReverbGain: number;
    /** Delay of late reverberation relative to early reflections, in seconds (0–0.1) */
    readonly lateReverbDelay: number;
    /** Cycling time of the echo effect, in seconds (0.075–0.25) */
    readonly echoTime: number;
    /** Amplitude of the echo effect (0–1) */
    readonly echoDepth: number;
    /** Cycling time of the modulation effect, in seconds (0.04–4) */
    readonly modulationTime: number;
    /** Amplitude of the modulation effect (0–1) */
    readonly modulationDepth: number;
    /** Linear gain applied per meter of distance for high-frequency air absorption (0.892–1) */
    readonly airAbsorptionGainHF: number;
    /** Reference frequency for high-frequency decay ratio, in Hz (1000–20000) */
    readonly hfReference: number;
    /** Reference frequency for low-frequency decay ratio, in Hz (20–1000) */
    readonly lfReference: number;
    /** Rolloff factor for reflected sound sources (0–10) */
    readonly roomRolloffFactor: number;
    /** Whether to limit high-frequency decay time to the air absorption limit (0 or 1) */
    readonly decayHFLimit: number;
    /** Returns the effect slot gain for the given emitter. Returns -1 if the emitter does not have hasRelativeReverb set to true */
    relativeGains(emitter: Emitter): number;
    /** Returns the reverb direction for the given emitter. Returns null if the emitter does not have hasRelativeReverb set to true */
    relativeDirections(emitter: Emitter): Vector | null;
}

export interface Emitter {
    readonly id: number;
    destroy(): void;
    /** Invalidates the ray cache — all memory is cleaned up and all rays are re-cast */
    resetTrails(): void;
    /** Validates this emitter's configuration */
    validate(): void;

    /** Adds an emitter to this emitter's target list */
    addTarget(target: Emitter): void;
    /** Removes an emitter from this emitter's target list */
    removeTarget(target: Emitter): void;
    /** Returns whether the target emitter is in this emitter's target list */
    hasTarget(target: Emitter): boolean;
    /** Returns whether a target emitter has been raytraced. If true, it is safe to check the target's filter via getTargetFilter */
    hasRaytracedTarget(target: Emitter): boolean;
    /** Returns an object containing the LF and HF gain for a target emitter. Only access when hasRaytracedTarget is true */
    getTargetFilter(target: Emitter): TargetFilter;
    /** Returns the current ray trails for this emitter */
    getWasmTrails(prev?: Trail[]): Trail[];

    /** 3D position of this emitter */
    position: Vector;
    /** Sets the position of this emitter */
    setPosition(x: number, y: number, z: number): void;

    /** Whether this emitter is used as a reference point for calculating relative reverb gain and direction */
    hasRelativeReverb: boolean;
    /** True if this emitter hasn't cast its own rays yet */
    initialising: boolean;
    /** Whether to clamp this emitter's position to the world bounds */
    clampPosition: boolean;
    /** Whether to only scatter rays around their yaw (no pitch or Y component) */
    onlyScatterXZ: boolean;
    /** Whether this emitter's EAX is blended to produce grouped EAX. Set to false for listener emitters */
    affectsGroupedEAX: boolean;
    /** User-defined type for this emitter */
    type: number;
    /** Index indicating which grouped EAX reverb effect should be used for this emitter */
    groupedEAXIndex: number;
    /** How long (in milliseconds) the echogram records data for. Returning reverb rays after this period are ignored */
    maxEchogramTime: number;
    /** Length (in milliseconds) of each entry in the echogram. 10ms is recommended */
    echogramGranularity: number;
    /** Number of trails rebuilt from scratch each frame to prevent staleness when the listener moves */
    refreshRayCount: number;
    /** Number of emitters allocated ahead of time to prevent runtime allocations */
    reservedEmitterTargets: number;
    /** Number of reverb rays cast */
    reverbRayCount: number;
    /** Number of bounces per reverb ray */
    reverbBounceCount: number;
    /** Number of occlusion rays cast */
    occlusionRayCount: number;
    /** Number of bounces per occlusion ray */
    occlusionBounceCount: number;
    /** Number of permeation rays cast */
    permeationRayCount: number;
    /** Number of bounces per permeation ray */
    permeationBounceCount: number;
    /** Number of ambient occlusion rays cast */
    ambientOcclusionRayCount: number;
    /** Number of bounces per ambient occlusion ray */
    ambientOcclusionBounceCount: number;
    /** Number of ambient permeation rays cast */
    ambientPermeationRayCount: number;
    /** Number of bounces per ambient permeation ray */
    ambientPermeationBounceCount: number;
    /** Number of visualisation rays cast */
    visualisationRayCount: number;
    /** Number of times each visualisation ray bounces */
    visualisationBounceCount: number;
    /** How often (in milliseconds) to cast visualisation rays */
    visualisationUpdateFrequency: number;
    /** Lower bound of the relative reverb blend range */
    relativeReverbInnerThreshold: number;
    /** Upper bound of the relative reverb blend range */
    relativeReverbOuterThreshold: number;
    /** Amount of returning energy required for reverb to be at maximum volume */
    reverbEnergyCap: number;
    /** Threshold below which permeation rays are cancelled to prevent unnecessary traversal */
    minimumPermeationEnergy: number;
    /** A ray trail is re-created if an old bounce position is further than this distance from the new position */
    refreshDistanceThreshold: number;
    /** Percentage of ambient occlusion rays that reached the edge of the world (0.0 to 1.0) */
    outsidePercent: number;
    /** Percentage of occlusion energy required for the emitter to be at full volume */
    occlusionEnergyCap: number;
    /** Percentage of permeation energy required for the emitter to be at full volume */
    permeationEnergyCap: number;
    /** Percentage of occlusion energy required for ambient sounds to be at full volume */
    ambientOcclusionEnergyCap: number;
    /** Percentage of permeation energy required for ambient sounds to be at full volume */
    ambientPermeationEnergyCap: number;
    /** Low- and high-frequency gain for ambient sounds */
    readonly ambientFilter: { readonly gainLF: number; readonly gainHF: number };

    /** When defined, rays are cast from these positions rather than the emitter's position */
    set overridePositions(v: number[] | null);
    /** When defined, rays are cast in these directions rather than the default spherical distribution */
    set overrideRayDirections(v: number[] | null);
    /** Invoked after this emitter casts its rays for the first time */
    set onRaytracingComplete(fn: (() => void) | null);
    /** Invoked when another emitter raytraces this emitter for the first time */
    set onRaytracedByAnotherEmitter(fn: ((other: Emitter) => void) | null);
    /** Invoked with each bounce produced by visualisation rays. Do not store the array outside the callback */
    set visualisationCallback(fn: ((data: { position: Vector; normal: Vector }[]) => void) | null);

    /** Whether this emitter casts rays. False if all ray counts and/or bounce counts are 0 */
    readonly castsRays: boolean;
    /** Whether this emitter is within the world bounds. Emitters outside the world are not raytraced */
    readonly withinWorldBounds: boolean;
    /** True if both reverbRayCount and reverbBounceCount are greater than zero */
    readonly reverbEnabled: boolean;
    /** True if both occlusionRayCount and occlusionBounceCount are greater than zero */
    readonly occlusionEnabled: boolean;
    /** True if both permeationRayCount and permeationBounceCount are greater than zero */
    readonly permeationEnabled: boolean;
    /** True if both ambientOcclusionRayCount and ambientOcclusionBounceCount are greater than zero */
    readonly ambientOcclusionEnabled: boolean;
    /** True if both ambientPermeationRayCount and ambientPermeationBounceCount are greater than zero */
    readonly ambientPermeationEnabled: boolean;
    /** True if both visualisationRayCount and visualisationBounceCount are greater than zero */
    readonly visualisationEnabled: boolean;
    /** Number of trails this emitter will create */
    readonly trailCount: number;
    /** Number of bounces per trail */
    readonly trailBounceCount: number;
    /** Reverb data transformed into more usable parameters */
    readonly processedReverb: ProcessedReverb;
    /** EAX reverb properties for the listener, compatible with EAX reverb effects */
    readonly eax: EAXReverb;
}

export interface World {
    readonly id: number;
    destroy(): void;
    /** Updates the raytracing simulation. Call regularly to process results and submit new work */
    update(): void;
    /** Blocks until all background raytracing threads complete and handles the results */
    wait(): void;

    /** Adds a 3D primitive to the raytracing scene */
    addPrimitive(p: Primitive): void;
    /** Removes a 3D primitive from the raytracing scene */
    removePrimitive(p: Primitive): void;
    /** Adds an emitter to the raytracing scene */
    addEmitter(e: Emitter): void;
    /** Removes an emitter from the raytracing scene */
    removeEmitter(e: Emitter): void;
    /** Exports all world settings, materials, primitives, and emitters to a binary buffer */
    export(): Uint8Array;
    /** Imports world settings, materials, primitives, and emitters from a binary buffer produced by export(). Existing primitives and emitters are not removed before importing */
    import(data: Uint8Array | ArrayBuffer): { emitters: Emitter[]; primitives: Primitive[] };
    /** Returns true if a material exists in this world */
    hasMaterial(materialType: MaterialType): boolean;
    /** Adds or replaces a material in this world */
    addMaterial(materialType: MaterialType, properties: MaterialProperties): void;
    /** Returns the acoustic properties for a specific material */
    getMaterial(materialType: MaterialType): MaterialProperties;
    /** Returns the total number of threads used by this world */
    getThreadCount(): number;
    /** Returns the number of background threads used by this world */
    getBackgroundThreadCount(): number;

    /** Invoked after EAX reverb results are updated, before onRaytracingComplete callbacks */
    set onReverbUpdated(fn: (() => void) | null);

    /** Air absorption settings. Set to null to disable air absorption */
    airAbsorption: AirAbsorptionSettings | null;
    /** Whether emitters outside the world have 0 occlusion/permeation energy (true) or maximum energy (false) */
    emittersOutsideTheWorldAreMuffled: boolean;
    /** Whether the entire world is considered indoors or outdoors. Affects reverb ray behaviour at world bounds */
    worldIsIndoors: boolean;
    /** Whether to log memory allocation warnings */
    logMemoryAllocationWarnings: boolean;
    /** Whether to expose trail data for rendering */
    exposeTrails: boolean;
    /** Maximum number of grouped EAX reverb properties. Higher values increase accuracy but cost more */
    maximumGroupedEAXCount: number;
    /** Averaged EAX reverb objects for all emitters with affectsGroupedEAX set to true */
    readonly groupedEAX: EAXReverb[];
    /** Number of work items to split trails across for load balancing */
    workItemCount: number;
    /** Maximum number of threads that can run in parallel for this world */
    maximumConcurrencyLevel: number;
    /** Inverse speed of sound in seconds per meter. Defaults to 1/343. Affects reverb calculation */
    inverseSpeedOfSound: number;
    /** Low-frequency reference in Hz for air absorption, reverb, and material scattering */
    referenceFrequencyLF: number;
    /** High-frequency reference in Hz for air absorption, reverb, and material scattering */
    referenceFrequencyHF: number;
    /** Meters per world unit. Affects air absorption and reverb calculation */
    metersPerUnit: number;
    /** Epsilon value for ray offsets, world bounds clamping and line-of-sight tests. Defaults to 0.01 */
    epsilon: number;
    /** Minimum bounds of the world. Emitters and primitives outside these bounds are ignored */
    position: Vector;
    /** Size of the world. Emitters and primitives fully outside these bounds are ignored */
    size: Vector;
    /** Maximum bounds of the world (position + size) */
    maxBounds: Vector;

    /** True when raytracing has run at least once and reverb objects are safe to access */
    readonly reverbCalculated: boolean;
    /** When set to true, update() stops submitting work to background threads. Poll areThreadsRunning until false, then call destroy() */
    pendingShutdown: boolean;
    /** True while background raytracing threads are still running. Poll this after setting pendingShutdown, then call destroy() when false */
    readonly areThreadsRunning: boolean;
    /** Average time (in milliseconds) spent by update() on the main thread */
    readonly mainThreadTime: number;
    /** Average time (in milliseconds) spent in the preparation thread updating the BVH */
    readonly preparationTime: number;
    /** Average real-world elapsed time (in milliseconds) for raytracing threads to complete */
    readonly raytracingTime: number;
    /** Average time (in milliseconds) spent in the analysis thread calculating reverb properties */
    readonly analysisTime: number;
}

export interface Primitive {
    readonly id: number;
    destroy(): void;
    /** Controls the amount of energy lost when rays bounce off this primitive, permeate through it, and scatter off it */
    material: MaterialType;
}

export interface SpherePrimitive extends Primitive {
    /** Center position of the sphere in world space */
    center: Vector;
    /** Radius of the sphere */
    radius: number;
}

export interface CapsulePrimitive extends Primitive {
    /** Radius of the capsule */
    radius: number;
    /** Length of the cylindrical body between the hemispherical caps */
    length: number;
    /** This transform must only contain rotation and translation components, not scale */
    transform: Matrix;
}

export interface ConePrimitive extends Primitive {
    /** Radius of the cone base */
    radius: number;
    /** Height of the cone from base to apex */
    height: number;
    /** This transform must only contain rotation and translation components, not scale */
    transform: Matrix;
}

export interface CylinderPrimitive extends Primitive {
    /** Radius of the cylinder */
    radius: number;
    /** Length of the cylinder along its axis */
    length: number;
    /** This transform must only contain rotation and translation components, not scale */
    transform: Matrix;
}

export interface DiskPrimitive extends Primitive {
    /** Radius of the disk */
    radius: number;
    /** This transform must only contain rotation and translation components, not scale */
    transform: Matrix;
}

export interface HalfSpherePrimitive extends Primitive {
    /** Radius of the hemisphere */
    radius: number;
    /** This transform must only contain rotation and translation components, not scale */
    transform: Matrix;
}

export interface PlanePrimitive extends Primitive {
    /** Width of the plane */
    width: number;
    /** Height of the plane */
    height: number;
    /** This transform must only contain rotation and translation components, not scale */
    transform: Matrix;
}

export interface PrismPrimitive extends Primitive {
    /** Dimensions of the prism along each axis */
    size: Vector;
    /** This transform must only contain rotation and translation components, not scale */
    transform: Matrix;
}

export interface RectangularConePrimitive extends Primitive {
    /** Width of the rectangular base */
    width: number;
    /** Length of the rectangular base */
    length: number;
    /** Height of the cone from base to apex */
    height: number;
    /** This transform must only contain rotation and translation components, not scale */
    transform: Matrix;
}

export interface TrianglePrimitive extends Primitive {
    position0: Vector;
    position1: Vector;
    position2: Vector;
}

export interface TriangularConePrimitive extends Primitive {
    /** The radius of the triangular base */
    radius: number;
    /** The height of the cone */
    height: number;
    /** This transform must only contain rotation and translation components, not scale */
    transform: Matrix;
}

export interface TriangularPrismPrimitive extends Primitive {
    /** Circumradius of the triangular cross-section */
    radius: number;
    /** Length of the prism along its axis */
    length: number;
    /** This transform must only contain rotation and translation components, not scale */
    transform: Matrix;
}

export interface Mesh {
    readonly id: number;
    destroy(): void;
}

export interface MeshPrimitive extends Primitive {
    /** Whether this mesh supports 3D permeation */
    supports3DPermeation: boolean;
    /** Transform applied to the mesh vertices */
    transform: Matrix;
}

export interface EAXConstants {
    readonly DENSITY_MIN: number;
    readonly DENSITY_MAX: number;
    readonly DIFFUSION_MIN: number;
    readonly DIFFUSION_MAX: number;
    readonly GAIN_MIN: number;
    readonly GAIN_MAX: number;
    readonly GAIN_LF_MIN: number;
    readonly GAIN_LF_MAX: number;
    readonly GAIN_HF_MIN: number;
    readonly GAIN_HF_MAX: number;
    readonly DECAY_TIME_MIN: number;
    readonly DECAY_TIME_MAX: number;
    readonly DECAY_LF_RATIO_MIN: number;
    readonly DECAY_LF_RATIO_MAX: number;
    readonly DECAY_HF_RATIO_MIN: number;
    readonly DECAY_HF_RATIO_MAX: number;
    readonly REFLECTIONS_GAIN_MIN: number;
    readonly REFLECTIONS_GAIN_MAX: number;
    readonly REFLECTIONS_DELAY_MIN: number;
    readonly REFLECTIONS_DELAY_MAX: number;
    readonly LATE_REVERB_GAIN_MIN: number;
    readonly LATE_REVERB_GAIN_MAX: number;
    readonly LATE_REVERB_DELAY_MIN: number;
    readonly LATE_REVERB_DELAY_MAX: number;
    readonly ECHO_TIME_MIN: number;
    readonly ECHO_TIME_MAX: number;
    readonly ECHO_DEPTH_MIN: number;
    readonly ECHO_DEPTH_MAX: number;
    readonly MODULATION_TIME_MIN: number;
    readonly MODULATION_TIME_MAX: number;
    readonly MODULATION_DEPTH_MIN: number;
    readonly MODULATION_DEPTH_MAX: number;
    readonly AIR_ABSORPTION_GAIN_HF_MIN: number;
    readonly AIR_ABSORPTION_GAIN_HF_MAX: number;
    readonly LF_REFERENCE_MIN: number;
    readonly LF_REFERENCE_MAX: number;
    readonly HF_REFERENCE_MIN: number;
    readonly HF_REFERENCE_MAX: number;
    readonly ROOM_ROLLOFF_FACTOR_MIN: number;
    readonly ROOM_ROLLOFF_FACTOR_MAX: number;
    readonly DECAY_HF_LIMIT_MIN: number;
    readonly DECAY_HF_LIMIT_MAX: number;
    readonly REFLECTIONS_PAN_MIN: Vector;
    readonly REFLECTIONS_PAN_MAX: Vector;
    readonly LATE_REVERB_PAN_MIN: Vector;
    readonly LATE_REVERB_PAN_MAX: Vector;
}

export interface EAXUtils {
    getSimilarity(a: EAXReverb, b: EAXReverb): number;
    findBestMatch(target: EAXReverb, candidates: EAXReverb[]): { result: EAXReverb | null; index: number };
}

export interface VAudio {
    World_Create(): World;
    Emitter_Create(): Emitter;
    LowPassFilter_Create(): LowPassFilter;
    AirAbsorptionSettings_Create(): AirAbsorptionSettings;
    MaterialProperties_Create(): MaterialProperties;
    MaterialProperties_CreateWith(
        absorptionLF: number, absorptionHF: number,
        scattering: number,
        transmissionLF: number, transmissionHF: number,
        planeTransmissionLF: number, planeTransmissionHF: number
    ): MaterialProperties;
    VisualisationData_Create(): VisualisationData;

    Mesh_Create(vertices: number[], minX: number, minY: number, minZ: number, maxX: number, maxY: number, maxZ: number): Mesh;

    MeshPrimitive_Create(material: MaterialType, vertices: number[], minX: number, minY: number, minZ: number, maxX: number, maxY: number, maxZ: number, transform: Matrix): MeshPrimitive;
    MeshPrimitive_CreateFromMesh(material: MaterialType, mesh: Mesh, transform: Matrix): MeshPrimitive;

    SpherePrimitive_Create(): SpherePrimitive;
    CapsulePrimitive_Create(): CapsulePrimitive;
    ConePrimitive_Create(): ConePrimitive;
    CylinderPrimitive_Create(): CylinderPrimitive;
    DiskPrimitive_Create(): DiskPrimitive;
    HalfSpherePrimitive_Create(): HalfSpherePrimitive;
    PlanePrimitive_Create(): PlanePrimitive;
    PrismPrimitive_Create(): PrismPrimitive;
    RectangularConePrimitive_Create(): RectangularConePrimitive;
    TrianglePrimitive_Create(): TrianglePrimitive;
    TriangularConePrimitive_Create(): TriangularConePrimitive;
    TriangularPrismPrimitive_Create(): TriangularPrismPrimitive;

    Matrix: MatrixStatic;
    EAXConstants: EAXConstants;
    EAXUtils: EAXUtils;
}

export declare function wrapVA(raw: object): VAudio;

/**
 * Initialises the .NET runtime and returns a ready-to-use VAudio instance.
 *
 * @param dotnet       The dotnet bootstrapper imported from `./_framework/dotnet.js`
 * @param threadCount  Number of worker threads available for raytracing. Must match the
 *                     value you intend to use for `world.maximumConcurrencyLevel`.
 *                     Setting `maximumConcurrencyLevel` higher than this value will throw.
 */
export declare function initVA(dotnet: object, threadCount: number): Promise<VAudio>;
