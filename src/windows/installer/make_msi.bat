rem "c:\Program Files\WiX Toolset v3.7\bin\candle.exe" MyInstallDirDlg.wxs
rem "c:\Program Files\WiX Toolset v3.7\bin\candle.exe" MyWixUI_InstallDir.wxs
rem "c:\Program Files\WiX Toolset v3.7\bin\candle.exe" tora.wxs
rem "c:\Program Files\WiX Toolset v3.7\bin\light.exe" -ext WixUIExtension -o tora.msi tora.wixobj MyInstallDirDlg.wixobj MyWixUI_InstallDir.wixobj

"c:\Program Files\WiX Toolset v3.7\bin\candle.exe" tora.wxs
"c:\Program Files\WiX Toolset v3.7\bin\light.exe" -ext WixUIExtension -o tora.msi tora.wixobj

@pause

