; NSIS Installer Script for simple-dhcpd
; Modern UI with license acceptance

!include "MUI2.nsh"
!include "FileFunc.nsh"

;--------------------------------
; General

Name "simple-dhcpd"
OutFile "simple-dhcpd-Setup-${VERSION}.exe"
Unicode True
InstallDir "$PROGRAMFILES\SimpleDaemons\simple-dhcpd"
RequestExecutionLevel admin

; Version Information
VIProductVersion "${VERSION}.0"
VIAddVersionKey "ProductName" "simple-dhcpd"
VIAddVersionKey "FileDescription" "simple-dhcpd Installer"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "CompanyName" "SimpleDaemons"
VIAddVersionKey "LegalCopyright" "Copyright (C) 2024 SimpleDaemons"

;--------------------------------
; Interface Settings

!define MUI_ICON "..\assets\icons\simple-dhcpd.ico"
!define MUI_UNICON "..\assets\icons\simple-dhcpd.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "..\assets\icons\header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "..\assets\icons\wizard.bmp"

;--------------------------------
; Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\licenses\LICENSE.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------
; Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Installer Sections

Section "Core Files" SecCore
    SectionIn RO
    
    SetOutPath "$INSTDIR"
    File /r "simple-dhcpd\*"
    
    ; Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    
    ; Registry entries
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\simple-dhcpd" \
        "DisplayName" "simple-dhcpd"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\simple-dhcpd" \
        "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\simple-dhcpd" \
        "Publisher" "SimpleDaemons"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\simple-dhcpd" \
        "DisplayVersion" "${VERSION}"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\simple-dhcpd" \
        "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\simple-dhcpd" \
        "NoRepair" 1
SectionEnd

Section "Start Menu Shortcuts" SecShortcuts
    CreateDirectory "$SMPROGRAMS\SimpleDaemons"
    CreateShortcut "$SMPROGRAMS\SimpleDaemons\simple-dhcpd.lnk" "$INSTDIR\simple-dhcpd.exe"
    CreateShortcut "$SMPROGRAMS\SimpleDaemons\Uninstall simple-dhcpd.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Windows Service" SecService
    ; Install as Windows service
    ExecWait '"$INSTDIR\simple-dhcpd.exe" install'
SectionEnd

;--------------------------------
; Descriptions

LangString DESC_SecCore ${LANG_ENGLISH} "Core application files (required)"
LangString DESC_SecShortcuts ${LANG_ENGLISH} "Create Start Menu shortcuts"
LangString DESC_SecService ${LANG_ENGLISH} "Install as Windows Service"

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} $(DESC_SecCore)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcuts} $(DESC_SecShortcuts)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecService} $(DESC_SecService)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
; Uninstaller

Section "Uninstall"
    ; Remove service
    ExecWait '"$INSTDIR\simple-dhcpd.exe" uninstall'
    
    ; Remove files
    RMDir /r "$INSTDIR"
    
    ; Remove shortcuts
    Delete "$SMPROGRAMS\SimpleDaemons\simple-dhcpd.lnk"
    Delete "$SMPROGRAMS\SimpleDaemons\Uninstall simple-dhcpd.lnk"
    RMDir "$SMPROGRAMS\SimpleDaemons"
    
    ; Remove registry entries
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\simple-dhcpd"
SectionEnd

