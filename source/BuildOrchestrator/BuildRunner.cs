using REngine.BuildOrchestrator.Models;

namespace REngine.BuildOrchestrator.Execution;

public sealed class TargetResult
{
    public required string Name { get; init; }
    public required bool Success { get; init; }
    public required TimeSpan Duration { get; init; }
    public required string LogFilePath { get; init; }
    public string? ErrorMessage { get; init; }
}

/// <summary>Runs one BuildTarget end-to-end: preflight checks, optional clean, script
/// generation, execution, and result reporting. One instance is reused across all targets
/// in a run so the repo/log paths are only resolved once.</summary>
public sealed class BuildRunner
{
    private readonly BuildConfig _config;
    private readonly string _repoRootFull;
    private readonly string _logRootFull;
    private readonly bool _dryRun;
    private readonly bool _clean;

    public BuildRunner(BuildConfig config, bool dryRun, bool clean)
    {
        _config = config;
        _dryRun = dryRun;
        _clean = clean;
        _repoRootFull = Path.GetFullPath(config.RepoRoot);
        _logRootFull = Path.Combine(_repoRootFull, config.LogDirectory);
    }

    public async Task<TargetResult> RunAsync(BuildTarget target, CancellationToken cancellationToken)
    {
        var buildDirFull = Path.GetFullPath(Path.Combine(_repoRootFull, target.BuildDir));
        var logFilePath = Path.Combine(_logRootFull, $"{target.Name}.log");
        var scratchDir = Path.Combine(_repoRootFull, ".build-orchestrator", target.Name);
        Directory.CreateDirectory(scratchDir);

        try
        {
            PreflightCheck(target);

            if (_clean && Directory.Exists(buildDirFull))
            {
                Console.WriteLine($"[{target.Name}] Cleaning {buildDirFull}");
                if (!_dryRun) Directory.Delete(buildDirFull, recursive: true);
            }

            string fileName;
            string arguments;

            if (target.Kind == TargetKind.Linux)
            {
                var distro = target.Wsl!.Distro;
                var repoWsl = await WslPathResolver.ToWslPathAsync(distro, _repoRootFull);
                var buildWsl = await WslPathResolver.ToWslPathAsync(distro, buildDirFull);
                var toolchainWsl = target.ToolchainFile is null
                    ? null
                    : await WslPathResolver.ToWslPathAsync(distro, Path.Combine(_repoRootFull, target.ToolchainFile));

                var script = ScriptBuilder.BuildLinuxScript(_config, target, repoWsl, buildWsl, toolchainWsl);
                var scriptPath = Path.Combine(scratchDir, "build.sh");
                await File.WriteAllTextAsync(scriptPath, script, cancellationToken);

                var scriptWslPath = await WslPathResolver.ToWslPathAsync(distro, scriptPath);
                fileName = "wsl.exe";
                arguments = $"-d {distro} -- bash \"{scriptWslPath}\"";
            }
            else
            {
                var toolchainFull = target.ToolchainFile is null
                    ? null
                    : Path.GetFullPath(Path.Combine(_repoRootFull, target.ToolchainFile));

                var script = ScriptBuilder.BuildWindowsScript(_config, target, _repoRootFull, buildDirFull, toolchainFull);
                var scriptPath = Path.Combine(scratchDir, "build.bat");
                await File.WriteAllTextAsync(scriptPath, script, cancellationToken);

                fileName = "cmd.exe";
                arguments = $"/d /c \"{scriptPath}\"";
            }

            if (_dryRun)
            {
                Console.WriteLine($"[{target.Name}] DRY RUN — would execute: {fileName} {arguments}");
                Console.WriteLine($"[{target.Name}] --- generated script ---");
                var scriptContent = await File.ReadAllTextAsync(
                    Path.Combine(scratchDir, target.Kind == TargetKind.Linux ? "build.sh" : "build.bat"),
                    cancellationToken);
                Console.WriteLine(scriptContent);
                return new TargetResult
                {
                    Name = target.Name, Success = true, Duration = TimeSpan.Zero, LogFilePath = logFilePath,
                };
            }

            var result = await ProcessRunner.RunAsync(
                fileName, arguments, _repoRootFull, logFilePath, target.Name, cancellationToken);

            return new TargetResult
            {
                Name = target.Name,
                Success = result.Success,
                Duration = result.Duration,
                LogFilePath = logFilePath,
                ErrorMessage = result.Success ? null : $"Exit code {result.ExitCode} — see {logFilePath}",
            };
        }
        catch (Exception ex)
        {
            return new TargetResult
            {
                Name = target.Name,
                Success = false,
                Duration = TimeSpan.Zero,
                LogFilePath = logFilePath,
                ErrorMessage = ex.Message,
            };
        }
    }

    private void PreflightCheck(BuildTarget target)
    {
        if (!Directory.Exists(_repoRootFull) || !File.Exists(Path.Combine(_repoRootFull, "CMakeLists.txt")))
            throw new InvalidOperationException(
                $"RepoRoot '{_repoRootFull}' doesn't look like the REngine_2.0 repo (no CMakeLists.txt found).");

        if (target.ToolchainFile is not null && !File.Exists(Path.Combine(_repoRootFull, target.ToolchainFile)))
            throw new InvalidOperationException($"Toolchain file not found: {target.ToolchainFile}");

        switch (target.Kind)
        {
            case TargetKind.Windows:
            case TargetKind.XboxGdk:
            case TargetKind.WindowsGdk:
                if (!File.Exists(target.Windows!.VcVarsAllPath))
                    throw new InvalidOperationException($"vcvarsall.bat not found: {target.Windows.VcVarsAllPath}");
                break;
            case TargetKind.Emscripten:
                if (!File.Exists(target.Emscripten!.EmsdkEnvScript))
                    throw new InvalidOperationException($"emsdk_env.bat not found: {target.Emscripten.EmsdkEnvScript}");
                break;
        }
    }
}
