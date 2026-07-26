using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

class ShaderCompiler
{
    static void Main(string[] args)
    {
        string shadercPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "shaderc.exe");
        if (!File.Exists(shadercPath)) { Console.WriteLine("ERROR: shaderc.exe not found!"); return; }

        string inputFolder = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "source");
        if (!Directory.Exists(inputFolder)) { Console.WriteLine("ERROR: source folder not found!"); return; }

        string varyingDefPath = Path.Combine(inputFolder, "varying.def.sc");
        if (!File.Exists(varyingDefPath)) { Console.WriteLine("ERROR: varying.def.sc not found in source folder!"); return; }

        string FindNearestVarying(string shaderDir)
        {
            if (string.IsNullOrEmpty(shaderDir)) return varyingDefPath;
            string rootFull = Path.GetFullPath(inputFolder).TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
            string dir = Path.GetFullPath(shaderDir).TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
            while (!string.IsNullOrEmpty(dir))
            {
                string candidate = Path.Combine(dir, "varying.def.sc");
                if (File.Exists(candidate)) return candidate;
                if (string.Equals(dir, rootFull, StringComparison.OrdinalIgnoreCase)) break;
                DirectoryInfo? parent = Directory.GetParent(dir);
                if (parent == null) break;
                dir = parent.FullName.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
            }
            return varyingDefPath;
        }

        string outputBase = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "compiled");

        // -----------------------------------------------------------------------
        // Shader types that produce DIFFERENT binaries per platform.
        // Everything else is compiled once for the first target in its group
        // and the resulting .bin is copied to all other matching platform folders.
        // -----------------------------------------------------------------------
        var platformSpecificTypes = new HashSet<string>(StringComparer.OrdinalIgnoreCase)
        {
            // nothing here currently — all types produce platform-independent output
        };

        var targets = new (string folder, string platform, string type, string vertexProfile, string fragmentProfile)[]
        {
            // Windows — dx12 shares the dx11 binary
            ("windows/dx11",  "windows", "dx11",  "s_5_0",  "s_5_0"),
            ("windows/dx12",  "windows", "dx11",  "s_5_0",  "s_5_0"),  // ← same type key as above
            ("windows/gl",    "windows", "gl",    "140",    "140"),
            ("windows/spirv", "windows", "spirv", "spirv",  "spirv"),
            // Linux
            ("linux/gl",      "linux",   "gl",    "140",    "140"),
            ("linux/spirv",   "linux",   "spirv", "spirv",  "spirv"),

            // Android
            ("android/gles",  "android", "glsl",  "300_es", "300_es"),
            ("android/spirv", "android", "spirv", "spirv",  "spirv"),

            // Web
            ("web/gles",      "html5",   "glsl",  "300_es", "300_es"),
            ("web/spirv",     "html5",   "spirv", "spirv",  "spirv"),
        };

        // Group targets whose (type + profiles) produce identical output.
        // Platform-independent types share a single compile job; others get one job each.
        var targetGroups = targets
            .GroupBy(t => (t.type, t.vertexProfile, t.fragmentProfile))
            .ToList();

        string[] shaderFiles = Directory.GetFiles(inputFolder, "*.sc", SearchOption.AllDirectories);

        // Separate compile jobs from copy jobs up front
        var compileJobs = new List<(
            string shaderFile,
            string shaderName,
            string relativeDir,
            (string folder, string platform, string type, string vertexProfile, string fragmentProfile) target)>();

        var copyJobs = new List<(
            string shaderFile,
            string shaderName,
            string relativeDir,
            (string folder, string platform, string type, string vertexProfile, string fragmentProfile) sourceTarget,
            (string folder, string platform, string type, string vertexProfile, string fragmentProfile) destTarget)>();

        foreach (var shaderFile in shaderFiles)
        {
            string shaderName = Path.GetFileName(shaderFile);
            if (shaderName.Equals("varying.def.sc", StringComparison.OrdinalIgnoreCase)) continue;

            if (!shaderName.StartsWith("vs_") && !shaderName.StartsWith("fs_") && !shaderName.StartsWith("cs_"))
            {
                Console.WriteLine($"WARNING: Cannot determine shader type for {shaderName}, skipping.");
                continue;
            }

            string relativeDir = Path.GetRelativePath(inputFolder, Path.GetDirectoryName(shaderFile)!);

            foreach (var group in targetGroups)
            {
                var groupTargets = group.ToList();
                bool isPlatformSpecific = platformSpecificTypes.Contains(group.Key.type);

                if (isPlatformSpecific)
                {
                    // Each platform target gets its own compile job
                    foreach (var t in groupTargets)
                        compileJobs.Add((shaderFile, shaderName, relativeDir, t));
                }
                else
                {
                    // Compile once against the first target, copy the result to the rest
                    var primary = groupTargets[0];
                    compileJobs.Add((shaderFile, shaderName, relativeDir, primary));
                    foreach (var other in groupTargets.Skip(1))
                        copyJobs.Add((shaderFile, shaderName, relativeDir, primary, other));
                }
            }
        }

        Console.WriteLine($"Found {shaderFiles.Length} shader file(s).");
        Console.WriteLine($"Compile jobs : {compileJobs.Count}");
        Console.WriteLine($"Copy jobs    : {copyJobs.Count}\n");

        int succeeded = 0, failed = 0, copied = 0, copySkipped = 0;
        object consoleLock = new object();

        // ── Phase 1: compile ────────────────────────────────────────────────────
        Parallel.ForEach(
            compileJobs,
            new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount },
            job =>
            {
                var (shaderFile, shaderName, relativeDir, target) = job;

                string subPath = relativeDir == "." ? target.folder : Path.Combine(target.folder, relativeDir);
                string outDir = Path.Combine(outputBase, subPath);
                Directory.CreateDirectory(outDir);

                string outFile = Path.Combine(outDir, Path.GetFileNameWithoutExtension(shaderFile) + ".bin");
                string typeFlag = shaderName.StartsWith("vs_") ? "v" : shaderName.StartsWith("fs_") ? "f" : "c";
                string profile = (typeFlag == "v") ? target.vertexProfile : target.fragmentProfile;
                string varyingToUse = FindNearestVarying(Path.GetDirectoryName(shaderFile)!);

                string argsStr = $"-f \"{shaderFile}\" -o \"{outFile}\" --type {typeFlag} " +
                                 $"--platform {target.platform} --profile {profile} " +
                                 $"--varyingdef \"{varyingToUse}\"";

                var stdoutLines = new List<string>();
                var stderrLines = new List<string>();

                var proc = new Process();
                proc.StartInfo.FileName = shadercPath;
                proc.StartInfo.Arguments = argsStr;
                proc.StartInfo.RedirectStandardOutput = true;
                proc.StartInfo.RedirectStandardError = true;
                proc.StartInfo.UseShellExecute = false;
                proc.StartInfo.CreateNoWindow = true;

                proc.OutputDataReceived += (_, e) => { if (e.Data != null) lock (stdoutLines) stdoutLines.Add(e.Data); };
                proc.ErrorDataReceived += (_, e) => { if (e.Data != null) lock (stderrLines) stderrLines.Add(e.Data); };

                proc.Start();
                proc.BeginOutputReadLine();
                proc.BeginErrorReadLine();
                proc.WaitForExit();

                bool ok = proc.ExitCode == 0;

                lock (consoleLock)
                {
                    if (ok)
                    {
                        Console.WriteLine($"[COMPILED] {target.folder} -> {Path.GetRelativePath(outputBase, outFile)} (varying: {Path.GetRelativePath(inputFolder, varyingToUse)})");
                        succeeded++;
                    }
                    else
                    {
                        Console.WriteLine($"[FAILED]   {target.folder} -> {shaderName} (varying: {Path.GetRelativePath(inputFolder, varyingToUse)})");
                        foreach (var line in stdoutLines) Console.WriteLine($"           {line}");
                        foreach (var line in stderrLines) Console.WriteLine($"           {line}");
                        failed++;
                    }
                }
            });

        // ── Phase 2: copy ───────────────────────────────────────────────────────
        Parallel.ForEach(
            copyJobs,
            new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount },
            job =>
            {
                var (shaderFile, shaderName, relativeDir, sourceTarget, destTarget) = job;

                string srcSubPath = relativeDir == "." ? sourceTarget.folder : Path.Combine(sourceTarget.folder, relativeDir);
                string srcFile = Path.Combine(outputBase, srcSubPath, Path.GetFileNameWithoutExtension(shaderFile) + ".bin");

                string dstSubPath = relativeDir == "." ? destTarget.folder : Path.Combine(destTarget.folder, relativeDir);
                string dstDir = Path.Combine(outputBase, dstSubPath);
                string dstFile = Path.Combine(dstDir, Path.GetFileNameWithoutExtension(shaderFile) + ".bin");

                lock (consoleLock)
                {
                    if (File.Exists(srcFile))
                    {
                        Directory.CreateDirectory(dstDir);
                        File.Copy(srcFile, dstFile, overwrite: true);
                        Console.WriteLine($"[COPIED]   {sourceTarget.folder} -> {destTarget.folder}/{Path.GetFileName(dstFile)}");
                        copied++;
                    }
                    else
                    {
                        // Source failed to compile — nothing to copy
                        Console.WriteLine($"[SKIPPED]  {destTarget.folder} -> {shaderName} (source compile failed)");
                        copySkipped++;
                    }
                }
            });

        Console.WriteLine($"\nShader compilation done!  {succeeded} compiled, {copied} copied, {failed} failed, {copySkipped} copies skipped.");
        Console.ReadLine();
    }
}