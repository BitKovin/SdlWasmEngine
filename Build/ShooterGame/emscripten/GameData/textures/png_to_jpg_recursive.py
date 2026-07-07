from pathlib import Path
import os
from PIL import Image

MAX_DEPTH = 10
MIN_SIZE = 10  # only process images strictly bigger than 10x10

# Paths are relative to the script folder.
# Add more entries here as needed.
IGNORE_DIRS = {
    "pp",
    "particles",
    "ui",
}

def is_ignored_dir(relative_dir: Path) -> bool:
    parts = relative_dir.parts
    if not parts:
        return False

    # Ignore if any path segment begins with one of the ignored top-level dirs
    # (e.g. "ui", "ui/something", "particles/effects", etc.)
    for ignored in IGNORE_DIRS:
        ignored_parts = Path(ignored).parts
        if parts[:len(ignored_parts)] == ignored_parts:
            return True
    return False

def has_any_transparency(img: Image.Image) -> bool:
    # Check alpha without scanning every pixel one by one manually.
    # If the alpha channel minimum is below 255, at least one pixel is transparent.
    if img.mode in ("RGBA", "LA"):
        return img.getchannel("A").getextrema()[0] < 255

    if img.mode == "P":
        if "transparency" in img.info:
            rgba = img.convert("RGBA")
            return rgba.getchannel("A").getextrema()[0] < 255
        return False

    if "transparency" in img.info:
        rgba = img.convert("RGBA")
        return rgba.getchannel("A").getextrema()[0] < 255

    return False

def convert_png_to_jpg(png_path: Path) -> None:
    jpg_path = png_path.with_suffix(".jpg")

    with Image.open(png_path) as img:
        width, height = img.size

        # Only process images bigger than 10x10.
        if width <= MIN_SIZE or height <= MIN_SIZE:
            print(f"Skipped small image: {png_path} ({width}x{height})")
            return

        # Skip if there is even a single transparent pixel.
        if has_any_transparency(img):
            print(f"Skipped transparent image: {png_path}")
            return

        rgb = img.convert("RGB")
        rgb.save(jpg_path, "JPEG", quality=95)

    png_path.unlink()
    print(f"Converted and deleted: {png_path} -> {jpg_path}")

def process_folder(root: Path) -> None:
    root = root.resolve()

    for dirpath, dirnames, filenames in os.walk(root):
        current_dir = Path(dirpath)
        relative_dir = current_dir.relative_to(root)

        # Stop descending past MAX_DEPTH.
        depth = len(relative_dir.parts)
        if depth >= MAX_DEPTH:
            dirnames[:] = []

        # Prune ignored directories from traversal.
        dirnames[:] = [
            d for d in dirnames
            if not is_ignored_dir(relative_dir / d)
        ]

        for filename in filenames:
            if filename.lower().endswith(".png"):
                png_path = current_dir / filename
                try:
                    convert_png_to_jpg(png_path)
                except Exception as e:
                    print(f"Failed: {png_path} ({e})")

if __name__ == "__main__":
    process_folder(Path(__file__).resolve().parent)