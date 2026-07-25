using System.Diagnostics;

namespace REngine.BuildOrchestrator.Execution;

/// <summary>Lightweight "is this CLI on PATH and runnable" check, shared by every
/// preflight check that needs one (docker, cmake, ...). Not a substitute for actually
/// running the real command — just turns "file not found" into a clear message up front
/// instead of a raw Win32Exception deep inside a build.</summary>
public static class ExecutableCheck
{
    public static bool IsAvailable(string exe, string versionArg = "--version")
    {
        try
        {
            var psi = new ProcessStartInfo(exe, versionArg)
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
            // Not on PATH, or couldn't be launched.
            return false;
        }
    }
}
