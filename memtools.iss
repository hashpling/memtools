; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=Address Space Monitor
AppVerName=Address Space Monitor 0.5a
AppPublisher=Charles Bailey
AppCopyright=Copyright (c) 2007 Charles Bailey
AppPublisherURL=http://www.hashpling.org
AppSupportURL=http://www.hashpling.org/asm/
AppUpdatesURL=http://www.hashpling.org/asm/
DefaultDirName={pf}\memmon
DefaultGroupName=Address Space Monitor
OutputBaseFilename=asmsetup-0_5
OutputDir=installer
Compression=lzma
SolidCompression=yes
PrivilegesRequired=none
ArchitecturesInstallIn64BitMode=x64
LicenseFile=licence.txt
VersionInfoVersion=0.5.1.1

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
Name: "source"; Description: "source files"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "win32\release\memmon.exe"; DestDir: "{app}\bin"; Flags: ignoreversion; Components: c32bitexe
Source: "redist\x86\Microsoft.VC80.CRT\Microsoft.VC80.CRT.manifest"; DestDir: "{app}\bin"; Components: c32bitexe
Source: "redist\x86\Microsoft.VC80.CRT\msvcm80.dll"; DestDir: "{app}\bin"; Components: c32bitexe
Source: "redist\x86\Microsoft.VC80.CRT\msvcp80.dll"; DestDir: "{app}\bin"; Components: c32bitexe
Source: "redist\x86\Microsoft.VC80.CRT\msvcr80.dll"; DestDir: "{app}\bin"; Components: c32bitexe
Source: "x64\release\memmon.exe"; DestDir: "{app}\bin64"; Flags: ignoreversion; Components: c64bitexe
Source: "redist\amd64\Microsoft.VC80.CRT\Microsoft.VC80.CRT.manifest"; DestDir: "{app}\bin64"; Components: c64bitexe
Source: "redist\amd64\Microsoft.VC80.CRT\msvcm80.dll"; DestDir: "{app}\bin64"; Components: c64bitexe
Source: "redist\amd64\Microsoft.VC80.CRT\msvcr80.dll"; DestDir: "{app}\bin64"; Components: c64bitexe
Source: "redist\amd64\Microsoft.VC80.CRT\msvcp80.dll"; DestDir: "{app}\bin64"; Components: c64bitexe
Source: "readme.txt"; DestDir: "{app}"; Flags: isreadme; Components: common
Source: "licence.txt"; DestDir: "{app}"; Components: common
Source: "Changelog"; DestDir: "{app}"; Components: common
Source: "readme.txt"; DestDir: "{app}\src"; Components: source
Source: "licence.txt"; DestDir: "{app}\src"; Components: source
Source: "Changelog"; DestDir: "{app}\src"; Components: source
Source: "memtools.sln"; DestDir: "{app}\src"; Components: source
Source: "memtools.iss"; DestDir: "{app}\src"; Components: source
Source: "no_deprecate.vsprops"; DestDir: "{app}\src"; Components: source
Source: "memmon\*"; Excludes: "*.aps,.*,*.user"; DestDir: "{app}\src\memmon"; Components: source
Source: "fmtlib\*"; Excludes: "*.aps,.*,*.user"; DestDir: "{app}\src\fmtlib"; Components: source
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Address Space Monitor"; Filename: "{app}\bin\memmon.exe"; Comment: "Run Address Space Monitor"; Components: c32bitexe
Name: "{commondesktop}\Address Space Monitor"; Filename: "{app}\bin\memmon.exe"; Comment: "Run Address Space Monitor"; Tasks: desktopicon; Components: c32bitexe
Name: "{group}\Address Space Monitor (64-bit)"; Filename: "{app}\bin64\memmon.exe"; Comment: "Run Address Space Monitor (64-bit)"; Components: c64bitexe
Name: "{commondesktop}\Address Space Monitor (64-bit)"; Filename: "{app}\bin64\memmon.exe"; Comment: "Run Address Space Monitor (64-bit)"; Tasks: desktopicon; Components: c64bitexe
Name: "{group}\ReadMe"; Filename: "{app}\readme.txt"; Comment: "View the readme file for Address Space Monitor"; Components: common
Name: "{group}\Licence"; Filename: "{app}\licence.txt"; Comment: "View the licence for Address Space Monitor"; Components: common

[Registry]
Root: HKCU; SubKey: "SOFTWARE\hashpling.org"; Flags: dontcreatekey uninsdeletekeyifempty
Root: HKCU; SubKey: "SOFTWARE\hashpling.org\memmon"; Flags: dontcreatekey uninsdeletekey

;[Run]
;Filename: "{app}\memmon.exe"; Description: "{cm:LaunchProgram,memmon}"; Flags: nowait postinstall skipifsilent

