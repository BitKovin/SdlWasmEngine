#!/usr/bin/env python3

import sys
import shutil
import subprocess
from pathlib import Path
import zipfile


SIZE_THRESHOLD = 10 * 1024 * 1024      # 10 MB
SAVE_THRESHOLD = 5 * 1024 * 1024      # 5 MB
MAX_UNPACK_ITERATIONS = 10

GLBCOMPRESS_ARGS = [
    "--max-texture-size", "2048",
    "--jpeg-quality", "50",
    "--png-to-jpeg-if-opaque",
    "--remove-unused",
]


def read_ignore_file(path):
    ignored = set()

    if path.exists():
        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()

                if line:
                    ignored.add(line)

    return ignored


def should_ignore_dir(dir_path):
    return (dir_path / ".ignoreFolder").exists()


def get_dir_size(path):
    total = 0

    for p in path.rglob("*"):
        if p.is_file():
            total += p.stat().st_size

    return total


def zip_folder(folder_path):
    zip_path = folder_path.with_suffix(".zip")

    with zipfile.ZipFile(
        zip_path,
        "w",
        compression=zipfile.ZIP_DEFLATED
    ) as zf:

        for file in folder_path.rglob("*"):
            if file.is_file():
                # include the folder itself in archive
                arcname = file.relative_to(folder_path.parent)
                zf.write(file, arcname)

    return zip_path


def maybe_compress(folder_path):
    size_before = get_dir_size(folder_path)

    if size_before < SIZE_THRESHOLD:
        return

    zip_path = zip_folder(folder_path)
    size_after = zip_path.stat().st_size

    saved = size_before - size_after

    if saved >= SAVE_THRESHOLD:
        shutil.rmtree(folder_path)

        print(
            f"Compressed: {folder_path} -> "
            f"{zip_path} (saved {saved // (1024 * 1024)} MB)"
        )

    else:
        zip_path.unlink()

        print(f"Skipped compression (not worth it): {folder_path}")


def process_directory(src, dst):
    if should_ignore_dir(src):
        return

    dst.mkdir(parents=True, exist_ok=True)

    ignore_file = src / ".ignoreFiles"
    blacklist = read_ignore_file(ignore_file)

    for item in src.iterdir():

        if item.name in [".ignoreFiles", ".ignoreFolder"]:
            continue

        if item.name in blacklist:
            continue

        target = dst / item.name

        if item.is_dir():
            process_directory(item, target)
        else:
            shutil.copy2(item, target)


def unpack_zip(zip_path):
    extract_dir = zip_path.parent

    try:
        with zipfile.ZipFile(zip_path, "r") as zf:
            zf.extractall(extract_dir)

        zip_path.unlink()

        print(f"Extracted: {zip_path}")

        return True

    except zipfile.BadZipFile:
        print(f"Bad zip file: {zip_path}")

    except Exception as e:
        print(f"Failed to extract {zip_path}: {e}")

    return False


def unpack_all_zips(root):
    """
    Recursively unpack zip files.
    Repeats multiple passes to handle nested zips.
    """

    for iteration in range(MAX_UNPACK_ITERATIONS):

        zip_files = list(root.rglob("*.zip"))

        if not zip_files:
            break

        print(
            f"Unpack iteration "
            f"{iteration + 1}: {len(zip_files)} zip(s)"
        )

        extracted_any = False

        for zip_file in zip_files:
            if unpack_zip(zip_file):
                extracted_any = True

        if not extracted_any:
            break


def find_glbcompress_exe():
    exe_path = Path(__file__).resolve().parent / "glbcompress.exe"

    if not exe_path.exists():
        print(f"Warning: glbcompress.exe not found at {exe_path}")
        print("Skipping GLB optimization.")
        return None

    return exe_path


def optimize_glb_file(glb_path, exe_path):
    # Unlikely-to-collide temp name, written alongside the original,
    # then swapped in on success.
    tmp_output = glb_path.with_name(glb_path.stem + ".__glbcompress_tmp__.glb")

    cmd = [
        str(exe_path),
        "compress",
        str(glb_path),
        "-o", str(tmp_output),
    ] + GLBCOMPRESS_ARGS

    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
        )

    except Exception as e:
        print(f"Failed to run glbcompress on {glb_path}: {e}")
        return

    if result.returncode != 0:
        print(f"glbcompress failed on {glb_path}:")
        print(result.stderr.strip() or result.stdout.strip())

        if tmp_output.exists():
            tmp_output.unlink()

        return

    if not tmp_output.exists():
        print(
            f"glbcompress reported success but produced no output "
            f"for {glb_path}"
        )
        return

    size_before = glb_path.stat().st_size
    size_after = tmp_output.stat().st_size

    # Replace the original with the optimized version
    tmp_output.replace(glb_path)

    saved = size_before - size_after

    print(
        f"Optimized: {glb_path} "
        f"({size_before // 1024} KB -> {size_after // 1024} KB, "
        f"saved {saved // 1024} KB)"
    )


def optimize_all_glb_files(root):
    exe_path = find_glbcompress_exe()

    if exe_path is None:
        return

    glb_files = list(root.rglob("*.glb"))

    if not glb_files:
        return

    print(f"Optimizing {len(glb_files)} .glb file(s)...")

    for glb_file in glb_files:
        optimize_glb_file(glb_file, exe_path)


def compress_top_level_dirs(output_root):
    for item in output_root.iterdir():
        if item.is_dir():
            maybe_compress(item)


def main():

    if len(sys.argv) != 3:
        print("Usage: finalize_gamedata.py <input> <output>")
        sys.exit(1)

    src = Path(sys.argv[1]).resolve()
    dst = Path(sys.argv[2]).resolve()

    if not src.exists():
        print(f"Input path does not exist: {src}")
        sys.exit(1)

    if dst.exists():
        shutil.rmtree(dst)

    # Step 1: copy everything
    process_directory(src, dst)

    # Step 2: unpack nested zip files (so any .glb inside an archive
    # becomes a plain file we can reach)
    unpack_all_zips(dst)

    # Step 3: optimize every .glb file in place, in or out of archives
    optimize_all_glb_files(dst)

    # Step 4: compress resulting folders
    compress_top_level_dirs(dst)

    print(f"Done: {dst}")


if __name__ == "__main__":
    main()