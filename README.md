# Procedural Audio Engine
A real-time procedural audio synthesis engine written in C++ using WASAPI. The project focuses on waveform generation, modular audio processing, and real-time manipulation through an interactive UI.

---

## 🎛️ Demo Application
The project includes a fully interactive **Demo Application** built with GLFW and ImGui.
This application provides a visual interface for testing and exploring the audio engine in real time. It acts as both a debugging tool and a live sandbox for audio experimentation.

### Features
- Create and manage multiple `Sound` instances dynamically
- Real-time control of:
  - Volume
  - Pitch multiplier
  - Base frequency
  - Decibel offset
- Switch between waveform generators:
  - Sine
  - Square
  - Sawtooth
  - White Noise
- Attach and configure:
  - Audio effects (e.g. fade effect)
  - Modulators (frequency, pitch, amplitude, dB)
- Playback controls (start, stop, pause, resume)
- Immediate visual feedback for all parameter changes

The demo application provides a practical way to test the engine’s full audio pipeline without needing to write additional code.

---

## 🧠 Engine Overview

The audio engine is built around a modular signal processing pipeline:
Each `Sound` instance is fully independent and processes audio in real time.

Key systems include:
- WASAPI-based low-level audio backend abstraction
- Procedural waveform generators
- Real-time modulation system
- Chainable audio effects system
- Per-sample audio processing
- Multi-sound mixing and management

---

## 🎬 Demonstration
A full walkthrough and demonstration of the engine and demo application:
**YouTube:** https://youtu.be/S8KHcnoKfls

---

## 🚀 Purpose
This project was developed to explore:
- Low-level Windows audio programming (WASAPI)
- Real-time audio synthesis and DSP concepts
- Modular engine architecture design
- Interactive audio tool development with ImGui

---
