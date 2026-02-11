using System;
using System.IO;
using System.IO.Compression;
using System.Threading;

//packs .bsp files and their corresponding folders into a .zip file, then deletes the original .bsp file and folder
//used to compress data inside of bsp file, since light volume data is not compressed inside of bsp file

class Program
{
    static void Main(string[] args)
    {
        if (args.Length != 1)
        {
            Console.WriteLine("Usage: Program.exe <file_path>");
            return;
        }

        string filePath = args[0];

        if (!File.Exists(filePath))
        {
            Console.WriteLine("File does not exist.");
            return;
        }

        try
        {
            string directory = Path.GetDirectoryName(filePath);
            string fileNameWithoutExt = Path.GetFileNameWithoutExtension(filePath);
            string zipFilePath = Path.Combine(directory, fileNameWithoutExt + ".zip");
            string folderPath = Path.Combine(directory, fileNameWithoutExt);

            // Delete existing zip if it exists
            if (File.Exists(zipFilePath))
            {
                File.Delete(zipFilePath);
            }

            using (var zipArchive = ZipFile.Open(zipFilePath, ZipArchiveMode.Create))
            {
                // Add the file to the zip
                zipArchive.CreateEntryFromFile(filePath, Path.GetFileName(filePath));

                // If the folder exists, add its contents to the zip
                if (Directory.Exists(folderPath))
                {
                    AddDirectoryToZip(zipArchive, folderPath, fileNameWithoutExt + "/");
                }
            } // Ensure archive is fully disposed here

            // Give the system a moment to release file handles
            Thread.Sleep(100);

            // Delete the original file with retry logic
            int retries = 3;
            for (int i = 0; i < retries; i++)
            {
                try
                {
                    File.Delete(filePath);
                    break;
                }
                catch (IOException)
                {
                    if (i == retries - 1) throw;
                    Thread.Sleep(100);
                }
            }

            // If the folder exists, delete it recursively
            if (Directory.Exists(folderPath))
            {
                Directory.Delete(folderPath, true);
            }

            Console.WriteLine("File and folder compressed, originals deleted successfully.");
        }
        catch (Exception ex)
        {
            Console.WriteLine("An error occurred: " + ex.Message);
        }
    }

    private static void AddDirectoryToZip(ZipArchive zipArchive, string sourceDir, string entryPrefix)
    {
        foreach (var file in Directory.GetFiles(sourceDir))
        {
            zipArchive.CreateEntryFromFile(file, entryPrefix + Path.GetFileName(file));
        }

        foreach (var subDir in Directory.GetDirectories(sourceDir))
        {
            AddDirectoryToZip(zipArchive, subDir, entryPrefix + Path.GetFileName(subDir) + "/");
        }
    }
}