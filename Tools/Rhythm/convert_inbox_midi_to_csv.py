#!/usr/bin/env python3
"""
Batch-convert every MIDI file in Tools/Rhythm/Inbox/MIDI into Unreal DataTable CSVs.

This does not convert raw audio to MIDI. Put the extracted .mid/.midi files here,
then import the matching audio files into Content/Assets/Mathiasprovot/RhythmGame/Audio.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from convert_midi_to_csv import convert, parse_lane_map


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_INPUT_DIR = REPO_ROOT / "Tools" / "Rhythm" / "Inbox" / "MIDI"
DEFAULT_OUTPUT_DIR = REPO_ROOT / "Content" / "Assets" / "Mathiasprovot" / "RhythmGame" / "Charts"


def iter_midi_files(input_dir: Path) -> list[Path]:
    files: list[Path] = []
    for pattern in ("*.mid", "*.midi"):
        files.extend(input_dir.glob(pattern))
    return sorted(files, key=lambda path: path.name.lower())


def main() -> int:
    parser = argparse.ArgumentParser(description="Batch-convert rhythm MIDI inbox files to Unreal CSV charts.")
    parser.add_argument("--input-dir", type=Path, default=DEFAULT_INPUT_DIR)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--map", dest="lane_map", help="Explicit pitch-to-lane map, e.g. 36:0,38:1,42:2")
    args = parser.parse_args()

    input_dir = args.input_dir.resolve()
    output_dir = args.output_dir.resolve()
    lane_map = parse_lane_map(args.lane_map)

    if not input_dir.exists():
        raise FileNotFoundError(f"Missing MIDI inbox: {input_dir}")

    midi_files = iter_midi_files(input_dir)
    if not midi_files:
        print(f"No MIDI files found in {input_dir}")
        return 0

    output_dir.mkdir(parents=True, exist_ok=True)
    for midi_file in midi_files:
        output_csv = output_dir / f"{midi_file.stem}.csv"
        count = convert(midi_file, output_csv, lane_map)
        print(f"{midi_file.name} -> {output_csv} ({count} notes)")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
