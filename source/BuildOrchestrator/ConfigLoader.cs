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
                case TargetKind.XboxGdk:
                case TargetKind.WindowsGdk:
                    if (target.Windows is null || string.IsNullOrWhiteSpace(target.Windows.VcVarsAllPath))
                        throw new InvalidOperationException(
                            $"Target '{target.Name}' ({target.Kind}) needs a 'windows.vcVarsAllPath'.");
                    break;
                case TargetKind.Linux:
                    if (target.Wsl is null || string.IsNullOrWhiteSpace(target.Wsl.Distro))
                        throw new InvalidOperationException(
                            $"Target '{target.Name}' (Linux) needs a 'wsl.distro'.");
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
                Generator = "Ninja",
                Wsl = new WslOptions { Distro = "Ubuntu" },
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
                // GRDKLatest/GameDKLatest env vars are set globally by the GDK installer and are
                // inherited automatically — no extra activation needed beyond vcvarsall for cl.exe.
                ExtraCMakeArgs = new List<string> { "-DXBOX_GDK=1" },
                Windows = new WindowsOptions
                {
                    VcVarsAllPath = @"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat",
                    Arch = "amd64",
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
                Windows = new WindowsOptions
                {
                    VcVarsAllPath = @"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat",
                    Arch = "amd64",
                },
            },
        },
    };
}
