# Heritage Channel

Heritage Channel is a 64-bit VST3 audio effect for Intel macOS and Windows. It combines a circuit-informed British discrete input/EQ stage, a variable-mu stereo limiter and a multitrack tape record/replay stage in one signal flow, with an original B.E.E hardware-style interface.

The Mac build targets **Intel (`x86_64`) macOS 11 or later** and produces a VST3 suitable for testing in Cubase LE/AI/Elements 9.5 on an Intel Mac. Cubase 9.5 itself is not officially supported on current macOS versions, so the final compatibility check must be made in Cubase. The Windows build retains an x64 SSE2 baseline and no AVX requirement.

## Signal flow

`Input transformer / drive → 1073-style EQ → 670-style variable-mu limiter → A800-style tape → output`

The current v0.1 model follows published circuit topology, component data, time constants and tape specifications. It has not yet been calibrated against captures from specific physical units. Unit-to-unit variation in transformers, valves, tape heads, alignment and ageing cannot be derived fully from schematics.

## Controls

### Console and EQ

- **Drive:** pushes the asymmetric input amplifier and transformer-memory model.
- **Trim:** compensates the driven input level.
- **HPF:** Off, 50, 80, 160 or 300 Hz at 18 dB/octave.
- **Low:** ±16 dB at 35, 60, 110 or 220 Hz.
- **Mid:** ±18 dB at 360, 700, 1.6k, 3.2k, 4.8k or 7.2 kHz, with Broad/Tight Q.
- **High:** ±16 dB shelf at 12 kHz.
- **Character:** Classic adds the fuller transformer colour; Clean reduces the nonlinear colour.

### Valve limiter

- **Threshold** and **Amount** set the compression depth and progressive ratio.
- **Time Constant 1–4** reproduces the published fixed attack/release choices.
- **Time Constant 5–6** uses dual release envelopes for program-dependent recovery.
- **Make-up** restores level after gain reduction.

### Tape

- **3M 250:** earliest and softest of the included high-energy formulations.
- **Ampex 456:** classic +6 studio formulation.
- **BASF SM900:** high-output +9 formulation with greater headroom and lower noise.
- **Speed:** 7.5, 15 or 30 ips. Speed changes head bump, replay bandwidth and record emphasis.
- **Saturation:** drives the stateful magnetic model.
- **Bias:** moves the saturation/headroom balance around the nominal alignment.
- **Noise:** adds stock-dependent tape noise; off by default.

The master section provides whole-chain wet/dry mix, output level and channel bypass.

All nonlinear stages run at 2× oversampling. The plug-in reports its oversampling latency to the host.

## Building and installing on Intel Mac

Upload the project contents to the top level of a GitHub repository, including the hidden `.github` folder. In **Actions**, run **Build Mac Intel VST3**. When it has a green tick, download the `HeritageChannel-Mac-Intel` artifact, extract both zip layers, and follow [INSTALL_MAC.md](INSTALL_MAC.md).

No local Xcode installation is required for the GitHub build.

## Installing the Windows build

Run `HeritageChannel-Win64-Setup.exe` as an administrator. The installer places the bundle in:

`C:\Program Files\Common Files\VST3\Heritage Channel.vst3`

Restart Cubase after installation. Cubase 5 should scan the standard VST3 directory during startup. If it does not appear, open **Devices → Plug-in Information** and update the plug-in list.

## Building on Windows

Requirements:

- Windows 10 or 11 build computer
- Visual Studio 2019 with Desktop development with C++
- CMake 3.15 or newer
- Git
- NSIS 3

From PowerShell:

```powershell
.\scripts\build-windows.ps1
```

The script obtains JUCE, creates an x64 Visual Studio build, runs the DSP tests and generates the NSIS installer. To build with Visual Studio 2022 plus its optional v142 toolset:

```powershell
.\scripts\build-windows.ps1 -VisualStudio 2022
```

The GitHub Actions workflow performs the same build manually or when a version tag is pushed.

For a Mac-to-offline-Windows workflow, follow [BUILD_ON_MAC.md](BUILD_ON_MAC.md). It uses the included GitHub Actions build and requires no development tools or internet connection on the Windows 7 computer.

## Reference documents

- Neve 1073 full schematic pack: https://retrorecordingsxr.com/wp-content/uploads/manuals/neve/1073-fullpak.pdf
- Fairchild 670 manual: https://funkwerkes.com/web/wp-content/techdocs/MixedProAudio/Fairchild-670-Limiter-Manual.pdf
- Fairchild 670 schematic: https://thehistoryofrecording.com/Schematics/Fairchild/Fairchild_670_stereo_limiting_amplifier_schem.pdf
- Studer A800 service notes: https://archive.org/details/Studer_A800_service_notes
- Ampex 456 data: https://www.videobex.it/specifiche/Studio_Mastering_Tape_456.pdf
- RTM/BASF SM900 data: https://static1.squarespace.com/static/62798251da32f57e31bea13d/t/62f2149f22e6b6134c31427e/1660032161030/fiche_sm900_rtm_2022.pdf

Product names in the references identify the historical equipment and media studied. Heritage Channel is an independent project and is not affiliated with or endorsed by their owners.
