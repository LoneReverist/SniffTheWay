"""Stage or apply the 1080p background conversion. Requires Pillow.

Run from any directory. Originals and previews stay in .codex-temp/backgrounds-1080p.
Only resources/textures supplies artwork; an existing backup is never overwritten.
"""
import argparse
import hashlib
import json
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, __version__ as pillow_version

ROOT = Path(__file__).resolve().parents[1]
TEXTURES = ROOT / "resources/textures"
WORK = ROOT / ".codex-temp/backgrounds-1080p"
TARGET = (1920, 1080)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def resize(image, method="lanczos"):
    resample = Image.Resampling.LANCZOS if method == "lanczos" else Image.Resampling.BICUBIC
    # Filter associated colors with alpha to prevent transparent RGB bleeding.
    result = image.convert("RGBa").resize(TARGET, resample).convert("RGBA") if image.mode == "RGBA" else image.resize(TARGET, resample)
    if method == "bicubic-sharp":
        if result.mode == "RGBA":
            alpha = result.getchannel("A")
            result = result.convert("RGB").filter(ImageFilter.UnsharpMask(radius=0.6, percent=35, threshold=3)).convert("RGBA")
            result.putalpha(alpha)
        else:
            result = result.filter(ImageFilter.UnsharpMask(radius=0.6, percent=35, threshold=3))
    return result


def previews():
    pilots = [
        ("story_backgrounds/1_1_picnic.png", (970, 590, 1390, 900)),
        ("story_backgrounds/3_3_night.png", (400, 470, 820, 780)),
        ("gameplay_backgrounds/1_forest_path.png", (750, 300, 1170, 610)),
        ("menus/title_forest_path.png", (750, 300, 1170, 610)),
        ("gameplay_backgrounds/2_fallen_tree_log.png", (750, 580, 1170, 890)),
    ]
    sheet = Image.new("RGB", (1260, len(pilots) * 350), "#333333")
    draw = ImageDraw.Draw(sheet)
    for row, (name, crop) in enumerate(pilots):
        with Image.open(WORK / "originals" / name) as source:
            for col, method in enumerate(["bicubic", "lanczos", "bicubic-sharp"]):
                result = resize(source, method)
                if result.mode == "RGBA":
                    backdrop = Image.new("RGBA", TARGET, "#808080")
                    result = Image.alpha_composite(backdrop, result).convert("RGB")
                sheet.paste(result.crop(crop), (col * 420, row * 350 + 40))
                draw.text((col * 420 + 5, row * 350 + 5), f"{Path(name).stem} / {method}", fill="white")
    sheet.save(WORK / "comparison.png")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--apply", action="store_true", help="Replace runtime images after staging and verification")
    args = parser.parse_args()
    paths = sorted((TEXTURES / "story_backgrounds").glob("*.png")) + sorted((TEXTURES / "gameplay_backgrounds").glob("*.png")) + [TEXTURES / "menus/title_forest_path.png"]
    if len(paths) != 41:
        raise ValueError(f"Expected 41 backgrounds, found {len(paths)}; review scope first")
    entries = []
    for path in paths:
        relative = path.relative_to(TEXTURES)
        backup = WORK / "originals" / relative
        output = WORK / "output" / relative
        backup.parent.mkdir(parents=True, exist_ok=True)
        output.parent.mkdir(parents=True, exist_ok=True)
        if not backup.exists():
            with Image.open(path) as image:
                if image.size != (1672, 941):
                    raise ValueError(f"Unexpected original size: {path}: {image.size}")
            shutil.copy2(path, backup)
        with Image.open(backup) as source:
            if source.mode not in ("RGB", "RGBA"):
                raise ValueError(f"Unsupported mode: {source.mode}")
            result = resize(source)
            metadata = {key: source.info[key] for key in ("icc_profile", "exif", "dpi") if key in source.info}
            result.save(output, optimize=True, **metadata)
            with Image.open(output) as check:
                check.load()
                assert check.size == TARGET and check.mode == source.mode
                assert check.tobytes() == result.tobytes()
                if source.mode == "RGBA":
                    assert check.getchannel("A").getextrema() == source.getchannel("A").getextrema()
            # Refuse to overwrite new artwork introduced since the backup was made.
            if digest(path) not in (digest(backup), digest(output)):
                raise ValueError(f"Runtime image changed since backup: {path}")
            entries.append({"path": relative.as_posix(), "source_size": list(source.size), "output_size": list(TARGET), "mode": source.mode, "source_sha256": digest(backup), "output_sha256": digest(output), "source_bytes": backup.stat().st_size, "output_bytes": output.stat().st_size})
    previews()
    manifest = {"method": "Lanczos; no sharpening; premultiplied-alpha filtering for RGBA", "pillow_version": pillow_version, "source": "resources/textures only", "files": entries}
    (WORK / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    if args.apply:
        for entry in entries:
            shutil.copy2(WORK / "output" / entry["path"], TEXTURES / entry["path"])
        assert all(digest(TEXTURES / item["path"]) == item["output_sha256"] for item in entries)
    print(f"{'Applied' if args.apply else 'Staged'} and verified {len(entries)} backgrounds. Report: {WORK / 'manifest.json'}")


if __name__ == "__main__":
    main()
