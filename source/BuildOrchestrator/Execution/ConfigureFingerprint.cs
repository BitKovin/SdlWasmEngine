using System.Security.Cryptography;
using System.Text;
using REngine.BuildOrchestrator.Models;

namespace REngine.BuildOrchestrator.Execution;

/// <summary>Fingerprints everything that feeds into a target's `cmake -S -B ...` configure
/// call, so BuildRunner can skip re-running full configure when nothing relevant changed
/// since last time — `cmake --build` alone still triggers CMake's own lightweight internal
/// reconfigure-if-needed check (driven by CMakeLists.txt mtimes), so this only skips the
/// expensive, unconditional full command-line reconfigure, not correctness.</summary>
public static class ConfigureFingerprint
{
    private const string FileName = ".rengine-configure-fingerprint";

    public static string Compute(BuildConfig config, BuildTarget target, string? toolchainFileFull)
    {
        var gameName = target.GameName ?? config.GameName;
        var parts = new[]
        {
            toolchainFileFull ?? "",
            target.Generator,
            target.Configuration,
            gameName,
            string.Join(';', target.ExtraCMakeArgs),
            config.FinalizeGameDataOnce ? "1" : "0",
        };

        var bytes = Encoding.UTF8.GetBytes(string.Join('\u0001', parts));
        return Convert.ToHexString(SHA256.HashData(bytes));
    }

    /// <summary>True if the build directory already has a completed configure (CMakeCache.txt
    /// exists) whose recorded fingerprint matches the current one — safe to skip straight to
    /// `cmake --build` instead of a full `cmake -S -B ...` reconfigure.</summary>
    public static bool CanSkipConfigure(string buildDirFull, string currentFingerprint)
    {
        var cacheFile = Path.Combine(buildDirFull, "CMakeCache.txt");
        var fingerprintFile = Path.Combine(buildDirFull, FileName);

        if (!File.Exists(cacheFile) || !File.Exists(fingerprintFile))
            return false;

        try
        {
            var stored = File.ReadAllText(fingerprintFile).Trim();
            return stored == currentFingerprint;
        }
        catch
        {
            return false;
        }
    }

    public static void Save(string buildDirFull, string fingerprint)
    {
        try
        {
            Directory.CreateDirectory(buildDirFull);
            File.WriteAllText(Path.Combine(buildDirFull, FileName), fingerprint);
        }
        catch
        {
            // Non-fatal: worst case, the next run just does a redundant full reconfigure.
        }
    }
}
