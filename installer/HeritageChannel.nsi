Unicode true
Name "Heritage Channel"
OutFile "HeritageChannel-Win64-Setup.exe"
InstallDir "$COMMONFILES64\VST3"
RequestExecutionLevel admin
SetCompressor /SOLID lzma

!define PRODUCT_VERSION "0.1.0"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\HeritageChannel"

VIProductVersion "0.1.0.0"
VIAddVersionKey "ProductName" "Heritage Channel"
VIAddVersionKey "FileDescription" "Heritage Channel 64-bit VST3 installer"
VIAddVersionKey "FileVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "CompanyName" "BrenH Audio"

Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

Section "Heritage Channel VST3" SEC_MAIN
    SetRegView 64
    SetOutPath "$INSTDIR"
    File /r "..\build\HeritageChannel_artefacts\Release\VST3\Heritage Channel.vst3"
    WriteUninstaller "$INSTDIR\Heritage Channel Uninstall.exe"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "DisplayName" "Heritage Channel"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "Publisher" "BrenH Audio"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\Heritage Channel Uninstall.exe"'
    WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoModify" 1
    WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoRepair" 1
SectionEnd

Section "Uninstall"
    SetRegView 64
    Delete "$COMMONFILES64\VST3\Heritage Channel.vst3"
    RMDir /r "$COMMONFILES64\VST3\Heritage Channel.vst3"
    Delete "$COMMONFILES64\VST3\Heritage Channel Uninstall.exe"
    DeleteRegKey HKLM "${UNINSTALL_KEY}"
SectionEnd
