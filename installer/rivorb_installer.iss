#define MyAppName      "Rivorb"
#define MyAppVersion   "1.0.0"
#define MyAppPublisher "Your Name Audio"

#ifndef MyBuildDir
  #define MyBuildDir "..\build\Rivorb_artefacts\Release\VST3"
#endif
#ifndef MyOutputDir
  #define MyOutputDir "output"
#endif

[Setup]
AppId={{YOUR-GUID-HERE}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={commonpf64}\Common Files\VST3
DisableDirPage=yes
OutputDir={#MyOutputDir}
OutputBaseFilename=Rivorb_v{#MyAppVersion}_Windows
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#MyBuildDir}\Rivorb.vst3\*"; \
  DestDir: "{commonpf64}\Common Files\VST3\Rivorb.vst3"; \
  Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"