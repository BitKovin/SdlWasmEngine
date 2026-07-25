using System.Text;
using REngine.BuildOrchestrator.Models;

namespace REngine.BuildOrchestrator.Execution;

/// <summary>
/// Generates the batch/shell scripts that activate a target's toolchain environment and
/// drive CMake. Writing a real script file — rather than cramming everything into one
/// "cmd /c ..." string — sidesteps cmd.exe/bash quoting problems and means a failed build
/// leaves behind a script you can just double-click or re-run by hand to reproduce it.
/// </summary>
public static class ScriptBuilder
{
    /// <summary>Windows, XboxGdk, WindowsGdk, and Emscripten all run as a generated .bat
    /// executed via cmd.exe on the host — they only differ in which environment gets
    /// activated first and which extra cmake args apply. skipConfigure omits the (expensive)
    /// full `cmake -S -B` call entirely when BuildRunner has determined the existing build
    /// directory is already configured with the same effective arguments.</summary>
    public static string BuildWindowsScript(
        BuildConfig config, BuildTarget target, string repoRootFull, string buildDirFull,
        string? toolchainFileFull, bool skipConfigure)
    {
        var gameName = target.GameName ?? config.GameName;
        var sb = new StringBuilder();

        sb.AppendLine("@echo off");
        sb.AppendLine("setlocal enabledelayedexpansion");
        sb.AppendLine();

        switch (target.Kind)
        {
            case TargetKind.Emscripten:
                sb.AppendLine("echo === Activating Emscripten environment ===");
                sb.AppendLine($"call \"{target.Emscripten!.EmsdkEnvScript}\"");
                break;
            case TargetKind.XboxGdk:
            case TargetKind.WindowsGdk:
                // GamingDesktopVars.cmd / GamingXboxVars.cmd sets up BOTH the GDK environment
                // and the matching VS/MSVC compiler environment in one call — no separate
                // vcvarsall.bat needed for these two target kinds.
                sb.AppendLine("echo === Activating GDK environment ===");
                sb.AppendLine($"call \"{target.Gdk!.VarsScriptPath}\" {target.Gdk.Flavor}");
                break;
            default:
                sb.AppendLine("echo === Activating Visual Studio environment ===");
                sb.AppendLine($"call \"{target.Windows!.VcVarsAllPath}\" {target.Windows.Arch}");
                break;
        }

        sb.AppendLine("if errorlevel 1 (");
        sb.AppendLine("  echo Failed to activate build environment.");
        sb.AppendLine("  exit /b 1");
        sb.AppendLine(")");
        sb.AppendLine();

        foreach (var line in target.ExtraEnvCommands)
            sb.AppendLine(line);

        if (skipConfigure)
        {
            sb.AppendLine($"echo === [{target.Name}] Build directory already configured, matching args — skipping reconfigure ===");
            sb.AppendLine();
        }
        else
        {
            var cmakeExe = target.Kind == TargetKind.Emscripten ? "emcmake cmake" : "cmake";
            var toolchainArg = toolchainFileFull is null ? "" : $" -DCMAKE_TOOLCHAIN_FILE=\"{toolchainFileFull}\"";
            // Auto-injected first so a target can still override it via its own ExtraCMakeArgs —
            // when the same -D flag is passed twice, CMake's command-line parsing lets the later
            // one win.
            var autoArgs = config.FinalizeGameDataOnce ? "-DSKIP_GAMEDATA_FINALIZE=1" : "";
            var extraArgs = string.Join(' ', new[] { autoArgs }.Concat(target.ExtraCMakeArgs).Where(a => a.Length > 0));

            sb.AppendLine($"echo === Configuring [{target.Name}] ===");
            sb.AppendLine(
                $"{cmakeExe} -S \"{repoRootFull}\" -B \"{buildDirFull}\" -G \"{target.Generator}\" " +
                $"-DCMAKE_BUILD_TYPE={target.Configuration}{toolchainArg} -DGAME_NAME=\"{gameName}\" {extraArgs}");
            sb.AppendLine("if errorlevel 1 exit /b 1");
            sb.AppendLine();
        }

        sb.AppendLine($"echo === Building [{target.Name}] ===");
        sb.AppendLine($"cmake --build \"{buildDirFull}\" --config {target.Configuration}");
        sb.AppendLine("if errorlevel 1 exit /b 1");
        sb.AppendLine();
        sb.AppendLine("exit /b 0");

        return sb.ToString();
    }

    /// <summary>Linux runs as a generated .sh executed inside the Steam Linux Runtime 3.0
    /// "sniper" container via `docker run`. Paths passed in are already container-side paths
    /// (RepoRoot bind-mounted at ContainerRepoPath), no runtime path translation needed.
    /// Content is built with explicit '\n' line endings regardless of host OS — if this ran
    /// through AppendLine on Windows (CRLF) bash would choke on stray '\r' at line ends.</summary>
    public static string BuildLinuxScript(
        BuildConfig config, BuildTarget target, string repoRootContainer, string buildDirContainer,
        string? toolchainFileContainer, bool skipConfigure)
    {
        var gameName = target.GameName ?? config.GameName;

        var sb = new StringBuilder();
        void Line(string text) => sb.Append(text).Append('\n');

        Line("#!/usr/bin/env bash");
        Line("set -euo pipefail");
        Line("");

        foreach (var line in target.ExtraEnvCommands)
            Line(line);

        if (skipConfigure)
        {
            Line($"echo '=== [{target.Name}] Build directory already configured, matching args — skipping reconfigure ==='");
            Line("");
        }
        else
        {
            var toolchainArg = toolchainFileContainer is null ? "" : $" -DCMAKE_TOOLCHAIN_FILE=\"{toolchainFileContainer}\"";
            var autoArgs = config.FinalizeGameDataOnce ? "-DSKIP_GAMEDATA_FINALIZE=1" : "";
            var extraArgs = string.Join(' ', new[] { autoArgs }.Concat(target.ExtraCMakeArgs).Where(a => a.Length > 0));

            Line($"echo '=== Configuring [{target.Name}] (sniper container) ==='");
            Line(
                $"cmake -S \"{repoRootContainer}\" -B \"{buildDirContainer}\" -G \"{target.Generator}\" " +
                $"-DCMAKE_BUILD_TYPE={target.Configuration}{toolchainArg} -DGAME_NAME=\"{gameName}\" {extraArgs}");
            Line("");
        }

        Line($"echo '=== Building [{target.Name}] ==='");
        Line($"cmake --build \"{buildDirContainer}\" --config {target.Configuration}");

        return sb.ToString();
    }
}
