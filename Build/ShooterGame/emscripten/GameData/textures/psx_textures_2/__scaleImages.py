from pathlib import Path
import tkinter as tk
import math
import os

# Formats commonly supported by Tkinter
SUPPORTED_EXTS = {".png", ".gif", ".ppm", ".pgm"}


def bilinear_resize(img, scale):
    src_w = img.width()
    src_h = img.height()

    new_w = max(1, int(src_w * scale))
    new_h = max(1, int(src_h * scale))

    out = tk.PhotoImage(width=new_w, height=new_h)

    for y in range(new_h):
        for x in range(new_w):

            # Map destination pixel to source
            gx = x / scale
            gy = y / scale

            x0 = int(math.floor(gx))
            y0 = int(math.floor(gy))

            x1 = min(x0 + 1, src_w - 1)
            y1 = min(y0 + 1, src_h - 1)

            dx = gx - x0
            dy = gy - y0

            c00 = img.get(x0, y0)
            c10 = img.get(x1, y0)
            c01 = img.get(x0, y1)
            c11 = img.get(x1, y1)

            r = (
                c00[0] * (1 - dx) * (1 - dy)
                + c10[0] * dx * (1 - dy)
                + c01[0] * (1 - dx) * dy
                + c11[0] * dx * dy
            )

            g = (
                c00[1] * (1 - dx) * (1 - dy)
                + c10[1] * dx * (1 - dy)
                + c01[1] * (1 - dx) * dy
                + c11[1] * dx * dy
            )

            b = (
                c00[2] * (1 - dx) * (1 - dy)
                + c10[2] * dx * (1 - dy)
                + c01[2] * (1 - dx) * dy
                + c11[2] * dx * dy
            )

            color = f"#{int(r):02x}{int(g):02x}{int(b):02x}"
            out.put(color, (x, y))

    return out


def main():
    try:
        factor = float(input("Enter scale factor (example 0.5): ").strip())

        if factor <= 0 or factor >= 1:
            print("Use a value between 0 and 1.")
            return

    except ValueError:
        print("Invalid number.")
        return

    folder = Path(__file__).resolve().parent

    root = tk.Tk()
    root.withdraw()

    processed = 0
    skipped = 0

    for path in folder.iterdir():

        if not path.is_file():
            continue

        if path.suffix.lower() not in SUPPORTED_EXTS:
            continue

        try:
            print(f"Processing {path.name}...")

            img = tk.PhotoImage(file=str(path))

            resized = bilinear_resize(img, factor)

            temp_path = path.with_name(path.stem + "_temp.png")

            resized.write(str(temp_path), format="png")

            os.remove(path)
            os.rename(temp_path, path.with_suffix(".png"))

            processed += 1

        except Exception as e:
            print(f"Skipped {path.name}: {e}")
            skipped += 1

    root.destroy()

    print("\nDone.")
    print(f"Processed: {processed}")
    print(f"Skipped: {skipped}")


if __name__ == "__main__":
    main()