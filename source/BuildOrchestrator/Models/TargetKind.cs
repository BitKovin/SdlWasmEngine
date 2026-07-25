namespace REngine.BuildOrchestrator.Models;

/// <summary>
/// Which environment-activation + build strategy a target uses.
/// Maps directly onto REngine_2.0's own toolchain/platform pairing:
///   Windows      -> cmake/toolchains/msvc.cmake, via vcvarsall.bat
///   Linux        -> cmake/toolchains/clang.cmake, built inside the Steam Linux Runtime
///                   3.0 "sniper" Docker container (registry.gitlab.steamos.cloud/steamrt/sniper/sdk)
///   Emscripten   -> emcmake (clang.cmake not required)
///   XboxGdk      -> cmake/toolchains/msvc.cmake + -DXBOX_GDK=1, via GamingXboxVars.cmd
///   WindowsGdk   -> cmake/toolchains/msvc.cmake + -DWINDOWS_GDK=1, via GamingDesktopVars.cmd
/// XboxGdk/WindowsGdk use GdkOptions (not WindowsOptions) — the GDK's own vars script sets up
/// both the GDK environment and the matching VS/MSVC compiler environment in one call.
/// </summary>
public enum TargetKind
{
    Windows,
    Linux,
    Emscripten,
    XboxGdk,
    WindowsGdk
}
