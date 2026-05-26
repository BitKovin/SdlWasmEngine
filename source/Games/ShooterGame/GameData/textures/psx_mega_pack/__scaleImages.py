from pathlib import Path
import tkinter as tk
import os

# Image formats usually supported by Tkinter
SUPPORTED_EXTS = {".png", ".gif", ".ppm", ".pgm"}

def main():
    try:
        factor = int(input("Enter downscale factor (integer >= 2): ").strip())
        if factor < 2:
            raise ValueError
    except ValueError:
        print("Invalid factor.")
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
            img = tk.PhotoImage(file=str(path))

            # Scale down
            scaled = img.subsample(factor, factor)

            # Temporary file
            temp_path = path.with_name(path.stem + "_temp.png")

            # Save scaled image
            scaled.write(str(temp_path), format="png")

            # Replace original
            os.remove(path)
            os.rename(temp_path, path.with_suffix(".png"))

            print(f"Replaced: {path.name}")
            processed += 1

        except Exception as e:
            print(f"Skipped {path.name}: {e}")
            skipped += 1

    root.destroy()

    print(f"\nDone.")
    print(f"Processed: {processed}")
    print(f"Skipped: {skipped}")

if __name__ == "__main__":
    main()