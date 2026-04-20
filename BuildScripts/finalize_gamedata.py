#!/usr/bin/env python3

import sys
import shutil
from pathlib import Path
import zipfile


SIZE_THRESHOLD = 20 * 1024 * 1024      # 20 MB
SAVE_THRESHOLD = 10 * 1024 * 1024      # 10 MB


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

    with zipfile.ZipFile(zip_path, 'w', compression=zipfile.ZIP_DEFLATED) as zf:
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
        print(f"Compressed: {folder_path} -> {zip_path} (saved {saved // (1024*1024)} MB)")
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

    process_directory(src, dst)

    # 🔥 compression step
    compress_top_level_dirs(dst)

    print(f"Done: {dst}")


if __name__ == "__main__":
    main()