using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Threading.Tasks;

class ShaderCompiler
{
    static void Main(string[] args)
    {
        // Path to shaderc.exe (assumed in the same folder as this program)
        string shadercPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "shaderc.exe");
        if (!File.Exists(shadercPath))
        {
            Console.WriteLine("ERROR: shaderc.exe not found!");
            return;
        }

        // Input folder containing .sc files
        string inputFolder = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "source");
        if (!Directory.Exists(inputFolder))
        {
            Console.WriteLine("ERROR: source folder not found!");
            return;
        }

        // Path to default varying.def.sc (root of source)
        string varyingDefPath = Path.Combine(inputFolder, "varying.def.sc");
        if (!File.Exists(varyingDefPath))
        {
            Console.WriteLine("ERROR: varying.def.sc not found in source folder!");
            return;
        }

        // Local helper: find nearest varying.def.sc starting from shader dir and walking up to inputFolder.
        string FindNearestVarying(string shaderDir)
        {
            if (string.IsNullOrEmpty(shaderDir))
                return varyingDefPath;

            string rootFull = Path.GetFullPath(inputFolder).TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
            string dir = Path.GetFullPath(shaderDir).TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);

            while (!string.IsNullOrEmpty(dir))
            {
                string candidate = Path.Combine(dir, "varying.def.sc");
                if (File.Exists(candidate))
                    return candidate;

                // Stop if we've reached the source root
                if (string.Equals(dir, rootFull, StringComparison.OrdinalIgnoreCase))
                    break;

                DirectoryInfo? parent = Directory.GetParent(dir);
                if (parent == null)
                    break;

                dir = parent.FullName.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
            }

            // fallback to default root varying.def.sc
            return varyingDefPath;
        }

        // Output base folder
        string outputBase = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "compiled");

        // Platforms/backends
        var targets = new (string folder, string platform, string type, string vertexProfile, string fragmentProfile)[]
        {
            // -------------------
            // Windows
            // -------------------
            ("windows/dx11",  "windows", "dx11",  "s_5_0",   "s_5_0"),
            ("windows/dx12",  "windows", "dx12",  "s_5_0",   "s_5_0"),
            ("windows/gl",    "windows", "gl",    "140",     "140"),
            ("windows/spirv", "windows", "spirv", "spirv",   "spirv"),

            // -------------------
            // Linux
            // -------------------
            ("linux/gl",    "linux", "gl",    "140",   "140"),
            ("linux/spirv", "linux", "spirv", "spirv", "spirv"),

            // -------------------
            // Web
            // -------------------
            ("web/gles",  "html5", "glsl",  "300_es", "300_es"),
            ("web/spirv", "html5", "spirv", "spirv",  "spirv"),
        };

        // Find all .sc files recursively, excluding varying.def.sc
        string[] shaderFiles = Directory.GetFiles(inputFolder, "*.sc", SearchOption.AllDirectories);

        // Build the full list of compile jobs
        var jobs = new List<(string shaderFile, string shaderName, string relativeDir, (string folder, string platform, string type, string vertexProfile, string fragmentProfile) target)>();

        foreach (var shaderFile in shaderFiles)
        {
            string shaderName = Path.GetFileName(shaderFile);

            // Skip varying definition file
            if (shaderName.Equals("varying.def.sc", StringComparison.OrdinalIgnoreCase))
                continue;

            // Determine shader type by prefix
            string typeFlag;
            if (shaderName.StartsWith("vs_")) typeFlag = "v";
            else if (shaderName.StartsWith("fs_")) typeFlag = "f";
            else if (shaderName.StartsWith("cs_")) typeFlag = "c";
            else
            {
                Console.WriteLine($"WARNING: Cannot determine shader type for {shaderName}, skipping.");
                continue;
            }

            // Preserve subfolder structure in output by computing relative path from inputFolder
            string relativeDir = Path.GetRelativePath(inputFolder, Path.GetDirectoryName(shaderFile)!);

            foreach (var target in targets)
                jobs.Add((shaderFile, shaderName, relativeDir, target));
        }

        Console.WriteLine($"Compiling {shaderFiles.Length} shader(s) across {targets.Length} target(s) = {jobs.Count} total jobs...\n");

        // Track results thread-safely
        int succeeded = 0;
        int failed = 0;
        object consoleLock = new object();

        // Run all jobs in parallel (capped at logical CPU count)
        Parallel.ForEach(
            jobs,
            new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount },
            job =>
            {
                var (shaderFile, shaderName, relativeDir, target) = job;

                // Mirror subfolder structure: compiled/<target.folder>/<relativeDir>/
                string subPath = relativeDir == "." ? target.folder : Path.Combine(target.folder, relativeDir);
                string outDir = Path.Combine(outputBase, subPath);
                Directory.CreateDirectory(outDir);

                string outFile = Path.Combine(outDir, Path.GetFileNameWithoutExtension(shaderFile) + ".bin");

                string typeFlag = shaderName.StartsWith("vs_") ? "v"
                                : shaderName.StartsWith("fs_") ? "f"
                                : "c";

                string profile = (typeFlag == "v") ? target.vertexProfile : target.fragmentProfile;

                // Find nearest varying.def.sc for this shader (search shader folder -> parent -> ... -> source root)
                string shaderDir = Path.GetDirectoryName(shaderFile)!;
                string varyingToUse = FindNearestVarying(shaderDir);

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

                // Print output atomically so lines from parallel jobs don't interleave
                lock (consoleLock)
                {
                    if (ok)
                    {
                        Console.WriteLine($"[OK]     {target.folder} -> {Path.GetRelativePath(outputBase, outFile)} (varying: {Path.GetRelativePath(inputFolder, varyingToUse)})");
                        succeeded++;
                    }
                    else
                    {
                        Console.WriteLine($"[FAILED] {target.folder} -> {shaderName} (varying: {Path.GetRelativePath(inputFolder, varyingToUse)})");
                        foreach (var line in stdoutLines) Console.WriteLine($"         {line}");
                        foreach (var line in stderrLines) Console.WriteLine($"         {line}");
                        failed++;
                    }
                }
            });

        Console.WriteLine($"\nShader compilation done!  {succeeded} succeeded, {failed} failed.");
        Console.ReadLine();
    }
}