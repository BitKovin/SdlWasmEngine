using System.Text.Json.Serialization;

namespace REngine.BuildOrchestrator.Models;

/// <summary>Root of build-targets.json — one file describes every platform build.</summary>
public sealed class BuildConfig
{
    /// <summary>Path to the REngine_2.0 repository root (the directory containing CMakeLists.txt),
    /// relative to the config file or absolute.</summary>
    public string RepoRoot { get; set; } = ".";

    /// <summary>Default value for -DGAME_NAME=... Overridable per-target.</summary>
    public string GameName { get; set; } = "Game";

    /// <summary>Directory (relative to RepoRoot) where per-target logs, generated scripts,
    /// and the summary.json report are written.</summary>
    public string LogDirectory { get; set; } = "BuildLogs";

    /// <summary>Run independent targets concurrently instead of one after another.</summary>
    public bool Parallel { get; set; } = false;

    /// <summary>Max number of targets built at the same time when Parallel is true.</summary>
    public int MaxParallelism { get; set; } = 2;

    /// <summary>Finalize GameData exactly once — via BuildScripts/RunGameDataFinalize.cmake —
    /// before any platform build starts, writing to the shared Build/&lt;GameName&gt;/GameData,
    /// and automatically pass -DSKIP_GAMEDATA_FINALIZE=1 to every target so none of them
    /// redundantly re-run the (potentially slow) finalize pipeline themselves. Requires `cmake`
    /// and a `python3`/`python` interpreter on the host PATH. Set false to fall back to the
    /// engine's own default: each target finalizes its own GameData independently.
    /// A specific target can still opt back out by adding "-DSKIP_GAMEDATA_FINALIZE=0" to its
    /// own ExtraCMakeArgs — later -D wins, so a per-target override always takes precedence.</summary>
    public bool FinalizeGameDataOnce { get; set; } = true;

    public List<BuildTarget> Targets { get; set; } = new();
}

public sealed class BuildTarget
{
    /// <summary>Unique, arbitrary identifier used in logs, --only, and the summary report.</summary>
    public string Name { get; set; } = "";

    [JsonConverter(typeof(JsonStringEnumConverter))]
    public TargetKind Kind { get; set; }

    public bool Enabled { get; set; } = true;

    /// <summary>CMake build directory, relative to RepoRoot. Must be unique per target.</summary>
    public string BuildDir { get; set; } = "";

    /// <summary>CMake toolchain file, relative to RepoRoot.
    /// Leave null for Emscripten — emcmake sets the compiler itself.</summary>
    public string? ToolchainFile { get; set; }

    public string Generator { get; set; } = "Ninja";

    /// <summary>Overrides BuildConfig.GameName for this target only.</summary>
    public string? GameName { get; set; }

    /// <summary>Extra "-D..." arguments appended to the cmake configure call
    /// (e.g. "-DXBOX_GDK=1", "-DGDK_SCARLETT=OFF", "-DENABLE_PTHREADS=ON").</summary>
    public List<string> ExtraCMakeArgs { get; set; } = new();

    /// <summary>Extra raw lines inserted into the generated script before the cmake calls —
    /// an escape hatch for anything environment-specific this tool doesn't model directly.</summary>
    public List<string> ExtraEnvCommands { get; set; } = new();

    /// <summary>Passed to `cmake --build --config` and `-DCMAKE_BUILD_TYPE=`.
    /// Harmless no-op for single-config generators like Ninja beyond the build-type cache var.</summary>
    public string Configuration { get; set; } = "Release";

    public WindowsOptions? Windows { get; set; }
    public DockerOptions? Docker { get; set; }
    public EmscriptenOptions? Emscripten { get; set; }
    public GdkOptions? Gdk { get; set; }
}

public sealed class WindowsOptions
{
    /// <summary>Full path to vcvarsall.bat from your Visual Studio install. Used for plain
    /// Windows builds only — Xbox GDK / Windows GDK targets use GdkOptions instead, since
    /// their own vars script sets up both the GDK environment and the matching VS/MSVC
    /// compiler environment in one call (no separate vcvarsall.bat needed).</summary>
    public string VcVarsAllPath { get; set; } = "";

    /// <summary>Architecture argument passed to vcvarsall.bat (e.g. x64, amd64).</summary>
    public string Arch { get; set; } = "x64";
}

public sealed class GdkOptions
{
    /// <summary>Path to the GDK "vars" script: GamingDesktopVars.cmd for Windows GDK, or
    /// GamingXboxVars.cmd for Xbox GDK — normally under
    /// "...\Microsoft GDK\Command Prompts\" (or under an extracted GDK's own
    /// "Command Prompts" folder if you're using an unpacked/portable GDK instead of the
    /// installed one). This one script sets up BOTH the GDK environment and the matching
    /// VS/MSVC compiler environment — it replaces vcvarsall.bat entirely for these targets.</summary>
    public string VarsScriptPath { get; set; } = "";

    /// <summary>First argument to the vars script, selecting platform + VS version — e.g.
    /// "GamingDesktopVS2022" for Windows GDK, or "GamingXboxVS2022" / "GamingXboxGen9VS2022"
    /// for Xbox GDK (exact accepted values depend on your installed GDK edition — run the
    /// vars script with no argument, or check the GDK's own Start-menu shortcuts, to confirm
    /// which ones yours accepts).</summary>
    public string Flavor { get; set; } = "";
}

public sealed class DockerOptions
{
    /// <summary>OCI image the Linux build runs inside. Defaults to the Steam Linux Runtime 3.0
    /// "sniper" SDK — Debian 11 "bullseye", ships cmake/make/clang/gcc preinstalled, and building
    /// against it gives you the same glibc/library baseline Valve recommends for shipping
    /// broadly-compatible native Linux builds on Steam.</summary>
    public string Image { get; set; } = "registry.gitlab.steamos.cloud/steamrt/sniper/sdk:latest";

    /// <summary>Path inside the container that RepoRoot is bind-mounted to.</summary>
    public string ContainerRepoPath { get; set; } = "/repo";

    /// <summary>Extra raw arguments inserted into the `docker run` invocation
    /// (e.g. "--platform", "linux/amd64", or "--memory", "8g").</summary>
    public List<string> ExtraDockerArgs { get; set; } = new();

    /// <summary>Name of a Docker named volume mounted at CacheMountPath, persisted across
    /// runs even though each build uses a fresh `docker run --rm` container. Used by the
    /// default CMake-bootstrap ExtraEnvCommands so the downloaded CMake survives between
    /// builds instead of being re-fetched every single time. Set empty to disable the mount.</summary>
    public string CacheVolumeName { get; set; } = "rengine-linux-build-cache";

    /// <summary>Where CacheVolumeName is mounted inside the container.</summary>
    public string CacheMountPath { get; set; } = "/cache";
}

public sealed class EmscriptenOptions
{
    /// <summary>Full path to emsdk_env.bat from your emsdk checkout.</summary>
    public string EmsdkEnvScript { get; set; } = "";
}
