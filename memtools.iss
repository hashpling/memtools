; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define VCREDIST GetEnv("VS100COMNTOOLS") + "..\..\VC\redist"
[Setup]
AppName=Address Space Monitor
AppVerName=Address Space Monitor 0.7a
AppPublisher=Charles Bailey
AppCopyright=Copyright (c) 2007-2010 Charles Bailey
AppPublisherURL=http://www.hashpling.org
AppSupportURL=http://www.hashpling.org/asm/
AppUpdatesURL=http://www.hashpling.org/asm/
DefaultDirName={pf}\memmon
DefaultGroupName=Address Space Monitor
OutputBaseFilename=asmsetup-0_7
OutputDir=installer
Compression=lzma
SolidCompression=yes
PrivilegesRequired=none
ArchitecturesInstallIn64BitMode=x64
LicenseFile=licence.txt
VersionInfoVersion=0.7.0.0

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: full; Description: "Install win32 and x64 versions"; Check: Is64BitInstallMode
Name: t64bitonly; Description: "Install x64 version"; Check: Is64BitInstallMode
Name: t32bitonly; Description: "Install win32 version"
Name: custom; Description: "Custom"; Flags: iscustom

[Components]
Name: "common"; Description: "common files"; Types: full t32bitonly t64bitonly custom; Flags: fixed
Name: "c32bitexe"; Description: "win32 program files"; Types: full t32bitonly custom
Name: "c64bitexe"; Description: "x64 program files"; Types: full t64bitonly; Check: Is64BitInstallMode

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "win32\release\joinrec.exe"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: c32bitexe
Source: "win32\release\memcat.exe"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: c32bitexe
Source: "win32\release\memdiff.exe"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: c32bitexe
Source: "win32\release\memmon.exe"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: c32bitexe
Source: "win32\release\splitrec.exe"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: c32bitexe
Source: "{#VCREDIST}\x86\Microsoft.VC100.CRT\msvcp100.dll"; DestDir: "{app}\bin"; Components: c32bitexe
Source: "{#VCREDIST}\x86\Microsoft.VC100.CRT\msvcr100.dll"; DestDir: "{app}\bin"; Components: c32bitexe
Source: "x64\release\joinrec.exe"; DestDir: "{app}\bin64"; Flags: ignoreversion; Components: c64bitexe
Source: "x64\release\memcat.exe"; DestDir: "{app}\bin64"; Flags: ignoreversion; Components: c64bitexe
Source: "x64\release\memdiff.exe"; DestDir: "{app}\bin64"; Flags: ignoreversion; Components: c64bitexe
Source: "x64\release\memmon.exe"; DestDir: "{app}\bin64"; Flags: ignoreversion; Components: c64bitexe
Source: "x64\release\splitrec.exe"; DestDir: "{app}\bin64"; Flags: ignoreversion; Components: c64bitexe
Source: "{#VCREDIST}\x64\Microsoft.VC100.CRT\msvcr100.dll"; DestDir: "{app}\bin64"; Components: c64bitexe
Source: "{#VCREDIST}\x64\Microsoft.VC100.CRT\msvcp100.dll"; DestDir: "{app}\bin64"; Components: c64bitexe
Source: "readme.txt"; DestDir: "{app}"; Flags: isreadme; Components: common
Source: "licence.txt"; DestDir: "{app}"; Components: common
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Address Space Monitor"; Filename: "{app}\bin\memmon.exe"; Comment: "Run Address Space Monitor"; Components: c32bitexe
Name: "{commondesktop}\Address Space Monitor"; Filename: "{app}\bin\memmon.exe"; Comment: "Run Address Space Monitor"; Tasks: desktopicon; Components: c32bitexe
Name: "{group}\Address Space Monitor (64-bit)"; Filename: "{app}\bin64\memmon.exe"; Comment: "Run Address Space Monitor (64-bit)"; Components: c64bitexe
Name: "{commondesktop}\Address Space Monitor (64-bit)"; Filename: "{app}\bin64\memmon.exe"; Comment: "Run Address Space Monitor (64-bit)"; Tasks: desktopicon; Components: c64bitexe
Name: "{group}\ReadMe"; Filename: "{app}\readme.txt"; Comment: "View the readme file for Address Space Monitor"; Components: common
Name: "{group}\Licence"; Filename: "{app}\licence.txt"; Comment: "View the licence for Address Space Monitor"; Components: common

[InstallDelete]
Type: files; Name: "{app}\bin\Microsoft.VC80.CRT.manifest"
Type: files; Name: "{app}\bin\msvcm80.dll"
Type: files; Name: "{app}\bin\msvcp80.dll"
Type: files; Name: "{app}\bin\msvcr80.dll"
Type: files; Name: "{app}\bin64\Microsoft.VC80.CRT.manifest"
Type: files; Name: "{app}\bin64\msvcm80.dll"
Type: files; Name: "{app}\bin64\msvcp80.dll"
Type: files; Name: "{app}\bin64\msvcr80.dll"
Type: files; Name: "{app}\Changelog"
Type: files; Name: "{app}\src\Changelog"
Type: files; Name: "{app}\bin\Microsoft.VC90.CRT.manifest"
Type: files; Name: "{app}\bin\msvcm90.dll"
Type: files; Name: "{app}\bin\msvcp90.dll"
Type: files; Name: "{app}\bin\msvcr90.dll"
Type: files; Name: "{app}\bin64\Microsoft.VC90.CRT.manifest"
Type: files; Name: "{app}\bin64\msvcm90.dll"
Type: files; Name: "{app}\bin64\msvcp90.dll"
Type: files; Name: "{app}\bin64\msvcr90.dll"

[Registry]
Root: HKCU; SubKey: "SOFTWARE\hashpling.org"; Flags: dontcreatekey uninsdeletekeyifempty
Root: HKCU; SubKey: "SOFTWARE\hashpling.org\memmon"; Flags: dontcreatekey uninsdeletekey

;[Run]
;Filename: "{app}\memmon.exe"; Description: "{cm:LaunchProgram,memmon}"; Flags: nowait postinstall skipifsilent

