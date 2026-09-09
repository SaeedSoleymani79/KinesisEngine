# KINESIS Engine

> A custom, high-end MIDI performance utility plugin designed to bridge the gap between standard computer keyboard input and expressive live performance.

## Overview
Built from the ground up using **C++** and the **JUCE Framework**, Kinesis Engine reimagines the traditional QWERTY keyboard as a dynamic, velocity-sensitive musical instrument. Bypassing standard UI templates in favor of a bespoke Neumorphic design, Kinesis delivers a tactile, premium studio-hardware experience directly to your digital workspace. 

## Key Features

### 🎛 Physics-Driven Interface
* **Momentum Faders:** A custom `PhysicsSlider` component utilizing a 60Hz delta-time loop for momentum-driven fading and user-adjustable friction.
* **Trackpad & Scroll Optimization:** High-resolution input support for fluid, expressive parameter sweeps.
* **Neumorphic Look & Feel:** A highly detailed rendering engine supporting live switching between a sleek Light Mode and a deep, premium Dark Mode aesthetic.

### 🎹 QWERTY Performance Mapping
* **Expressive Melody Playing:** Standard Z-M / Q-U keybed mapping engineered for dynamic, velocity-sensitive input.
* **Smart Chord Pads:** Number keys (1–6) act as velocity-sensitive performance pads that trigger complex, smart-quantized chord mappings in real-time.
* **Responsive Visuals:** A custom on-screen piano roll and UI elements that light up instantly with smart color feedback upon keystrokes.

### 📈 Real-Time Data Visualization
* **Dynamic Automation Graphs:** LED-style visualizers that plot dynamic curves in real-time based on fader movement and incoming MIDI velocity data.

## Architecture & Engineering
Kinesis Engine is built on a highly disciplined, fully modular **MVC (Model-View-Controller)** architecture to ensure absolute stability during live performances.

* **Core Logic & DSP (`Core/`):** Contains the `MidiEngine` and handles thread-safe parameter synchronization using APVTS (AudioProcessorValueTreeState). It utilizes fast, lock-free queues (`atomic_queue`, `readerwriterqueue`) to seamlessly hand off data between the audio and GUI threads without risking audio dropouts.
* **User Interface (`UI/`):** Houses the bespoke rendering classes (e.g., `CinematicFaderPad`, `MiniAutomationGraph`, `CinematicMidiKeyboard`, `ScaleAndChordPanel`), optimized for high-framerate rendering.

## Getting Started (Build Instructions)
*Ensure you have the latest JUCE framework and a compatible C++ compiler (MSVC, Xcode, or GCC) installed.*

1. Clone the repository.
2. Open the `.jucer` file via the JUCE Projucer (or generate via CMake).
3. Export the project to your preferred IDE (Visual Studio / Xcode).
4. Build the project as a Standalone Application, VST3, or AU plugin.

---
*Designed for seamless interaction and pristine audio performance.*
