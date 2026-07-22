using System.Text.Json;
using REngine.BuildOrchestrator.Models;

namespace REngine.BuildOrchestrator.Execution;

/// <summary>Drives every enabled BuildTarget through a BuildRunner — sequentially by
/// default, or with bounded concurrency when config.Parallel is set — and produces a
/// console summary table plus a machine-readable summary.json for CI to parse.</summary>
public sealed class OrchestratorEngine
{
    private readonly BuildConfig _config;
    private readonly BuildRunner _runner;

    public OrchestratorEngine(BuildConfig config, bool dryRun, bool clean)
    {
        _config = config;
        _runner = new BuildRunner(config, dryRun, clean);
    }

    public async Task<List<TargetResult>> RunAsync(List<BuildTarget> targets, CancellationToken cancellationToken)
    {
        var start = DateTimeOffset.Now;
        List<TargetResult> results;

        Console.WriteLine($"Building {targets.Count} target(s): {string.Join(", ", targets.Select(t => t.Name))}");
        Console.WriteLine(_config.Parallel ? $"Mode: parallel (max {_config.MaxParallelism})" : "Mode: sequential");
        Console.WriteLine();

        if (_config.Parallel)
        {
            using var gate = new SemaphoreSlim(Math.Max(1, _config.MaxParallelism));
            var tasks = targets.Select(async target =>
            {
                await gate.WaitAsync(cancellationToken);
                try
                {
                    return await _runner.RunAsync(target, cancellationToken);
                }
                finally
                {
                    gate.Release();
                }
            });
            results = (await Task.WhenAll(tasks)).ToList();
        }
        else
        {
            results = new List<TargetResult>();
            foreach (var target in targets)
            {
                if (cancellationToken.IsCancellationRequested)
                {
                    results.Add(new TargetResult
                    {
                        Name = target.Name,
                        Success = false,
                        Duration = TimeSpan.Zero,
                        LogFilePath = "",
                        ErrorMessage = "Skipped (cancelled)",
                    });
                    continue;
                }

                results.Add(await _runner.RunAsync(target, cancellationToken));
            }
        }

        PrintSummary(results, DateTimeOffset.Now - start);
        WriteSummaryFile(results);

        return results;
    }

    private static void PrintSummary(List<TargetResult> results, TimeSpan totalDuration)
    {
        Console.WriteLine();
        Console.WriteLine("=== Build summary ===");
        var nameWidth = Math.Max(6, results.Max(r => r.Name.Length));

        foreach (var r in results)
        {
            var status = r.Success ? "OK" : "FAILED";
            var line = $"  {r.Name.PadRight(nameWidth)}  {status,-6}  {r.Duration:mm\\:ss}";
            if (r.ErrorMessage is not null)
                line += $"  — {r.ErrorMessage}";
            Console.WriteLine(line);
        }

        Console.WriteLine($"Total time: {totalDuration:mm\\:ss}");
        Console.WriteLine(results.All(r => r.Success) ? "All targets succeeded." : "One or more targets failed.");
    }

    private void WriteSummaryFile(List<TargetResult> results)
    {
        try
        {
            var logRoot = Path.Combine(Path.GetFullPath(_config.RepoRoot), _config.LogDirectory);
            Directory.CreateDirectory(logRoot);
            var path = Path.Combine(logRoot, "summary.json");

            var payload = results.Select(r => new
            {
                r.Name,
                r.Success,
                DurationSeconds = r.Duration.TotalSeconds,
                r.LogFilePath,
                r.ErrorMessage,
            });

            File.WriteAllText(path, JsonSerializer.Serialize(payload, new JsonSerializerOptions { WriteIndented = true }));
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"Warning: failed to write summary.json: {ex.Message}");
        }
    }
}
