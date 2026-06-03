from pathlib import Path
import math
import os
import tkinter as tk

SUPPORTED_EXTS = {".png", ".gif", ".ppm", ".pgm"}


def srgb_to_linear(c: float) -> float:
    c = c / 255.0
    if c <= 0.04045:
        return c / 12.92
    return ((c + 0.055) / 1.055) ** 2.4


def linear_to_srgb(c: float) -> int:
    c = max(0.0, min(1.0, c))
    if c <= 0.0031308:
        v = 12.92 * c
    else:
        v = 1.055 * (c ** (1 / 2.4)) - 0.055
    return max(0, min(255, int(round(v * 255.0))))


def parse_pixel(value):
    # Tk may return a tuple, list, or a hex color string depending on build/image type.
    if isinstance(value, (tuple, list)):
        if len(value) >= 3:
            return int(value[0]), int(value[1]), int(value[2])
        if len(value) == 1:
            v = int(value[0])
            return v, v, v

    if isinstance(value, str):
        s = value.strip()
        if s.startswith("#") and len(s) == 7:
            return int(s[1:3], 16), int(s[3:5], 16), int(s[5:7], 16)
        parts = s.split()
        if len(parts) >= 3:
            return int(parts[0]), int(parts[1]), int(parts[2])

    raise ValueError(f"Unsupported pixel value: {value!r}")


def brighten_image(img: tk.PhotoImage, factor: float) -> tk.PhotoImage:
    w, h = img.width(), img.height()
    out = tk.PhotoImage(width=w, height=h)

    for y in range(h):
        for x in range(w):
            r, g, b = parse_pixel(img.get(x, y))

            lr = srgb_to_linear(r) * factor
            lg = srgb_to_linear(g) * factor
            lb = srgb_to_linear(b) * factor

            nr = linear_to_srgb(lr)
            ng = linear_to_srgb(lg)
            nb = linear_to_srgb(lb)

            out.put(f"#{nr:02x}{ng:02x}{nb:02x}", (x, y))

    return out


def main():
    try:
        factor = float(input("Brightness factor (1.0 = unchanged, 1.2 = brighter, 0.8 = darker): ").strip())
        if factor <= 0:
            raise ValueError
    except ValueError:
        print("Please enter a positive number.")
        return

    folder = Path(__file__).resolve().parent

    root = tk.Tk()
    root.withdraw()

    processed = 0
    skipped = 0

    for path in folder.iterdir():
        if not path.is_file() or path.suffix.lower() not in SUPPORTED_EXTS:
            continue

        try:
            img = tk.PhotoImage(file=str(path))
            brightened = brighten_image(img, factor)

            temp_path = path.with_name(f"{path.stem}.__tmp__{path.suffix}")
            fmt = path.suffix.lstrip(".").lower()

            brightened.write(str(temp_path), format=fmt)
            os.replace(temp_path, path)

            print(f"Updated: {path.name}")
            processed += 1
        except Exception as e:
            print(f"Skipped {path.name}: {e}")
            skipped += 1

    root.destroy()
    print(f"\nDone. Processed: {processed}, skipped: {skipped}")


if __name__ == "__main__":
    main()