from PIL import Image
from pathlib import Path

GEN = Path(__file__).parent / "generated"
for filename in GEN.glob("*.py"):
    filename.unlink()
def gen_file(subcategory, filename: str):
    BASE = Path(__file__).parent
    INPUT = BASE / "pics" / subcategory / filename
    OUTPUT = BASE / "generated" / (filename.removesuffix(".png") + ".py").replace("-", "_")

    img = Image.open(INPUT).convert("RGBA")
    width, height = img.size

    pixels = bytearray()

    for y in range(height):
        for x in range(width):
            r, g, b, a = img.getpixel((x, y))
            color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            pixels.append((color >> 8) & 0xFF)
            pixels.append(color & 0xFF)

    mask = bytearray()
    for y in range(height):
        for x in range(width):
            mask.append(1 if img.getpixel((x, y))[3] > 0 else 0)

    with open(OUTPUT, "w") as f:
        f.write(f"WIDTH = {width}\n")
        f.write(f"HEIGHT = {height}\n")
        f.write(f"PIXELS = bytearray({list(pixels)})\n")
        f.write(f"MASK = bytearray({list(mask)})\n")

BASE = Path(__file__).parent
PICS = BASE / "pics"
if input("Continue?"):
    exit(0)
names = []
for filename in PICS.rglob("*.png"):
    subcategory = filename.parent.name
    if filename.parent.parent.name != "pics":
        subcategory = f"{filename.parent.parent.name}/{filename.parent.name}"
    gen_file(subcategory, filename.name)
    names.append(filename.name.removesuffix(".png").replace("-", "_"))
with open(BASE / "generated" / "sprites.py", "w") as f:
    f.write(f"LIST = {names}")
