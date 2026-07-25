using REngine.BuildOrchestrator.Config;
using REngine.BuildOrchestrator.Execution;
using REngine.BuildOrchestrator.Models;

namespace REngine.BuildOrchestrator;

internal static class Program
{
    private static async Task<int> Main(string[] args)
    {
        CliOptions options;
        try
        {
            options = CliOptions.Parse(args);
        }
        catch (ArgumentException ex)
        {
            Console.Error.WriteLine($"Error: {ex.Message}");
            return 1;
        }

        if (options.ShowHelp)
        {
            CliOptions.PrintHelp();
            return 0;
        }

        if (options.Init)
        {
            try
            {
                ConfigLoader.WriteDefault(options.ConfigPath);
                Console.WriteLine(
                    $"Wrote starter config to {options.ConfigPath}. Edit the tool paths for your machine, " +
                    "then run again without --init.");
                return 0;
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"Error: {ex.Message}");
                return 1;
            }
        }

        BuildConfig config;
        try
        {
            config = ConfigLoader.Load(options.ConfigPath);
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"Error: {ex.Message}");
            return 1;
        }

        if (options.Parallel.HasValue)
            config.Parallel = options.Parallel.Value;

        if (options.List)
        {
            Console.WriteLine("Configured targets:");
            foreach (var t in config.Targets)
                Console.WriteLine($"  [{(t.Enabled ? "x" : " ")}] {t.Name,-16} {t.Kind,-12} -> {t.BuildDir}");
            return 0;
        }

        var targets = config.Targets.Where(t => t.Enabled).ToList();

        if (options.Only.Count > 0)
        {
            var wanted = new HashSet<string>(options.Only, StringComparer.OrdinalIgnoreCase);
            var matched = targets.Where(t => wanted.Contains(t.Name)).ToList();

            foreach (var w in wanted)
            {
                if (matched.All(t => !t.Name.Equals(w, StringComparison.OrdinalIgnoreCase)))
                    Console.Error.WriteLine($"Warning: --only target '{w}' not found (or not enabled) in config.");
            }

            targets = matched;
        }

        if (targets.Count == 0)
        {
            Console.Error.WriteLine("No enabled targets to build. Check your config or --only filter.");
            return 1;
        }

        using var cts = new CancellationTokenSource();
        Console.CancelKeyPress += (_, e) =>
        {
            e.Cancel = true;
            Console.WriteLine();
            Console.WriteLine("Cancelling — stopping running builds...");
            cts.Cancel();
        };

        var engine = new OrchestratorEngine(config, options.DryRun, options.Clean, options.ForceGameData);
        var results = await engine.RunAsync(targets, cts.Token);

        return results.All(r => r.Success) ? 0 : 1;
    }
}
