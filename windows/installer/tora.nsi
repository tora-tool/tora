; TOra.nsi
;
; This script is a basic TOra installer: it remembers the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install TOra into a directory that the user selects,

;--------------------------------

; The name of the installer
Name "TOra - Toolkit For Oracle"

; The file to write
OutFile "TOra Installer.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Toolkit For Oracle"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKCU "Software\Toolkit For Oracle" "Install_Dir"

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Toolkit For Oracle (required)"

  SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
 
  ; Put file there
  File /r files\*.*
  
  ; Write the installation path into the registry
  WriteRegStr HKCU "Software\Toolkit For Oracle" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Toolkit For Oracle" "DisplayName" "Toolkit For Oracle (TOra)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Toolkit For Oracle" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Toolkit For Oracle" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Toolkit For Oracle" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Toolkit For Oracle"
  CreateShortCut "$SMPROGRAMS\Toolkit For Oracle\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Toolkit For Oracle\TOra.lnk" "$INSTDIR\tora.exe" "" "$INSTDIR\tora.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Toolkit For Oracle"
  
  ; Currently uninstaller preserves all the settings
  ; uncomment line below if you want otherwise
  ; DeleteRegKey HKCU "Software\Toolkit For Oracle"

  ; Remove files from directories
  Delete $INSTDIR\*.*
  Delete $INSTDIR\help\*.*
  Delete $INSTDIR\help\api\*.*
  Delete $INSTDIR\help\images\*.*
  
  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Toolkit For Oracle\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Toolkit For Oracle"
  RMDir "$INSTDIR\help\api"
  RMDir "$INSTDIR\help\images"
  RMDir "$INSTDIR\help"
  RMDir "$INSTDIR"

SectionEnd
