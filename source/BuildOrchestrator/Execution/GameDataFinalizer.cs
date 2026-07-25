using REngine.BuildOrchestrator.Models;

namespace REngine.BuildOrchestrator.Execution;

/// <summary>Runs BuildScripts/RunGameDataFinalize.cmake exactly once, before any platform
/// target is dispatched, producing the shared Build/&lt;GameName&gt;/GameData that every
/// target then either preloads directly (Emscripten) or copies from (everyone else) — see
/// PrepareGameData.cmake / SKIP_GAMEDATA_FINALIZE on the engine side. Reuses TargetResult /
/// ProcessRunner so this step shows up in the same console output, log file, and summary.json
/// as every other target.</summary>
public static class GameDataFinalizer
{
    public const string StepName = "gamedata";

    public static async Task<TargetResult> RunAsync(
        BuildConfig config, string repoRootFull, string logRootFull, bool dryRun, bool forceGameData,
        CancellationToken cancellationToken)
    {
        var logFilePath = Path.Combine(logRootFull, $"{StepName}.log");
        var scriptPath = Path.Combine(repoRootFull, "BuildScripts", "RunGameDataFinalize.cmake");

        if (!File.Exists(scriptPath))
        {
            return new TargetResult
            {
                Name = StepName,
                Success = false,
                Duration = TimeSpan.Zero,
                LogFilePath = logFilePath,
                ErrorMessage = $"Not found: {scriptPath}. Update the engine checkout, or set " +
                               "FinalizeGameDataOnce=false to fall back to each target finalizing its own GameData.",
            };
        }

        if (!ExecutableCheck.IsAvailable("cmake"))
        {
            return new TargetResult
            {
                Name = StepName,
                Success = false,
                Duration = TimeSpan.Zero,
                LogFilePath = logFilePath,
                ErrorMessage = "cmake not found on the host PATH (needed to run RunGameDataFinalize.cmake " +
                               "before any platform build starts).",
            };
        }

        var arguments = new List<string> { $"-DGAME_NAME={config.GameName}" };
        if (forceGameData)
            arguments.Add("-DFORCE_GAMEDATA=1");
        arguments.Add("-P");
        arguments.Add(scriptPath);

        if (dryRun)
        {
            Console.WriteLine($"[{StepName}] DRY RUN — would execute: cmake {string.Join(' ', arguments)}");
            return new TargetResult { Name = StepName, Success = true, Duration = TimeSpan.Zero, LogFilePath = logFilePath };
        }

        Console.WriteLine($"[{StepName}] Finalizing GameData once, before any platform build...");
        var result = await ProcessRunner.RunAsync(
            "cmake", arguments, repoRootFull, logFilePath, StepName, cancellationToken);

        return new TargetResult
        {
            Name = StepName,
            Success = result.Success,
            Duration = result.Duration,
            LogFilePath = logFilePath,
            ErrorMessage = result.Success ? null : $"Exit code {result.ExitCode} — see {logFilePath}",
        };
    }
}
