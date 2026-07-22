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
    public WslOptions? Wsl { get; set; }
    public EmscriptenOptions? Emscripten { get; set; }
}

public sealed class WindowsOptions
{
    /// <summary>Full path to vcvarsall.bat from your Visual Studio install.
    /// Needed for Windows, Xbox GDK, and Windows GDK targets so cl.exe/link.exe are on PATH.</summary>
    public string VcVarsAllPath { get; set; } = "";

    /// <summary>Architecture argument passed to vcvarsall.bat (e.g. x64, amd64).</summary>
    public string Arch { get; set; } = "x64";
}

public sealed class WslOptions
{
    /// <summary>WSL distro name the Linux build runs in, e.g. "Ubuntu". Run `wsl -l -v` to list yours.</summary>
    public string Distro { get; set; } = "Ubuntu";
}

public sealed class EmscriptenOptions
{
    /// <summary>Full path to emsdk_env.bat from your emsdk checkout.</summary>
    public string EmsdkEnvScript { get; set; } = "";
}
