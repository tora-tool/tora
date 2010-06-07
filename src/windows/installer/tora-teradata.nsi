; TOra.nsi
;
; This script is a basic TOra installer: it remembers the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install TOra into a directory that the user selects,

;--------------------------------

; The name of the installer
Name "TOra"

; The file to write
OutFile "TOra-ora11-td13-2.3.1.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Tora"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKCU "Software\Tora" "Install_Dir"

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Tora (required)"

  SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
 
  ; Put file there
  File /r files\*.*
  
  ; Write the installation path into the registry
  WriteRegStr HKCU "Software\Tora" "Install_Dir" "$INSTDIR"

  ; Required Teradata keys for gss.
  WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Teradata" "" ""
  WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Teradata\Teradata GSS" "InstallDir" "$PROGRAMFILES\Tora\"
  WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Teradata\Teradata GSS\nt-i386" "ClientCurrentVersion" "13.10.00.02"
  WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Teradata\Teradata GSS\nt-i386\13.10.00.02" "" ""
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tora" "DisplayName" "TOra"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tora" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tora" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tora" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Tora"
  CreateShortCut "$SMPROGRAMS\Tora\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Tora\TOra.lnk" "$INSTDIR\tora.exe" "" "$INSTDIR\tora.exe" 0
  
SectionEnd


;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tora"
  
  ; Currently uninstaller preserves all the settings
  ; uncomment line below if you want otherwise
  ; DeleteRegKey HKCU "Software\Tora"

  ; Remove files from directories
  Delete $INSTDIR\*.*
  Delete $INSTDIR\sqldrivers\*.*
  Delete $INSTDIR\help\*.*
  Delete $INSTDIR\help\api\*.*
  Delete $INSTDIR\help\images\*.*
  
  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Tora\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Tora"
  RMDir "$INSTDIR\sqldrivers"
  RMDir "$INSTDIR\help\api"
  RMDir "$INSTDIR\help\images"
  RMDir "$INSTDIR\help"
  RMDir "$INSTDIR"

SectionEnd
