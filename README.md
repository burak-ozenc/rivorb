# Rivörb

A VST3 reverb plugin with spectral-adaptive processing. Instead of manually EQ-ing your reverb to fit the source, Rivörb analyzes the incoming signal and shapes the reverb tail automatically.

Built with JUCE / C++.

---

## Signal Chain

```
Input
  ↓
Pre-EQ          — filter what enters the reverb (HP + 3 parametric bells)
  ↓
Harmonic Sat    — even/odd harmonic waveshaper for warmth
  ↓
Pre-delay       — 0–100ms
  ↓
Reverb Engine   — wet signal only
  ↓
Modulation      — chorus on the reverb tail
  ↓
Post-EQ         — auto spectral analysis drives EQ cuts on the tail (optional)
  ↓
Compressor      — stabilize reverb dynamics, sidechain-ready
  ↓
Wet/Dry Mix
  ↓
Output
```

---

## Features

- **Spectral-adaptive Post-EQ** — FFT analysis of the source every 2 seconds. Automatically cuts problem frequencies (low mud, boxiness, harshness, sibilance) from the reverb tail so it sits with the dry signal rather than fighting it. Toggle on/off.
- **Pre-EQ** — shape the signal before it enters the reverb. Controls what the reverb "hears" — a cleaner feed produces a cleaner tail.
- **Harmonic Saturator** — blend between odd harmonics (tanh, gritty) and even harmonics (quadratic, warm/tape-like). At zero drive it is fully transparent.
- **Modulation** — subtle chorus on the reverb tail. Prevents the tail from sounding static.
- **Compressor** — envelope follower with independent attack and release. Use a slow release to get a natural reverb swell in the gaps between phrases.

---

## Parameters

| Section | Parameter | Range | Default |
|---|---|---|---|
| Pre-EQ | HP Frequency | 20–500 Hz | 80 Hz |
| Pre-EQ | Bell 1 Freq / Gain | 100–2000 Hz / ±12 dB | 500 Hz / 0 dB |
| Pre-EQ | Bell 2 Freq / Gain | 500–5000 Hz / ±12 dB | 2000 Hz / 0 dB |
| Pre-EQ | Bell 3 Freq / Gain | 2–16 kHz / ±12 dB | 8000 Hz / 0 dB |
| Saturator | Drive | 0–1 | 0 |
| Saturator | Even/Odd Blend | 0–1 | 1 (even) |
| Reverb | Room Size | 0.1–1.0 | 0.55 |
| Reverb | Damping | 0–1 | 0.45 |
| Reverb | Pre-delay | 0–100 ms | 20 ms |
| Reverb | Wet Level | 0–1 | 0.35 |
| Modulation | Rate | 0.01–5 Hz | 0.5 Hz |
| Modulation | Depth | 0–1 | 0 |
| Post-EQ | Enabled | on/off | on |
| Compressor | Threshold | -60–0 dB | -24 dB |
| Compressor | Ratio | 1:1–20:1 | 4:1 |
| Compressor | Attack | 1–200 ms | 10 ms |
| Compressor | Release | 10–2000 ms | 200 ms |

---

## Building

**Requirements**
- JUCE 8+
- CMake 3.22+
- MSVC (Visual Studio 2022 build tools) or Clang
- CLion or Visual Studio

**Steps**

```bash
git clone https://github.com/yourname/Rivörb
cd Rivörb
```

Open `CMakeLists.txt` in CLion. Update the JUCE path if needed:

```cmake
add_subdirectory(C:/JUCE JUCE)
```

Build → the `.vst3` file will be at:
```
cmake-build-debug/Rivörb_artefacts/Debug/VST3/Rivörb.vst3
```

Point your DAW's plugin scanner at that folder.

---

## Background

The spectral analysis algorithm was prototyped in Python first (librosa, scipy, pedalboard) before being ported to C++. The Python prototype is available as a Gradio app on Hugging Face Spaces for offline validation.

[Here](https://huggingface.co/spaces/burak-ozenc/Rivörb-test) is the link for this app on HF Spaces.

---

## Format

VST3 — Windows (x64). Mac / AU support planned.

---

## License

MIT