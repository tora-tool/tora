<?xml version="1.0" encoding="utf-8"?>
<Include>
  <?define MajorVersion="$(var.GITVERSION_MAJOR)" ?>
  <?define MinorVersion="0" ?>
  <?define BuildVersion="$(var.GITVERSION_COUNT)" ?>

  <!-- Platform = CMAKE_SIZEOF_VOID_P 8=> 64bit -->
  <?define VoidP = "@CMAKE_SIZEOF_VOID_P@" ?>
  <?if $(var.VoidP) = 8 ?>
  <?define Platform = "x64" ?>
  <?define ProductName = "Tora (64 bit)" ?>
  <?define Win64 = "yes" ?>
  <?define PlatformProgramFilesFolder = "ProgramFiles64Folder" ?>
  <?define PlatformSystemFolder = "System64Folder" ?>
  <?else ?>
  <?define Platform = "x86" ?>
  <?define ProductName = "Tora" ?>
  <?define Win64 = "no" ?>
  <?define PlatformProgramFilesFolder = "ProgramFilesFolder" ?>
  <?define PlatformSystemFolder = "SystemFolder" ?>
  <?endif ?>

  <!-- Revision is NOT used by WiX in the upgrade procedure -->
  <?define Revision="0" ?>
  <!-- Full version number to display -->
  <?define ProductVersion="$(var.MajorVersion).$(var.MinorVersion).$(var.BuildVersion).$(var.Revision)" ?>

  <!-- Upgrade code HAS to be the same for all updates. Once you've chosen it DON'T CHANGE it. -->
  <?define UpgradeCode="BAAA2F25-E626-4CC5-BC37-86477A2409AC" ?>

  <?define BuildType = "@CMAKE_BUILD_TYPE@" ?>
  <?define BUILD_ABS_PATH = "@EXECUTABLE_OUTPUT_PATH@" ?>
</Include>
