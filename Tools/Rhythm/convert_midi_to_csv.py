#!/usr/bin/env python3
"""
Convert a MIDI file into an Unreal DataTable CSV for FRhythmNoteRow.

Output columns:
Name,TimeSec,Lane,DurationSec,Velocity,Pitch

Examples:
  python Tools/Rhythm/convert_midi_to_csv.py song.mid Content/Assets/Mathiasprovot/song.csv
  python Tools/Rhythm/convert_midi_to_csv.py song.mid out.csv --map 36:0,38:1,42:2
"""

from __future__ import annotations

import argparse
import csv
import struct
from dataclasses import dataclass
from pathlib import Path


TPQN_DEFAULT = 480


@dataclass
class MidiEvent:
    tick: int
    event_type: str
    channel: int
    pitch: int
    velocity: int


@dataclass
class Note:
    start_tick: int
    end_tick: int
    pitch: int
    velocity: int


@dataclass
class Tempo:
    tick: int
    micros_per_quarter: int


def read_var_len(data: bytes, offset: int) -> tuple[int, int]:
    value = 0
    while True:
        byte = data[offset]
        offset += 1
        value = (value << 7) | (byte & 0x7F)
        if not byte & 0x80:
            return value, offset


def parse_midi(path: Path) -> tuple[int, list[MidiEvent], list[Tempo]]:
    data = path.read_bytes()
    if data[:4] != b"MThd":
        raise ValueError("Not a MIDI file: missing MThd header")

    header_len = struct.unpack(">I", data[4:8])[0]
    fmt, track_count, division = struct.unpack(">HHH", data[8:14])
    if division & 0x8000:
        raise ValueError("SMPTE time division is not supported")

    ticks_per_quarter = division or TPQN_DEFAULT
    offset = 8 + header_len
    events: list[MidiEvent] = []
    tempos: list[Tempo] = [Tempo(0, 500000)]

    for _ in range(track_count):
        if data[offset : offset + 4] != b"MTrk":
            raise ValueError("Invalid MIDI track chunk")
        track_len = struct.unpack(">I", data[offset + 4 : offset + 8])[0]
        track = data[offset + 8 : offset + 8 + track_len]
        offset += 8 + track_len

        tick = 0
        pos = 0
        running_status: int | None = None

        while pos < len(track):
            delta, pos = read_var_len(track, pos)
            tick += delta

            status = track[pos]
            if status & 0x80:
                pos += 1
                running_status = status
            elif running_status is not None:
                status = running_status
            else:
                raise ValueError("Running status used before a status byte")

            if status == 0xFF:
                meta_type = track[pos]
                pos += 1
                length, pos = read_var_len(track, pos)
                payload = track[pos : pos + length]
                pos += length
                if meta_type == 0x51 and length == 3:
                    tempos.append(Tempo(tick, int.from_bytes(payload, "big")))
                continue

            if status in (0xF0, 0xF7):
                length, pos = read_var_len(track, pos)
                pos += length
                continue

            event_kind = status & 0xF0
            channel = status & 0x0F
            data_len = 1 if event_kind in (0xC0, 0xD0) else 2
            payload = track[pos : pos + data_len]
            pos += data_len

            if event_kind == 0x90:
                pitch, velocity = payload[0], payload[1]
                events.append(MidiEvent(tick, "note_on" if velocity > 0 else "note_off", channel, pitch, velocity))
            elif event_kind == 0x80:
                pitch, velocity = payload[0], payload[1]
                events.append(MidiEvent(tick, "note_off", channel, pitch, velocity))

    return ticks_per_quarter, events, sorted(tempos, key=lambda t: t.tick)


def collect_notes(events: list[MidiEvent]) -> list[Note]:
    active: dict[tuple[int, int], list[MidiEvent]] = {}
    notes: list[Note] = []

    for event in sorted(events, key=lambda e: (e.tick, e.event_type == "note_on")):
        key = (event.channel, event.pitch)
        if event.event_type == "note_on":
            active.setdefault(key, []).append(event)
        elif event.event_type == "note_off" and active.get(key):
            start = active[key].pop(0)
            if event.tick >= start.tick:
                notes.append(Note(start.tick, event.tick, start.pitch, start.velocity))

    return sorted(notes, key=lambda n: (n.start_tick, n.pitch))


def tick_to_seconds(tick: int, ticks_per_quarter: int, tempos: list[Tempo]) -> float:
    seconds = 0.0
    last_tick = 0
    last_tempo = 500000

    for tempo in tempos:
        if tempo.tick > tick:
            break
        seconds += (tempo.tick - last_tick) * last_tempo / ticks_per_quarter / 1_000_000.0
        last_tick = tempo.tick
        last_tempo = tempo.micros_per_quarter

    seconds += (tick - last_tick) * last_tempo / ticks_per_quarter / 1_000_000.0
    return seconds


def parse_lane_map(value: str | None) -> dict[int, int]:
    if not value:
        return {}
    result: dict[int, int] = {}
    for item in value.split(","):
        pitch, lane = item.split(":", 1)
        result[int(pitch.strip())] = max(0, min(2, int(lane.strip())))
    return result


def lane_for_pitch(pitch: int, ranked_pitches: list[int], explicit_map: dict[int, int]) -> int:
    if pitch in explicit_map:
        return explicit_map[pitch]
    if not ranked_pitches:
        return 1
    rank = ranked_pitches.index(pitch)
    if len(ranked_pitches) == 1:
        return 1
    normalized = rank / (len(ranked_pitches) - 1)
    return max(0, min(2, round(normalized * 2)))


def convert(input_path: Path, output_path: Path, explicit_map: dict[int, int]) -> int:
    ticks_per_quarter, events, tempos = parse_midi(input_path)
    notes = collect_notes(events)
    ranked_pitches = sorted({note.pitch for note in notes})

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", newline="", encoding="utf-8") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["Name", "TimeSec", "Lane", "DurationSec", "Velocity", "Pitch"])
        for index, note in enumerate(notes):
            start_sec = tick_to_seconds(note.start_tick, ticks_per_quarter, tempos)
            end_sec = tick_to_seconds(note.end_tick, ticks_per_quarter, tempos)
            writer.writerow(
                [
                    f"Note_{index:05d}",
                    f"{start_sec:.6f}",
                    lane_for_pitch(note.pitch, ranked_pitches, explicit_map),
                    f"{max(0.0, end_sec - start_sec):.6f}",
                    note.velocity,
                    note.pitch,
                ]
            )

    return len(notes)


def main() -> int:
    parser = argparse.ArgumentParser(description="Convert MIDI notes into Unreal FRhythmNoteRow CSV.")
    parser.add_argument("input_midi", type=Path)
    parser.add_argument("output_csv", type=Path)
    parser.add_argument("--map", dest="lane_map", help="Explicit pitch-to-lane map, e.g. 36:0,38:1,42:2")
    args = parser.parse_args()

    count = convert(args.input_midi, args.output_csv, parse_lane_map(args.lane_map))
    print(f"Wrote {count} notes to {args.output_csv}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
