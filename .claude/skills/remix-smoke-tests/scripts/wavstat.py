"""Summarize a run's audio dump: per-second RMS and peak of the raw s16 stereo 32 kHz PCM.

Usage: wavstat.py <name>... (run with .toolchain/host/venv/bin/python -I)
Reads build/agent-shots/vs/<name>/audio.wav, skips the 44-byte header (it stays 0 when the run ends before the dump limit),
and prints one line per run: seconds captured, seconds with sound, peak, and the RMS of each second.
"""
import array
import math
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
RATE = 32000
CHANNELS = 2
SOUND_RMS = 20.0


def summarize(name: str) -> str:
    path = ROOT / "build/agent-shots/vs" / name / "audio.wav"
    if not path.exists():
        return f"{name}: no audio.wav"
    raw = path.read_bytes()[44:]
    samples = array.array("h")
    samples.frombytes(raw[: len(raw) // 2 * 2])
    window = RATE * CHANNELS
    rms = []
    for start in range(0, len(samples) - window + 1, window):
        chunk = samples[start:start + window]
        rms.append(math.sqrt(sum(v * v for v in chunk) / len(chunk)))
    peak = max((abs(v) for v in samples), default=0)
    sounding = sum(1 for v in rms if v >= SOUND_RMS)
    return f"{name}: {len(rms)} s captured, {sounding} s with sound, peak {peak}, rms/s " + " ".join(f"{v:.0f}" for v in rms)


for run in sys.argv[1:]:
    print(summarize(run))
