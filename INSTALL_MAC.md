# Install Heritage Channel on an Intel Mac

This build is a 64-bit Intel (`x86_64`) VST3 for macOS 11 or later. It is the correct architecture for an Intel Mac running macOS Tahoe.

## Install

1. Extract `HeritageChannel-Mac-Intel.zip` completely.
2. Control-click **Install Heritage Channel.command** and choose **Open**.
3. If macOS asks whether to open it, choose **Open** again.
4. Restart Cubase.
5. In Cubase open **Studio > VST Plug-in Manager**, rescan plug-ins, then look under audio effects for **Heritage Channel**.

The installer copies the complete bundle to:

`~/Library/Audio/Plug-Ins/VST3/Heritage Channel.vst3`

It also clears the download quarantine flag and verifies the ad-hoc signature. It does not need administrator access.

## Manual install

Copy the complete `Heritage Channel.vst3` bundle to:

`~/Library/Audio/Plug-Ins/VST3/`

Then open Terminal and run:

```bash
xattr -cr "$HOME/Library/Audio/Plug-Ins/VST3/Heritage Channel.vst3"
codesign --verify --deep --strict "$HOME/Library/Audio/Plug-Ins/VST3/Heritage Channel.vst3"
```

## Cubase 9.5 note

Cubase 9.5 is much older than macOS Tahoe and Steinberg does not list this combination as supported. The plug-in is deliberately built as an Intel VST3, but successful loading still depends on Cubase itself running correctly on Tahoe. If it is blacklisted, remove it from the blocklist in VST Plug-in Manager and rescan. If the interface is blank or Cubase crashes while scanning, keep the plug-in removed and report the exact Cubase scan message.
