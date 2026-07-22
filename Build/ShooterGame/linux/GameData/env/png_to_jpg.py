from pathlib import Path
from PIL import Image

def convert_pngs_to_jpgs():
    folder = Path(__file__).resolve().parent

    for png_path in folder.glob("*.png"):
        jpg_path = png_path.with_suffix(".jpg")

        try:
            with Image.open(png_path) as img:
                # JPG doesn't support transparency, so flatten onto white.
                if img.mode in ("RGBA", "LA") or (img.mode == "P" and "transparency" in img.info):
                    rgba = img.convert("RGBA")
                    background = Image.new("RGB", rgba.size, (255, 255, 255))
                    background.paste(rgba, mask=rgba.getchannel("A"))
                    rgb_img = background
                else:
                    rgb_img = img.convert("RGB")

                rgb_img.save(jpg_path, "JPEG", quality=95)

            # Delete the original PNG only after successful conversion.
            png_path.unlink()

            print(f"Converted and deleted: {png_path.name} -> {jpg_path.name}")

        except Exception as e:
            print(f"Failed to convert {png_path.name}: {e}")

if __name__ == "__main__":
    convert_pngs_to_jpgs()