# Build on a Mac, install on an offline Windows 7 computer

The files in this project are source code. A Windows build must happen before the plug-in can be installed. The easiest route from a Mac is GitHub Actions, which supplies a temporary Windows build computer online. The resulting installer can then be copied to a USB stick.

## On the Mac

1. Create a free account at https://github.com if you do not already have one.
2. Sign in and choose **New repository**.
3. Name it `HeritageChannel`, select **Private**, leave the initialisation boxes unticked, and create it.
4. On the empty repository page choose **uploading an existing file**.
5. Extract `HeritageChannel-v0.1-source.zip`, then open the enclosed `HeritageChannel` folder.
6. In Finder press **Command + Shift + .** so the hidden `.github` folder is visible.
7. Drag every item inside `HeritageChannel` onto the GitHub upload page. The items must sit at the top level of the repository; do not upload the enclosing folder itself.
8. Enter `Initial build` in the commit box and select **Commit changes**.
9. Open the repository's **Actions** tab. If GitHub asks, enable workflows.
10. Select **Windows 7 x64 VST3** on the left, choose **Run workflow**, then confirm **Run workflow**.
11. Wait for the build to finish with a green tick. Open that completed run.
12. Under **Artifacts**, download `HeritageChannel-Win64`.
13. Extract the downloaded artifact. Copy `HeritageChannel-Win64-Setup.exe` to the USB stick.

The online build normally takes several minutes. It downloads JUCE and NSIS inside GitHub's temporary Windows computer; none of those tools need to be installed on the Mac or Windows 7 computer.

## On the offline Windows 7 computer

1. Close Cubase.
2. Insert the USB stick.
3. Right-click `HeritageChannel-Win64-Setup.exe` and choose **Run as administrator**.
4. Windows may report that the publisher is unknown because this private build is not code-signed. Confirm **Run** if the file came directly from your GitHub build.
5. Complete the installer. It places the 64-bit plug-in in `C:\Program Files\Common Files\VST3`.
6. Start the 64-bit edition of Cubase 5. Cubase should scan the VST3 folder during startup.
7. If the plug-in is missing, open **Devices → Plug-in Information**, update/rescan the plug-in list, then restart Cubase.
8. Insert it on a stereo audio channel and confirm that **Heritage Channel** opens and passes audio.

No internet connection is needed for these Windows steps.

## Manual installation fallback

The downloaded GitHub artifact also contains the compiled `Heritage Channel.vst3`. If the installer will not run, copy that complete file or folder from the USB stick to:

`C:\Program Files\Common Files\VST3\`

Keep the complete `.vst3` bundle intact. Administrator permission is required to write to this folder. Restart Cubase afterwards.
