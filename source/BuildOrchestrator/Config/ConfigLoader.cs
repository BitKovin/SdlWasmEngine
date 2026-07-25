using System.Text.Json;
using REngine.BuildOrchestrator.Models;

namespace REngine.BuildOrchestrator.Config;

public static class ConfigLoader
{
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        PropertyNameCaseInsensitive = true,
        ReadCommentHandling = JsonCommentHandling.Skip,
        AllowTrailingCommas = true,
        WriteIndented = true,
    };

    public static BuildConfig Load(string path)
    {
        if (!File.Exists(path))
        {
            throw new FileNotFoundException(
                $"Config file not found: {path}\nRun with --init to generate a starter config.", path);
        }

        var json = File.ReadAllText(path);
        var config = JsonSerializer.Deserialize<BuildConfig>(json, JsonOptions);

        if (config is null)
            throw new InvalidOperationException($"Config file '{path}' is empty or invalid.");

        Validate(config, path);
        return config;
    }

    public static void WriteDefault(string path)
    {
        if (File.Exists(path))
            throw new InvalidOperationException($"Refusing to overwrite existing file: {path}");

        var json = JsonSerializer.Serialize(CreateDefault(), JsonOptions);
        File.WriteAllText(path, json);
    }

    private static void Validate(BuildConfig config, string path)
    {
        if (config.Targets.Count == 0)
            throw new InvalidOperationException($"'{path}' defines no targets.");

        var names = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        var buildDirs = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

        foreach (var target in config.Targets)
        {
            if (string.IsNullOrWhiteSpace(target.Name))
                throw new InvalidOperationException("Every target needs a non-empty 'name'.");

            if (!names.Add(target.Name))
                throw new InvalidOperationException($"Duplicate target name: '{target.Name}'.");

            if (string.IsNullOrWhiteSpace(target.BuildDir))
                throw new InvalidOperationException($"Target '{target.Name}' needs a 'buildDir'.");

            if (!buildDirs.Add(target.BuildDir))
                throw new InvalidOperationException(
                    $"Target '{target.Name}' reuses buildDir '{target.BuildDir}' — " +
                    "each target needs its own build directory.");

            switch (target.Kind)
            {
                case TargetKind.Windows:
                    if (target.Windows is null || string.IsNullOrWhiteSpace(target.Windows.VcVarsAllPath))
                        throw new InvalidOperationException(
                            $"Target '{target.Name}' (Windows) needs a 'windows.vcVarsAllPath'.");
                    break;
                case TargetKind.XboxGdk:
                case TargetKind.WindowsGdk:
                    if (target.Gdk is null || string.IsNullOrWhiteSpace(target.Gdk.VarsScriptPath)
                                            || string.IsNullOrWhiteSpace(target.Gdk.Flavor))
                        throw new InvalidOperationException(
                            $"Target '{target.Name}' ({target.Kind}) needs 'gdk.varsScriptPath' and 'gdk.flavor'.");
                    break;
                case TargetKind.Linux:
                    if (target.Docker is null || string.IsNullOrWhiteSpace(target.Docker.Image))
                        throw new InvalidOperationException(
                            $"Target '{target.Name}' (Linux) needs a 'docker.image'.");
                    break;
                case TargetKind.Emscripten:
                    if (target.Emscripten is null || string.IsNullOrWhiteSpace(target.Emscripten.EmsdkEnvScript))
                        throw new InvalidOperationException(
                            $"Target '{target.Name}' (Emscripten) needs an 'emscripten.emsdkEnvScript'.");
                    break;
            }
        }
    }

    /// <summary>
    /// Starter config mirroring REngine_2.0's own documented toolchain/platform pairing
    /// (see CMakeLists.txt and cmake/toolchains/*.cmake). Xbox/Windows GDK ship disabled
    /// since they require a licensed GDK install most machines won't have.
    /// </summary>
    private static BuildConfig CreateDefault() => new()
    {
        RepoRoot = ".",
        GameName = "Game",
        LogDirectory = "BuildLogs",
        Parallel = false,
        MaxParallelism = 2,
        Targets = new List<BuildTarget>
        {
            new()
            {
                Name = "windows",
                Kind = TargetKind.Windows,
                Enabled = true,
                BuildDir = "build/windows",
                ToolchainFile = "cmake/toolchains/msvc.cmake",
                Generator = "Ninja",
                Windows = new WindowsOptions
                {
                    VcVarsAllPath = @"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat",
                    Arch = "x64",
                },
            },
            new()
            {
                Name = "linux",
                Kind = TargetKind.Linux,
                Enabled = true,
                BuildDir = "build/linux",
                ToolchainFile = "cmake/toolchains/clang.cmake",
                // ninja-build ships preinstalled in the sniper SDK (confirmed against its package
                // manifest) — meaningfully faster configure/generate/no-op-rebuild-check than Unix
                // Makefiles for a project with this many vendored sub-libraries.
                Generator = "Ninja",
                // sniper/sdk ships CMake 3.25, but openAL-soft (vendored under sourceLibraries)
                // uses the $<BUILD_LOCAL_INTERFACE:...> generator expression, which needs 3.26+.
                // Bootstrap a newer CMake at container start rather than relying on the image's own.
                // Pinned to the latest CMake 3.x release rather than 4.x: CMake 4.0 dropped support
                // for cmake_minimum_required() below 3.5, which risks breaking other older vendored
                // dependencies that haven't bumped their own minimum version.
                // Installed under Docker.CacheMountPath (a named volume, default /cache) rather than
                // /opt, and guarded by an existence check, so a fresh --rm container reuses the
                // already-downloaded CMake instead of re-fetching it on every single build.
                ExtraEnvCommands = new List<string>
                {
                    "CMAKE_VERSION=3.31.12",
                    "if [ ! -x /cache/cmake/bin/cmake ]; then",
                    "  echo 'Bootstrapping CMake '$CMAKE_VERSION' into cache volume (first run only)...'",
                    "  curl -fsSL \"https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz\" -o /tmp/cmake.tar.gz",
                    "  mkdir -p /cache/cmake",
                    "  tar -xzf /tmp/cmake.tar.gz -C /cache/cmake --strip-components=1",
                    "fi",
                    "export PATH=/cache/cmake/bin:$PATH",
                    "cmake --version",
                },
                Docker = new DockerOptions
                {
                    Image = "registry.gitlab.steamos.cloud/steamrt/sniper/sdk:latest",
                    ContainerRepoPath = "/repo",
                    CacheVolumeName = "rengine-linux-build-cache",
                    CacheMountPath = "/cache",
                },
            },
            new()
            {
                Name = "emscripten",
                Kind = TargetKind.Emscripten,
                Enabled = true,
                BuildDir = "build/emscripten",
                Generator = "Ninja",
                Emscripten = new EmscriptenOptions { EmsdkEnvScript = @"C:\emsdk\emsdk_env.bat" },
            },
            new()
            {
                Name = "xbox-gdk",
                Kind = TargetKind.XboxGdk,
                Enabled = false,
                BuildDir = "build/xbox_gdk",
                ToolchainFile = "cmake/toolchains/msvc.cmake",
                Generator = "Ninja",
                // GDK_SCARLETT defaults ON (Series X|S) in the repo; add "-DGDK_SCARLETT=OFF" for Xbox One.
                ExtraCMakeArgs = new List<string> { "-DXBOX_GDK=1" },
                // Best-effort default — confirm against your own installed GDK. The vars-script
                // path/flavor naming varies by GDK edition: older editions use "GamingXboxVS2022",
                // newer "New layout" editions use "GamingXboxGen8VS2022"/"GamingXboxGen9VS2022"
                // (Gen8 = Xbox One, Gen9 = Series X|S). Run the vars script with no argument, or
                // check the GDK's own Start-menu shortcuts, to see which your install accepts.
                Gdk = new GdkOptions
                {
                    VarsScriptPath = @"C:\Program Files (x86)\Microsoft GDK\Command Prompts\GamingXboxVars.cmd",
                    Flavor = "GamingXboxVS2022",
                },
            },
            new()
            {
                Name = "windows-gdk",
                Kind = TargetKind.WindowsGdk,
                Enabled = false,
                BuildDir = "build/windows_gdk",
                ToolchainFile = "cmake/toolchains/msvc.cmake",
                Generator = "Ninja",
                ExtraCMakeArgs = new List<string> { "-DWINDOWS_GDK=1" },
                Gdk = new GdkOptions
                {
                    VarsScriptPath = @"C:\Program Files (x86)\Microsoft GDK\Command Prompts\GamingDesktopVars.cmd",
                    Flavor = "GamingDesktopVS2022",
                },
            },
        },
    };
}
