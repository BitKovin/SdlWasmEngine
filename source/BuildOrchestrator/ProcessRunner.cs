using System.Diagnostics;

namespace REngine.BuildOrchestrator.Execution;

public sealed class ProcessResult
{
    public required int ExitCode { get; init; }
    public required bool Success { get; init; }
    public required TimeSpan Duration { get; init; }
}

/// <summary>Runs a single process, streaming its output live to the console (prefixed per
/// target so interleaved parallel builds stay readable) and to a per-target log file.</summary>
public static class ProcessRunner
{
    public static async Task<ProcessResult> RunAsync(
        string fileName,
        string arguments,
        string workingDirectory,
        string logFilePath,
        string targetName,
        CancellationToken cancellationToken)
    {
        var stopwatch = Stopwatch.StartNew();

        Directory.CreateDirectory(Path.GetDirectoryName(logFilePath)!);
        await using var log = new StreamWriter(logFilePath, append: false) { AutoFlush = true };

        var psi = new ProcessStartInfo
        {
            FileName = fileName,
            Arguments = arguments,
            WorkingDirectory = workingDirectory,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true,
        };

        using var process = new Process { StartInfo = psi, EnableRaisingEvents = true };
        var sync = new object();

        void Write(string? line)
        {
            if (line is null) return;
            lock (sync)
            {
                Console.WriteLine($"[{targetName}] {line}");
                log.WriteLine(line);
            }
        }

        process.OutputDataReceived += (_, e) => Write(e.Data);
        process.ErrorDataReceived += (_, e) => Write(e.Data);

        try
        {
            process.Start();
        }
        catch (Exception ex)
        {
            log.WriteLine($"Failed to start process: {ex.Message}");
            return new ProcessResult { ExitCode = -1, Success = false, Duration = stopwatch.Elapsed };
        }

        process.BeginOutputReadLine();
        process.BeginErrorReadLine();

        await using var registration = cancellationToken.Register(() =>
        {
            try
            {
                if (!process.HasExited)
                    process.Kill(entireProcessTree: true);
            }
            catch
            {
                // Best-effort: the process may have already exited between the check and Kill().
            }
        });

        await process.WaitForExitAsync(CancellationToken.None);
        stopwatch.Stop();

        var exitCode = process.ExitCode;
        return new ProcessResult
        {
            ExitCode = exitCode,
            Success = exitCode == 0 && !cancellationToken.IsCancellationRequested,
            Duration = stopwatch.Elapsed,
        };
    }
}
