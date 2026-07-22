namespace REngine.BuildOrchestrator;

internal sealed class CliOptions
{
    public string ConfigPath { get; private set; } = "build-targets.json";
    public bool DryRun { get; private set; }
    public bool Clean { get; private set; }
    public bool Init { get; private set; }
    public bool List { get; private set; }
    public bool ShowHelp { get; private set; }
    public bool? Parallel { get; private set; }
    public List<string> Only { get; } = new();

    public static CliOptions Parse(string[] args)
    {
        var options = new CliOptions();

        for (var i = 0; i < args.Length; i++)
        {
            switch (args[i])
            {
                case "--config":
                    options.ConfigPath = RequireValue(args, ref i, "--config");
                    break;
                case "--only":
                    var value = RequireValue(args, ref i, "--only");
                    options.Only.AddRange(
                        value.Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries));
                    break;
                case "--dry-run":
                    options.DryRun = true;
                    break;
                case "--clean":
                    options.Clean = true;
                    break;
                case "--init":
                    options.Init = true;
                    break;
                case "--list":
                    options.List = true;
                    break;
                case "--parallel":
                    options.Parallel = true;
                    break;
                case "--sequential":
                    options.Parallel = false;
                    break;
                case "-h":
                case "--help":
                    options.ShowHelp = true;
                    break;
                default:
                    throw new ArgumentException($"Unknown argument: {args[i]} (use --help)");
            }
        }

        return options;
    }

    private static string RequireValue(string[] args, ref int i, string flag)
    {
        if (i + 1 >= args.Length)
            throw new ArgumentException($"{flag} requires a value.");
        return args[++i];
    }

    public static void PrintHelp()
    {
        Console.WriteLine("""
        rengine-build — builds REngine_2.0 for every configured platform in one run.

        Usage:
          rengine-build [options]

        Options:
          --config <path>   Path to the JSON target config (default: build-targets.json)
          --init             Write a starter build-targets.json next to the exe and exit
          --list             List configured targets (enabled/disabled) and exit
          --only a,b,c       Build only the named targets
          --clean            Delete each target's build directory before configuring
          --dry-run          Print the generated build script for each target without running it
          --parallel         Build independent targets concurrently (overrides config)
          --sequential       Build targets one at a time (overrides config)
          -h, --help         Show this help

        Examples:
          rengine-build --init
          rengine-build --only windows,linux --dry-run
          rengine-build --clean --parallel
        """);
    }
}
