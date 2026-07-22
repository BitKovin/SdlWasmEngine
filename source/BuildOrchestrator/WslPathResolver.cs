using System.Diagnostics;

namespace REngine.BuildOrchestrator.Execution;

/// <summary>Translates a Windows path into the corresponding path inside a WSL distro by
/// shelling out to the distro's own `wslpath`, so custom/non-default mount configurations
/// are respected rather than assumed.</summary>
public static class WslPathResolver
{
    public static async Task<string> ToWslPathAsync(string distro, string windowsPath)
    {
        var psi = new ProcessStartInfo
        {
            FileName = "wsl.exe",
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true,
        };
        psi.ArgumentList.Add("-d");
        psi.ArgumentList.Add(distro);
        // -e/--exec bypasses the distro's default login shell. Without it, wsl.exe hands the
        // whole command line to `bash -c "..."`, and bash's backslash-escaping silently eats
        // every backslash in a Windows path that doesn't happen to contain a space (the space
        // case gets quoted by wsl.exe and survives) — e.g. "F:\Git\Foo" becomes "F:GitFoo"
        // before wslpath ever sees it. -e execs wslpath directly, so the already-correctly-
        // split ArgumentList entries reach it unmodified.
        psi.ArgumentList.Add("-e");
        psi.ArgumentList.Add("wslpath");
        psi.ArgumentList.Add("-a");
        psi.ArgumentList.Add(windowsPath);

        using var process = new Process { StartInfo = psi };

        try
        {
            process.Start();
        }
        catch (Exception ex)
        {
            throw new InvalidOperationException(
                $"Could not launch wsl.exe to translate '{windowsPath}'. Is WSL installed? ({ex.Message})", ex);
        }

        var output = (await process.StandardOutput.ReadToEndAsync()).Trim();
        var error = await process.StandardError.ReadToEndAsync();
        await process.WaitForExitAsync();

        if (process.ExitCode != 0 || string.IsNullOrWhiteSpace(output))
        {
            throw new InvalidOperationException(
                $"wslpath failed to translate '{windowsPath}' in distro '{distro}': {error}");
        }

        return output;
    }
}