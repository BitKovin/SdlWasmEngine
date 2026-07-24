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
    /// activated first (vcvarsall.bat vs emsdk_env.bat) and which extra cmake args apply.</summary>
    public static string BuildWindowsScript(
        BuildConfig config, BuildTarget target, string repoRootFull, string buildDirFull, string? toolchainFileFull)
    {
        var gameName = target.GameName ?? config.GameName;
        var sb = new StringBuilder();

        sb.AppendLine("@echo off");
        sb.AppendLine("setlocal enabledelayedexpansion");
        sb.AppendLine();

        if (target.Kind == TargetKind.Emscripten)
        {
            sb.AppendLine("echo === Activating Emscripten environment ===");
            sb.AppendLine($"call \"{target.Emscripten!.EmsdkEnvScript}\"");
        }
        else
        {
            sb.AppendLine("echo === Activating Visual Studio environment ===");
            sb.AppendLine($"call \"{target.Windows!.VcVarsAllPath}\" {target.Windows.Arch}");
        }

        sb.AppendLine("if errorlevel 1 (");
        sb.AppendLine("  echo Failed to activate build environment.");
        sb.AppendLine("  exit /b 1");
        sb.AppendLine(")");
        sb.AppendLine();

        foreach (var line in target.ExtraEnvCommands)
            sb.AppendLine(line);

        var cmakeExe = target.Kind == TargetKind.Emscripten ? "emcmake cmake" : "cmake";
        var toolchainArg = toolchainFileFull is null ? "" : $" -DCMAKE_TOOLCHAIN_FILE=\"{toolchainFileFull}\"";
        var extraArgs = string.Join(' ', target.ExtraCMakeArgs);

        sb.AppendLine($"echo === Configuring [{target.Name}] ===");
        sb.AppendLine(
            $"{cmakeExe} -S \"{repoRootFull}\" -B \"{buildDirFull}\" -G \"{target.Generator}\" " +
            $"-DCMAKE_BUILD_TYPE={target.Configuration}{toolchainArg} -DGAME_NAME=\"{gameName}\" {extraArgs}");
        sb.AppendLine("if errorlevel 1 exit /b 1");
        sb.AppendLine();

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
        BuildConfig config, BuildTarget target, string repoRootContainer, string buildDirContainer, string? toolchainFileContainer)
    {
        var gameName = target.GameName ?? config.GameName;
        var toolchainArg = toolchainFileContainer is null ? "" : $" -DCMAKE_TOOLCHAIN_FILE=\"{toolchainFileContainer}\"";
        var extraArgs = string.Join(' ', target.ExtraCMakeArgs);

        var sb = new StringBuilder();
        void Line(string text) => sb.Append(text).Append('\n');

        Line("#!/usr/bin/env bash");
        Line("set -euo pipefail");
        Line("");

        foreach (var line in target.ExtraEnvCommands)
            Line(line);

        Line($"echo '=== Configuring [{target.Name}] (sniper container) ==='");
        Line(
            $"cmake -S \"{repoRootContainer}\" -B \"{buildDirContainer}\" -G \"{target.Generator}\" " +
            $"-DCMAKE_BUILD_TYPE={target.Configuration}{toolchainArg} -DGAME_NAME=\"{gameName}\" {extraArgs}");
        Line("");
        Line($"echo '=== Building [{target.Name}] ==='");
        Line($"cmake --build \"{buildDirContainer}\" --config {target.Configuration}");

        return sb.ToString();
    }
}
