; DesktopPet Inno Setup Script
; Requires Inno Setup 6.7.1+

#define MyAppName "DesktopPet"
#define MyAppVersion "0.30.6"
#define MyAppPublisher "rekjorekjo"
#define MyAppExeName "DesktopPet.exe"
#define MyDistDir "..\..\dist\DesktopPet-v" + MyAppVersion + "-release"

[Setup]
AppId={{A8F2E1B6-3C74-4C0C-9DB7-7F53E7770C61}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} v{#MyAppVersion}
AppPublisher={#MyAppPublisher}
ShowLanguageDialog=no
WizardSizePercent=130
DefaultDirName={localappdata}\Programs\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=output
OutputBaseFilename=DesktopPet_Setup_v{#MyAppVersion}
SetupIconFile=..\..\resources\icons\app_icon.ico
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest
CloseApplications=yes
CloseApplicationsFilter=DesktopPet.exe,DesktopPet-resize.exe
UninstallDisplayIcon={app}\{#MyAppExeName}
VersionInfoVersion={#MyAppVersion}.0
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription={#MyAppName} Setup
VersionInfoProductName={#MyAppName}
VersionInfoProductVersion={#MyAppVersion}

[Languages]
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"

[LangOptions]
DialogFontName=Microsoft YaHei UI
DialogFontSize=10
WelcomeFontName=Microsoft YaHei UI
WelcomeFontSize=14

[Files]
Source: "{#MyDistDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "pets\*,pets,actions\*,actions,logs\*,logs,chat_logs\*,chat_logs,config\api_profiles.json,config\chat_settings.json,build\*,build,.git\*,.git"

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{#MyAppName} 图片处理工具"; Filename: "{app}\DesktopPet-resize.exe"
Name: "{group}\卸载 {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{userdesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "创建桌面快捷方式"; Flags: unchecked

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "启动 {#MyAppName}"; Flags: nowait postinstall skipifsilent
