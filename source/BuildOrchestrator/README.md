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
| GameData pre-step | **`cmake`** on the bare host PATH (outside any VS Dev Prompt / container / emsdk activation) — needed to run `RunGameDataFinalize.cmake` before any platform build starts. See "GameData finalization" below; set `FinalizeGameDataOnce: false` to skip this requirement. |

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
   - Flip `enabled: true` on `xbox-gdk` / `windows-gdk` once GDK is installed, and fix
     `gdk.varsScriptPath` / `gdk.flavor` to match your install — see "GDK configuration" below.

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

## GameData finalization

By default (`FinalizeGameDataOnce: true`), the tool runs `BuildScripts/RunGameDataFinalize.cmake`
once — as the actual first step, before any platform build starts — producing a single shared
`Build/<GameName>/GameData`, and automatically passes `-DSKIP_GAMEDATA_FINALIZE=1` to every
target so none of them redundantly re-run the (potentially slow, GLB-compressing) finalize
pipeline themselves:

- **Emscripten** preloads directly from the shared location (`cmake/platforms/emscripten.cmake`) —
  no copy. This also fixes a latent ordering issue in the engine's original per-platform design:
  `--preload-file` needs GameData to exist *at link time*, but the old per-platform finalize only
  ran in `POST_BUILD`, strictly *after* linking.
- **Every other platform** gets a cheap `POST_BUILD` directory copy from the shared location into
  its own `Build/<GameName>/<platform>/GameData` instead — no Python, no re-finalizing.

This requires `cmake` and a `python3`/`python` interpreter on the host PATH (the same two tools
a standalone build already needs). Set `FinalizeGameDataOnce: false` to fall back to the engine's
original behavior — each target finalizes its own GameData independently, no shared step. A
single target can also opt back out on its own by adding `"-DSKIP_GAMEDATA_FINALIZE=0"` to its
`ExtraCMakeArgs` — CMake takes the last `-D` on the command line, so a per-target override always
wins over the auto-injected flag.

The pre-step shows up as a `gamedata` entry in the console output, `BuildLogs/gamedata.log`, and
`summary.json`, same as any other target. If it fails, the run aborts immediately — no platform
build is attempted, since every target depends on it having succeeded.

**Change detection**: `finalize_gamedata.py` itself now fingerprints the source `GameData` tree
(file paths, sizes, and mtimes — not full content hashing, which would be slow for large binary
assets) and skips all work — copying, zip unpacking, GLB optimization, compression — if nothing
changed since the last successful run. This applies whether it's invoked by this tool, by
`RunGameDataFinalize.cmake` directly, or by a standalone per-platform build. Pass `--force-gamedata`
to this tool (or `--force` directly to the python script, or `-DFORCE_GAMEDATA=1` to the cmake
script) to bypass the check and reprocess anyway.

## GDK configuration

Xbox GDK and Windows GDK targets don't use `vcvarsall.bat` at all. Instead they activate through
the GDK's own "vars" script, which sets up **both** the GDK environment and the matching VS/MSVC
compiler environment in one call — this is the same script the GDK's own Start Menu shortcuts run
(e.g. a "Gaming Desktop x64" shortcut launches
`cmd.exe /k "...\Microsoft GDK\Command Prompts\GamingDesktopVars.cmd" GamingDesktopVS2022`, minus
the `/k` since we don't want an interactive shell left open).

```json
"Gdk": {
  "VarsScriptPath": "C:\\Program Files (x86)\\Microsoft GDK\\Command Prompts\\GamingDesktopVars.cmd",
  "Flavor": "GamingDesktopVS2022"
}
```

- **Windows GDK** — confirmed against a real install: `GamingDesktopVars.cmd` + `GamingDesktopVS2022`.
- **Xbox GDK** — same mechanism, different script (`GamingXboxVars.cmd`) and flavor name, but the
  exact accepted flavor string depends on your GDK edition: older editions use `GamingXboxVS2022`;
  newer "New layout" editions use `GamingXboxGen8VS2022` (Xbox One) or `GamingXboxGen9VS2022`
  (Series X\|S) instead. The shipped default is a best-effort guess — if it doesn't match, run the
  vars script with no arguments (or check the GDK's own Start Menu shortcuts) to see which flavor
  strings your install actually accepts, and update `Flavor` accordingly.

Both scripts live under your GDK's own `Command Prompts` folder — if you're using an unpacked/
portable GDK instead of the installed one, point `VarsScriptPath` at the equivalent script there.

## Linux build performance

A few things specifically help iteration speed on the Linux/sniper leg, which is the slowest by
a wide margin the first time:

- **Generator**: the shipped default is `Ninja` (confirmed preinstalled in the sniper SDK) rather
  than `Unix Makefiles` — Ninja's configure/generate and its no-op-rebuild check are both
  meaningfully faster than Makefiles for a project with this many vendored sub-libraries.
- **CMake bootstrap caching**: the default `ExtraEnvCommands` (added to work around sniper's
  CMake 3.25 — see "Notes" below) now checks `/cache/cmake/bin/cmake` before downloading anything,
  and `/cache` is a named Docker volume (`docker.cacheVolumeName`, default
  `rengine-linux-build-cache`) that persists across builds even though each build runs in a fresh
  `docker run --rm` container. Without this, the ~30MB CMake download+extract reran on *every
  single build*, changed code or not.
- **Configure skipping**: every target — not just Linux — now skips the full `cmake -S -B ...`
  reconfigure entirely when the build directory already has a matching configure from last time
  (tracked via a small fingerprint file next to `CMakeCache.txt`, covering the toolchain file,
  generator, `ExtraCMakeArgs`, game name, and configuration). It goes straight to `cmake --build`,
  which still runs CMake's own lightweight internal "does anything actually need reconfiguring"
  check — just not the full, expensive command-line reconfigure every time. Changing
  `ExtraCMakeArgs`/`ToolchainFile`/`Generator`/`GameName` in the config is detected automatically
  and triggers a fresh reconfigure; `--clean` always forces one too.

None of this touches the GameData caching described above — that's a separate fingerprint, over
the actual game asset tree.

## CLI reference

```
rengine-build [options]

  --config <path>   Path to the JSON target config (default: build-targets.json)
  --init             Write a starter build-targets.json next to the exe and exit
  --list             List configured targets (enabled/disabled) and exit
  --only a,b,c       Build only the named targets
  --clean            Delete each target's build directory before configuring
  --force-gamedata   Bypass GameData's unchanged-skip check and reprocess it anyway
  --dry-run          Print the generated build script for each target without running it
  --parallel         Build independent targets concurrently (overrides config)
  --sequential       Build targets one at a time (overrides config)
  -h, --help         Show this help
```

Exit code is `0` only if every target succeeded, so it's CI-friendly as-is.

## What actually happens per target

- **Windows** — a generated `.bat` runs `vcvarsall.bat`, then
  `cmake -S ... -B ... -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/msvc.cmake ...`,
  then `cmake --build`.
- **Xbox GDK / Windows GDK** — same shape, but activates via the GDK's own vars script
  (`GamingXboxVars.cmd` / `GamingDesktopVars.cmd` + a flavor argument) instead of `vcvarsall.bat` —
  see "GDK configuration" above. Adds `-DXBOX_GDK=1` / `-DWINDOWS_GDK=1`.
- **Linux** — `RepoRoot` is bind-mounted into a `sniper/sdk` container at `/repo` (configurable),
  and a generated `.sh` runs inside it via `docker run --rm -v <repo>:/repo -w /repo -v
  <cache-volume>:/cache <image> bash /repo/.build-orchestrator/linux/build.sh`, using
  `cmake/toolchains/clang.cmake`. Because the mount point is a plain bind-mount, build output lands
  right back in `<RepoRoot>/build/linux` on the host — no copy-out step. Container paths are built
  with plain string concatenation (no runtime path-translation step needed, unlike WSL's `wslpath`).
- **Emscripten** — a generated `.bat` calls `emsdk_env.bat`, then `emcmake cmake ...` (no
  `CMAKE_TOOLCHAIN_FILE` needed — `emcmake` sets the compiler itself, per the repo's own comments).

Every target also skips the full reconfigure step when the build directory's already up to date
with the same effective arguments — see "Linux build performance" above (it applies to every
target, not just Linux).

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
| `FinalizeGameDataOnce` | Finalize GameData once, centrally, before any platform build (default `true`). See "GameData finalization" below |
| `Targets[].Kind` | `Windows`, `Linux`, `Emscripten`, `XboxGdk`, or `WindowsGdk` |
| `Targets[].BuildDir` | CMake build dir, relative to `RepoRoot`; must be unique per target |
| `Targets[].Docker.Image` | Sniper (or other) image the Linux target builds inside |
| `Targets[].Docker.ContainerRepoPath` | Where `RepoRoot` is bind-mounted inside the container (default `/repo`) |
| `Targets[].Docker.CacheVolumeName` / `CacheMountPath` | Named Docker volume mounted alongside the repo, persisted across `--rm` container runs — used by the default CMake bootstrap so it doesn't re-download every build. Set `CacheVolumeName` empty to disable the mount |
| `Targets[].Docker.ExtraDockerArgs` | Extra raw `docker run` args, e.g. `["--platform", "linux/amd64"]` on ARM hosts, or `["--memory", "8g"]` |
| `Targets[].Gdk.VarsScriptPath` / `Flavor` | Xbox GDK / Windows GDK only — path to `GamingXboxVars.cmd`/`GamingDesktopVars.cmd` and its flavor argument. See "GDK configuration" above |
| `Targets[].ExtraCMakeArgs` | Extra `-D...` flags, e.g. `"-DGDK_SCARLETT=OFF"` for Xbox One instead of Series X\|S, or `"-DENABLE_PTHREADS=ON"` |
| `Targets[].ExtraEnvCommands` | Raw lines injected into the generated script before the cmake calls — an escape hatch for anything machine-specific this tool doesn't model (e.g. `apt-get update && apt-get install -y <package>` inside the Linux container, if you ever need something the sniper SDK doesn't already ship) |

## Notes / things worth knowing

- **sniper ships CMake 3.25** — older than some generator expressions REngine's vendored
  dependencies use (e.g. openAL-soft's `$<BUILD_LOCAL_INTERFACE:...>`, added in CMake 3.26).
  The default config works around this by bootstrapping CMake 3.31.12 (the latest 3.x release)
  inside the container via `ExtraEnvCommands` before running `cmake` — deliberately *not* jumping
  to CMake 4.x, since CMake 4.0 dropped support for `cmake_minimum_required()` below 3.5, which
  risks breaking other older vendored libraries that haven't bumped their own minimum version.
  If you hit a similar "generator expression not recognized" error elsewhere, bump
  `CMAKE_VERSION` in that same `ExtraEnvCommands` block.
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
