# rengine-build

A small, dependency-free C# console app that builds [REngine_2.0](https://github.com/BitKovin/REngine_2.0)
for every target platform in one run: Windows, Linux, Emscripten, and (optionally) Xbox GDK / Windows GDK.

It doesn't cross-compile — each target still uses its real toolchain — it just drives them all
from one place: activates the right environment, runs `cmake` configure + build for each enabled
target, streams output to the console and to per-target logs, and prints a pass/fail summary at
the end. **Run it from a Windows machine** — that's the only OS that can reach MSVC, GDK, Docker
(for the Linux build), and Emscripten all at once.

## Prerequisites

| Target       | Needs |
|--------------|-------|
| Windows      | Visual Studio (with the C++ workload) |
| Linux        | **Docker Desktop** (or Docker Engine), running. The Linux build happens inside the [Steam Linux Runtime 3.0 "sniper"](https://gitlab.steamos.cloud/steamrt/sniper/sdk) SDK container — no WSL, no manually installed Linux toolchain. First run pulls the image (~1–2 GB), so be patient once. |
| Emscripten   | [emsdk](https://emscripten.org/docs/getting_started/downloads.html) installed and activated once (`emsdk install latest && emsdk activate latest`) |
| Xbox GDK / Windows GDK | Visual Studio + the [Microsoft GDK extension](https://github.com/microsoft/GDK) installed and licensed. This is the one leg that can't run on a public CI runner — only a machine you control with the GDK installed. |
| All targets  | **Python 3** on PATH — `BuildScripts/PrepareGameData.cmake` requires it (`find_package(Python3 ... REQUIRED)`) to finalize game data after linking. |

The tool itself only needs the [.NET 8 runtime](https://dotnet.microsoft.com/download/dotnet/8.0)
(or build it self-contained — see below).

## Why sniper instead of a plain Linux install/WSL

`registry.gitlab.steamos.cloud/steamrt/sniper/sdk` is Valve's own official build environment for
Linux titles on Steam: Debian 11 "bullseye", with `cmake`, `make`, `clang`/`clang-11`, and `gcc`
already installed (confirmed against the SDK's package manifest — REngine's existing
`cmake/toolchains/clang.cmake` works in it unmodified, no extra `apt-get install` needed). Two
concrete advantages over building against whatever's on your WSL distro:

- **No local Linux toolchain setup at all** — Docker pulls a known-good, pinned environment.
- **Correct compatibility baseline.** Building against sniper's ~2021-era glibc means your Linux
  binary will actually run on the range of distros real Steam players have, instead of picking up
  symbol versions from a bleeding-edge Ubuntu that older systems can't satisfy — this is the same
  environment Valve recommends game developers actually ship from.

## Setup

1. Build the tool:
   ```
   dotnet publish -c Release -r win-x64 --self-contained true -p:PublishSingleFile=true
   ```
   This produces a single `rengine-build.exe` under `bin/Release/net8.0/win-x64/publish/`
   with no .NET install required to run it. (Plain `dotnet build` also works if you already
   have the runtime and just want to iterate.)

2. Copy `rengine-build.exe` (or run `dotnet rengine-build.dll`) into your REngine_2.0 checkout,
   or anywhere else — `RepoRoot` in the config points at the repo, it doesn't have to live inside it.

3. Generate a starter config:
   ```
   rengine-build --init
   ```
   This writes `build-targets.json` with `windows`, `linux`, and `emscripten` targets enabled,
   and `xbox-gdk` / `windows-gdk` present but disabled.

4. Edit `build-targets.json`:
   - Set `RepoRoot` to your REngine_2.0 checkout path.
   - Fix `windows.vcVarsAllPath` to match your Visual Studio install.
   - Fix `emscripten.emsdkEnvScript` to match your emsdk checkout.
   - Leave `linux.docker.image` as-is, or pin it to a specific sniper snapshot tag instead of
     `:latest` for fully reproducible builds (see the [sniper release notes](https://gitlab.steamos.cloud/steamrt/steamrt/-/wikis/Sniper-release-notes) for available tags).
   - Set `GameName` to your actual game (matches `-DGAME_NAME=`), or leave `"Game"` for the default.
   - Flip `enabled: true` on `xbox-gdk` / `windows-gdk` once GDK is installed.

5. Sanity-check without building anything:
   ```
   rengine-build --dry-run
   ```
   This prints the exact generated script per target — a quick way to confirm paths look right
   before spending 20 minutes compiling.

6. Build everything:
   ```
   rengine-build
   ```

## CLI reference

```
rengine-build [options]

  --config <path>   Path to the JSON target config (default: build-targets.json)
  --init             Write a starter build-targets.json next to the exe and exit
  --list             List configured targets (enabled/disabled) and exit
  --only a,b,c       Build only the named targets
  --clean            Delete each target's build directory before configuring
  --dry-run          Print the generated build script for each target without running it
  --parallel         Build independent targets concurrently (overrides config)
  --sequential       Build targets one at a time (overrides config)
  -h, --help         Show this help
```

Exit code is `0` only if every target succeeded, so it's CI-friendly as-is.

## What actually happens per target

- **Windows / Xbox GDK / Windows GDK** — a generated `.bat` runs `vcvarsall.bat`, then
  `cmake -S ... -B ... -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/msvc.cmake ...`,
  then `cmake --build`. GDK targets add `-DXBOX_GDK=1` / `-DWINDOWS_GDK=1`; the GDK's own
  `GRDKLatest`/`GameDKLatest` environment variables (set globally by the GDK installer) are
  inherited automatically, so no extra activation is needed beyond `vcvarsall.bat` for `cl.exe`.
- **Linux** — `RepoRoot` is bind-mounted into a `sniper/sdk` container at `/repo` (configurable),
  and a generated `.sh` runs inside it via `docker run --rm -v <repo>:/repo -w /repo <image> bash
  /repo/.build-orchestrator/linux/build.sh`, using `cmake/toolchains/clang.cmake`. Because the
  mount point is a plain bind-mount, build output lands right back in `<RepoRoot>/build/linux` on
  the host — no copy-out step. Container paths are built with plain string concatenation (no
  runtime path-translation step needed, unlike WSL's `wslpath`).
- **Emscripten** — a generated `.bat` calls `emsdk_env.bat`, then `emcmake cmake ...` (no
  `CMAKE_TOOLCHAIN_FILE` needed — `emcmake` sets the compiler itself, per the repo's own comments).

Every generated script is written to `.build-orchestrator/<target>/build.bat` (or `.sh`) inside
`RepoRoot` and left in place after the run, so a failure is easy to reproduce by hand — for
Windows-family targets, open a Developer Command Prompt and run it directly; for Linux, run
`docker run --rm -v <repo>:/repo -w /repo <image> bash /repo/.build-orchestrator/linux/build.sh`
yourself.

Console output is streamed live prefixed with `[target-name]`, full output goes to
`BuildLogs/<target>.log`, and a machine-readable `BuildLogs/summary.json` is written at the end
(name, success, duration, log path) for CI to parse.

## Config reference (`build-targets.json`)

| Field | Meaning |
|---|---|
| `RepoRoot` | Path to the REngine_2.0 checkout |
| `GameName` | Default `-DGAME_NAME=`; override per-target with `Targets[].GameName` |
| `LogDirectory` | Where logs + `summary.json` go (relative to `RepoRoot`) |
| `Parallel` / `MaxParallelism` | Build targets concurrently instead of one after another |
| `Targets[].Kind` | `Windows`, `Linux`, `Emscripten`, `XboxGdk`, or `WindowsGdk` |
| `Targets[].BuildDir` | CMake build dir, relative to `RepoRoot`; must be unique per target |
| `Targets[].Docker.Image` | Sniper (or other) image the Linux target builds inside |
| `Targets[].Docker.ContainerRepoPath` | Where `RepoRoot` is bind-mounted inside the container (default `/repo`) |
| `Targets[].Docker.ExtraDockerArgs` | Extra raw `docker run` args, e.g. `["--platform", "linux/amd64"]` on ARM hosts, or `["--memory", "8g"]` |
| `Targets[].ExtraCMakeArgs` | Extra `-D...` flags, e.g. `"-DGDK_SCARLETT=OFF"` for Xbox One instead of Series X\|S, or `"-DENABLE_PTHREADS=ON"` |
| `Targets[].ExtraEnvCommands` | Raw lines injected into the generated script before the cmake calls — an escape hatch for anything machine-specific this tool doesn't model (e.g. `apt-get update && apt-get install -y <package>` inside the Linux container, if you ever need something the sniper SDK doesn't already ship) |

## Notes / things worth knowing

- **GDK targets can't run on hosted CI** (GitHub-hosted runners, etc.) — the GDK is licensed
  and only installable on a machine you control. Windows, Linux (Docker), and Emscripten all
  work fine on a self-hosted or hosted Windows runner with Docker available.
- **Parallel mode** runs targets as separate processes with separate build directories, so it's
  safe correctness-wise — the tradeoff is CPU/RAM contention between simultaneous compiles.
  Start with `MaxParallelism: 2` and see how your machine handles it.
- **Container runs as root by default.** Fine for the build itself; if you care about file
  ownership on the host afterward, add `"--user", "<uid>:<gid>"` to `Docker.ExtraDockerArgs`
  (mostly relevant if you ever run this orchestrator from a Linux host instead of Windows).
- **FMOD** (and other prebuilt binary vendored libs) ship prebuilt for specific ABIs. This
  tool doesn't touch that — it just invokes the same toolchains the repo already documents —
  but if you ever swap `msvc.cmake` for a different compiler on Windows, watch for ABI
  mismatches against those prebuilt libs.
