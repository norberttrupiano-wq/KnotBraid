; ============================================================
; KnotBraid installer script
; Compatible with Inno Setup 6.7.1
; Compile from the directory containing this script.
; ============================================================

#define MyAppName "KnotBraid"
#define MyAppVersion "1.0"
#define MyReleaseLabel "V1.0"
#define MyAppPublisher "Norbert Trupiano"
#define MyAppURL "https://github.com/norberttrupiano-wq/LogiKnotting"
#define MyAppExeName "KnotBraid.exe"
#define MyManualName "KnotBraid-Manuel-Utilisateur.pdf"
#define MyEnglishManualName "KnotBraid-User-Manual-en.pdf"
#define MySourceReleaseDir "..\KnotBraidLauncher\build\Release"
#define MySourceManual "..\Docs\KnotBraid-Manuel-Utilisateur.pdf"
#define MySourceEnglishManual "..\Docs\KnotBraid-User-Manual-en.pdf"
#define MyLicenseFile "License-KnotBraid.txt"

[Setup]
AppId={{A7E9C2C7-4D4D-4F55-9A7D-B9B1D7A0A2E1}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
AppCopyright=(c) Norbert Trupiano
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
UninstallDisplayIcon={app}\{#MyAppExeName}
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
ChangesAssociations=no
DisableProgramGroupPage=yes
CloseApplications=yes
CloseApplicationsFilter={#MyAppExeName}
LicenseFile={#MyLicenseFile}
OutputDir=Output
OutputBaseFilename=KnotBraid-Setup-{#MyReleaseLabel}
PrivilegesRequired=admin

[Languages]
Name: "french"; MessagesFile: "compiler:Languages\French.isl"

[Tasks]
Name: "desktopicon"; Description: "Creer un raccourci sur le Bureau"; GroupDescription: "Raccourcis :"; Flags: unchecked

[Files]
Source: "{#MySourceReleaseDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MySourceManual}"; DestDir: "{app}"; DestName: "{#MyManualName}"; Flags: ignoreversion
Source: "{#MySourceManual}"; DestDir: "{app}\docs"; DestName: "{#MyManualName}"; Flags: ignoreversion
Source: "{#MySourceEnglishManual}"; DestDir: "{app}"; DestName: "{#MyEnglishManualName}"; Flags: ignoreversion
Source: "{#MySourceEnglishManual}"; DestDir: "{app}\docs"; DestName: "{#MyEnglishManualName}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autoprograms}\Aide KnotBraid"; Filename: "{app}\docs\{#MyManualName}"
Name: "{autoprograms}\KnotBraid User Manual (English)"; Filename: "{app}\docs\{#MyEnglishManualName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Lancer {#MyAppName}"; Flags: nowait postinstall skipifsilent

[Code]
procedure DeleteUserSettingsKeys();
begin
  RegDeleteKeyIncludingSubkeys(HKCU, 'Software\KnotBraid\Shell');
  RegDeleteKeyIncludingSubkeys(HKCU, 'Software\LogiKnotting\LogiKnotting');
  RegDeleteKeyIncludingSubkeys(HKCU, 'Software\LogiBraiding\LogiBraiding');
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then
    DeleteUserSettingsKeys();
end;
