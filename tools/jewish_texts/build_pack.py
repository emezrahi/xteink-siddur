#!/usr/bin/env python3
"""
Build the offline X3 Jewish text pack.

Sources are pinned by version title and license, not by Sefaria's "merged" text:
  - Tehillim + Torah: Tanach with Ta'amei Hamikra (Public Domain)
  - Mishnah: Torat Emet 357 (Public Domain)
  - Targum Onkelos: Onkelos {Book} (Public Domain)
Parashah boundaries come from hebcal/hebcal-leyning src/aliyot.json (BSD-2-Clause).

The output is intentionally plain UTF-8 text + TSV indexes so the ESP32 can read
it from microSD without a JSON parser or embedding several megabytes in flash.
"""
from __future__ import annotations

import argparse
import hashlib
import html
import json
import re
import shutil
import sys
import urllib.request
import zipfile
from pathlib import Path
from typing import Any

SEFARIA_BOOKS_INDEX = "https://raw.githubusercontent.com/Sefaria/Sefaria-Export/master/books.json"
HEBCAL_ALIYOT = "https://raw.githubusercontent.com/hebcal/hebcal-leyning/main/src/aliyot.json"

TANACH_VERSION = "Tanach with Ta'amei Hamikra"
MISHNAH_VERSION = "Torat Emet 357"
TORAH_BOOKS = ["Genesis", "Exodus", "Leviticus", "Numbers", "Deuteronomy"]
SEDER_ORDER = ["Zeraim", "Moed", "Nashim", "Nezikin", "Kodashim", "Tahorot"]
ALLOWED_LICENSES = {"Public Domain", "PD"}

TAG_RE = re.compile(r"<[^>]+>")
SPACE_RE = re.compile(r"[ \t\r\f\v]+")


def fetch_bytes(url: str) -> bytes:
    req = urllib.request.Request(url, headers={"User-Agent": "x3-jewish-text-pack/1.0"})
    with urllib.request.urlopen(req, timeout=90) as response:
        return response.read()


def fetch_json(url: str) -> Any:
    return json.loads(fetch_bytes(url).decode("utf-8"))


def clean_text(value: str) -> str:
    value = html.unescape(TAG_RE.sub("", value or ""))
    value = SPACE_RE.sub(" ", value)
    return value.strip()


def safe_slug(value: str) -> str:
    value = value.replace("Mishnah ", "")
    value = re.sub(r"[^A-Za-z0-9]+", "-", value).strip("-").lower()
    return value or "text"


def hebrew_number(n: int) -> str:
    if n <= 0 or n >= 100:
        return str(n)
    ones = ["", "א", "ב", "ג", "ד", "ה", "ו", "ז", "ח", "ט"]
    tens = ["", "י", "כ", "ל", "מ", "נ", "ס", "ע", "פ", "צ"]
    if n == 15:
        letters = "טו"
    elif n == 16:
        letters = "טז"
    else:
        letters = tens[n // 10] + ones[n % 10]
    return letters + "׳" if len(letters) == 1 else letters[:-1] + "״" + letters[-1]


def ensure_pd(data: dict[str, Any], expected_title: str, expected_version: str) -> None:
    got_title = data.get("title")
    got_version = data.get("versionTitle")
    license_name = data.get("license")
    if got_title != expected_title:
        raise RuntimeError(f"Wrong title: expected {expected_title!r}, got {got_title!r}")
    if got_version != expected_version:
        raise RuntimeError(f"Wrong version for {expected_title}: expected {expected_version!r}, got {got_version!r}")
    if license_name not in ALLOWED_LICENSES:
        raise RuntimeError(f"Refusing non-public-domain source {expected_title}: license={license_name!r}")


def select_entry(entries: list[dict[str, Any]], title: str, version: str, language: str = "Hebrew") -> dict[str, Any]:
    exact = [
        e for e in entries
        if e.get("title") == title and e.get("versionTitle") == version and e.get("language") == language
    ]
    if len(exact) != 1:
        candidates = [
            (e.get("title"), e.get("versionTitle"), e.get("language"))
            for e in entries if e.get("title") == title
        ]
        raise RuntimeError(f"Expected one exact Sefaria export for {title}/{version}/{language}; "
                           f"found {len(exact)}. Candidates: {candidates}")
    return exact[0]


def download_version(entry: dict[str, Any], source_log: list[dict[str, Any]]) -> dict[str, Any]:
    url = entry.get("json_url")
    if not url:
        raise RuntimeError(f"No json_url in books.json entry: {entry}")
    raw = fetch_bytes(url)
    data = json.loads(raw.decode("utf-8"))
    ensure_pd(data, entry["title"], entry["versionTitle"])
    source_log.append({
        "title": entry["title"],
        "versionTitle": entry["versionTitle"],
        "license": data.get("license"),
        "versionSource": data.get("versionSource"),
        "json_url": url,
        "sha256": hashlib.sha256(raw).hexdigest(),
    })
    return data


def write_text(path: Path, value: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(value.rstrip() + "\n", encoding="utf-8")


def build_tehillim(entries: list[dict[str, Any]], root: Path, source_log: list[dict[str, Any]]) -> None:
    data = download_version(select_entry(entries, "Psalms", TANACH_VERSION), source_log)
    chapters = data["text"]
    if len(chapters) != 150:
        raise RuntimeError(f"Expected 150 Tehillim chapters, got {len(chapters)}")
    index = []
    for chapter_no, verses in enumerate(chapters, 1):
        text = "\n\n".join(clean_text(v) for v in verses if clean_text(v))
        name = f"{chapter_no:03d}.txt"
        write_text(root / "tehillim" / name, text)
        index.append(f"{chapter_no}\t{name}\t{len(verses)}")
    write_text(root / "tehillim" / "index.tsv", "\n".join(index))


def mishnah_seder(entry: dict[str, Any]) -> str:
    for category in entry.get("categories", []):
        if category.startswith("Seder "):
            return category.removeprefix("Seder ")
    raise RuntimeError(f"Missing Mishnah seder for {entry.get('title')}: {entry.get('categories')}")


def build_mishnah(entries: list[dict[str, Any]], root: Path, source_log: list[dict[str, Any]]) -> None:
    selected = [
        e for e in entries
        if e.get("language") == "Hebrew"
        and e.get("versionTitle") == MISHNAH_VERSION
        and "Mishnah" in e.get("categories", [])
        and (str(e.get("title", "")).startswith("Mishnah ") or e.get("title") == "Pirkei Avot")
    ]
    if len(selected) != 63:
        raise RuntimeError(f"Expected 63 Mishnah tractates in {MISHNAH_VERSION}, got {len(selected)}")

    selected.sort(key=lambda e: (
        SEDER_ORDER.index(mishnah_seder(e)) if mishnah_seder(e) in SEDER_ORDER else 99,
        e["title"],
    ))

    seder_indexes: dict[str, list[str]] = {s: [] for s in SEDER_ORDER}
    master = []
    for entry in selected:
        seder = mishnah_seder(entry)
        data = download_version(entry, source_log)
        chapters = data["text"]
        tractate = "Avot" if entry["title"] == "Pirkei Avot" else entry["title"].removeprefix("Mishnah ")
        slug = safe_slug(tractate)
        seder_no = SEDER_ORDER.index(seder) + 1
        folder = root / "mishnah" / f"{seder_no:02d}-{seder.lower()}" / slug
        he_title = clean_text(data.get("heTitle", ""))

        for perek_no, mishnayot in enumerate(chapters, 1):
            parts = []
            for mishnah_no, mishnah in enumerate(mishnayot, 1):
                body = clean_text(mishnah)
                if not body:
                    continue
                parts.append(f"משנה {hebrew_number(mishnah_no)}\n\n{body}")
            write_text(folder / f"{perek_no:02d}.txt", "\n\n\n".join(parts))

        row = f"{slug}\t{tractate}\t{he_title}\t{len(chapters)}"
        seder_indexes[seder].append(row)
        master.append(f"{seder}\t{row}")

    for i, seder in enumerate(SEDER_ORDER, 1):
        write_text(root / "mishnah" / f"{i:02d}-{seder.lower()}" / "index.tsv",
                   "\n".join(seder_indexes[seder]))
    write_text(root / "mishnah" / "index.tsv", "\n".join(master))


def verse_range(book_text: list[list[str]], start: str, end: str) -> list[tuple[int, int, str]]:
    sc, sv = (int(x) for x in start.split(":"))
    ec, ev = (int(x) for x in end.split(":"))
    result = []
    for chapter in range(sc, ec + 1):
        verses = book_text[chapter - 1]
        first = sv if chapter == sc else 1
        last = ev if chapter == ec else len(verses)
        for verse in range(first, last + 1):
            result.append((chapter, verse, clean_text(verses[verse - 1])))
    return result


def build_shnayim(entries: list[dict[str, Any]], root: Path, source_log: list[dict[str, Any]]) -> None:
    torah: dict[int, dict[str, Any]] = {}
    onkelos: dict[int, dict[str, Any]] = {}
    for book_num, book in enumerate(TORAH_BOOKS, 1):
        torah[book_num] = download_version(select_entry(entries, book, TANACH_VERSION), source_log)
        onkelos_title = f"Onkelos {book}"
        onkelos[book_num] = download_version(select_entry(entries, onkelos_title, onkelos_title), source_log)

        t = torah[book_num]["text"]
        o = onkelos[book_num]["text"]
        if len(t) != len(o):
            raise RuntimeError(f"Chapter count mismatch {book}: Torah={len(t)}, Onkelos={len(o)}")
        for c, (tv, ov) in enumerate(zip(t, o), 1):
            if len(tv) != len(ov):
                raise RuntimeError(f"Verse count mismatch {book} {c}: Torah={len(tv)}, Onkelos={len(ov)}")

    aliyot_raw = fetch_bytes(HEBCAL_ALIYOT)
    aliyot = json.loads(aliyot_raw.decode("utf-8"))
    source_log.append({
        "title": "Hebcal aliyot.json",
        "versionTitle": "hebcal-leyning main/src/aliyot.json",
        "license": "BSD-2-Clause",
        "versionSource": "https://github.com/hebcal/hebcal-leyning",
        "json_url": HEBCAL_ALIYOT,
        "sha256": hashlib.sha256(aliyot_raw).hexdigest(),
    })

    parashot = [
        (name, item) for name, item in aliyot.items()
        if isinstance(item, dict) and isinstance(item.get("num"), int) and 1 <= item["num"] <= 54
    ]
    parashot.sort(key=lambda x: x[1]["num"])
    if len(parashot) != 54 or [x[1]["num"] for x in parashot] != list(range(1, 55)):
        raise RuntimeError(f"Expected parashot 1..54 from Hebcal, got {[x[1].get('num') for x in parashot]}")

    index = []
    for name, item in parashot:
        book_num = int(item["book"])
        first = item["fullkriyah"]["1"][0]
        last = item["fullkriyah"]["7"][1]
        mikra = verse_range(torah[book_num]["text"], first, last)
        targum = verse_range(onkelos[book_num]["text"], first, last)
        if len(mikra) != len(targum):
            raise RuntimeError(f"Shnayim verse mismatch for {name}: {len(mikra)} vs {len(targum)}")

        chunks = []
        for (c1, v1, hebrew), (c2, v2, aramaic) in zip(mikra, targum):
            if (c1, v1) != (c2, v2) or not hebrew or not aramaic:
                raise RuntimeError(f"Bad aligned verse in {name}: {(c1, v1)} / {(c2, v2)}")
            # Shnayim mikra ve'echad targum: each Torah verse twice, then Onkelos once.
            chunks.append(f"{hebrew}\n\n{hebrew}\n\n{aramaic}")

        slug = f"{item['num']:02d}-{safe_slug(name)}"
        filename = slug + ".txt"
        write_text(root / "shnayim" / filename, "\n\n\n".join(chunks))
        index.append(f"{item['num']}\t{name}\t{filename}\t{TORAH_BOOKS[book_num - 1]}\t{first}\t{last}\t{len(mikra)}")

    write_text(root / "shnayim" / "index.tsv", "\n".join(index))


def make_zip(content_root: Path, zip_path: Path) -> None:
    zip_path.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as zf:
        for path in sorted(content_root.rglob("*")):
            if path.is_file():
                zf.write(path, Path("jewish-texts") / path.relative_to(content_root))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output-dir", default="build/jewish-texts")
    parser.add_argument("--zip", default="build/x3-jewish-texts.zip")
    args = parser.parse_args()

    out = Path(args.output_dir)
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)

    books = fetch_json(SEFARIA_BOOKS_INDEX)
    entries = books["books"] if isinstance(books, dict) and "books" in books else books
    if not isinstance(entries, list):
        raise RuntimeError("Unexpected books.json shape")

    source_log: list[dict[str, Any]] = []
    build_tehillim(entries, out, source_log)
    build_mishnah(entries, out, source_log)
    build_shnayim(entries, out, source_log)

    source_manifest = {
        "format": 1,
        "generatedBy": "tools/jewish_texts/build_pack.py",
        "contentRootOnSd": "/jewish-texts",
        "libraries": {
            "tehillim": {"chapters": 150, "sourceVersion": TANACH_VERSION},
            "mishnah": {"tractates": 63, "sourceVersion": MISHNAH_VERSION},
            "shnayimMikrah": {
                "parashot": 54,
                "torahVersion": TANACH_VERSION,
                "targumVersionPattern": "Onkelos {Book}",
                "format": "Torah verse twice, followed by Onkelos once",
            },
        },
        "sources": source_log,
    }
    (out / "SOURCES.json").write_text(json.dumps(source_manifest, ensure_ascii=False, indent=2) + "\n",
                                      encoding="utf-8")
    write_text(out / "README.txt",
               "X3 Jewish Text Library\n\n"
               "Copy this entire jewish-texts folder to the root of the X3 microSD card.\n\n"
               "Contains:\n"
               "- Tehillim 1-150\n"
               "- All 63 Mishnah tractates, organized by Seder / Masechet / Perek\n"
               "- All 54 weekly parashot as Shnayim Mikra Ve'echad Targum\n\n"
               "See SOURCES.json for exact source versions, licenses, URLs and SHA-256 hashes.\n")
    make_zip(out, Path(args.zip))
    print(json.dumps({
        "output": str(out),
        "zip": args.zip,
        "files": sum(1 for p in out.rglob("*") if p.is_file()),
        "sources": len(source_log),
    }, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    sys.exit(main())
