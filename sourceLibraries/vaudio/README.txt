Vercidium Audio v1.2.1
======================

Raytraced audio SDK providing realistic muffling, reverb, ambience and visualisation.

Documentation: https://vercidium.com/docs
Report a bug or request a feature: https://github.com/vercidium-audio/support
Discord: https://discord.gg/dhdVRSeAKe


CONTENTS
--------

dotnet/
  dev/          C# SDK for development builds (includes rendering dependencies)
  production/   C# SDK for production/release builds

native/
  include/   vaudio.h — public header for the C SDK
  windows/   Windows DLL and import library (x64)
  linux/     Linux shared library (x64)
  mac/       macOS shared library (arm64)
  android/   Android shared libraries (arm64-v8a, armeabi-v7a, x86, x86_64)

wasm/
  vaudio-wrapper.js     JavaScript entry point
  vaudio-wrapper.d.ts   TypeScript declaration file
  _framework/           .NET WASM runtime and compiled assemblies (serve this folder statically alongside vaudio-wrapper.js)
