using System.Diagnostics;
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
            List<string> arguments;
            string scriptFileNameForDryRunPreview;

            if (target.Kind == TargetKind.Linux)
            {
                var docker = target.Docker!;
                var containerRoot = docker.ContainerRepoPath.TrimEnd('/');

                // RepoRoot is bind-mounted to containerRoot, so container-side paths are just
                // string concatenation with forward slashes — no runtime path translation needed
                // (unlike WSL's wslpath, Docker Desktop accepts the raw Windows host path directly
                // in the -v flag and does its own mapping).
                var buildDirContainer = $"{containerRoot}/{ToContainerRelativePath(target.BuildDir)}";
                var toolchainContainer = target.ToolchainFile is null
                    ? null
                    : $"{containerRoot}/{ToContainerRelativePath(target.ToolchainFile)}";

                var script = ScriptBuilder.BuildLinuxScript(_config, target, containerRoot, buildDirContainer, toolchainContainer);
                var scriptPath = Path.Combine(scratchDir, "build.sh");
                await File.WriteAllTextAsync(scriptPath, script, cancellationToken);

                var scriptContainerPath = $"{containerRoot}/.build-orchestrator/{target.Name}/build.sh";

                fileName = "docker";
                arguments = new List<string> { "run", "--rm", "-v", $"{_repoRootFull}:{containerRoot}", "-w", containerRoot };
                arguments.AddRange(docker.ExtraDockerArgs);
                arguments.Add(docker.Image);
                arguments.Add("bash");
                arguments.Add(scriptContainerPath);
                scriptFileNameForDryRunPreview = "build.sh";
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
                arguments = new List<string> { "/d", "/c", scriptPath };
                scriptFileNameForDryRunPreview = "build.bat";
            }

            if (_dryRun)
            {
                Console.WriteLine($"[{target.Name}] DRY RUN — would execute: {fileName} {string.Join(' ', arguments)}");
                Console.WriteLine($"[{target.Name}] --- generated script ---");
                var scriptContent = await File.ReadAllTextAsync(
                    Path.Combine(scratchDir, scriptFileNameForDryRunPreview), cancellationToken);
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

    /// <summary>Normalizes a config-relative path (which may use Windows backslashes if a
    /// user hand-edited the JSON) into a forward-slash path suitable for use inside the
    /// Linux container.</summary>
    private static string ToContainerRelativePath(string relativePath) =>
        relativePath.Replace('\\', '/').TrimStart('/');

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
            case TargetKind.Linux:
                if (target.Docker is null)
                    throw new InvalidOperationException($"Target '{target.Name}' (Linux) has no 'docker' options.");
                if (!IsDockerCliAvailable())
                    throw new InvalidOperationException(
                        "docker CLI not found on PATH. Install Docker Desktop (or Docker Engine) " +
                        "and make sure it's running.");
                break;
        }
    }

    private static bool IsDockerCliAvailable()
    {
        try
        {
            var psi = new ProcessStartInfo("docker", "--version")
            {
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                UseShellExecute = false,
                CreateNoWindow = true,
            };
            using var process = Process.Start(psi);
            if (process is null) return false;
            process.WaitForExit(5000);
            return process.HasExited && process.ExitCode == 0;
        }
        catch
        {
            // docker.exe not on PATH, or couldn't be launched.
            return false;
        }
    }
}
