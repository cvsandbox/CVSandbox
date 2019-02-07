; Computer Vision Sandbox set-up project

[Setup]
AppName=Computer Vision Sandbox
AppVerName=Computer Vision Sandbox 2.0.0
AppPublisher=Andrew Kirillov
AppPublisherURL=http://www.cvsandbox.com/
AppSupportURL=http://www.cvsandbox.com/
AppUpdatesURL=http://www.cvsandbox.com/
VersionInfoCompany=Andrew Kirillov
VersionInfoVersion=2.0.0
VersionInfoCopyright=Copyright © 2019 Andrew Kirillov
DefaultDirName={pf}\Computer Vision Sandbox
DefaultGroupName=Computer Vision Sandbox
AllowNoIcons=yes
OutputBaseFilename=setup
Compression=lzma
SolidCompression=yes
LicenseFile=eula.txt
WizardImageFile=cvs_wizard_image.bmp
WizardSmallImageFile=cvs_wizard_small_image.bmp

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Components]
Name: "core"; Description: "Core Files"; Types: full compact custom; Flags: fixed
Name: "plugins"; Description: "Plug-ins"
Name: "plugins\core"; Description: "Core Plug-ins"; Types: full compact custom; Flags: fixed
Name: "plugins\lua"; Description: "Lua Scripting"; Types: full custom
Name: "plugins\vcam"; Description: "CVSandbox Virtual Camera"; Types: full custom
Name: "plugins\raspberry"; Description: "Raspberry Pi support"; Types: full custom
Name: "plugins\gamepad"; Description: "Gamepad devices"; Types: full custom

[Files]
Source: "Files\*"; DestDir: "{app}"; Components: core;
Source: "Files\platforms\*"; DestDir: "{app}\platforms"; Components: core;
Source: "Files\cvsplugins\*"; DestDir: "{app}\cvsplugins"; Components: "plugins\core"; Flags: recursesubdirs
Source: "Files\Lua\cvsplugins\*"; DestDir: "{app}\cvsplugins"; Components: "plugins\lua"; Flags: recursesubdirs
Source: "Files\scripting_samples\*.lua"; DestDir: "{app}\scripting_samples"; Components: "plugins\lua"
Source: "Files\CVSCamera\cvsplugins\*"; DestDir: "{app}\cvsplugins"; Components: "plugins\vcam"; Flags: recursesubdirs
Source: "Files\CVSCamera\vcam\cvs_vcam.dll"; DestDir: "{app}\vcam"; Components: "plugins\vcam"; Flags: regserver 32bit
Source: "Files\CVSCamera\vcam\cvs_vcam64.dll"; DestDir: "{app}\vcam"; Components: "plugins\vcam"
Source: "Files\CVSCamera\vcam\regsvr.bat"; DestDir: "{app}\vcam"; Components: "plugins\vcam"
Source: "Files\CVSCamera\vcam\Release Notes.txt"; DestDir: "{app}\vcam"; Components: "plugins\vcam"
Source: "Files\Raspberry\cvsplugins\*"; DestDir: "{app}\cvsplugins"; Components: "plugins\raspberry"; Flags: recursesubdirs
Source: "Files\Gamepad\cvsplugins\*"; DestDir: "{app}\cvsplugins"; Components: "plugins\gamepad"; Flags: recursesubdirs
Source: "Files\eula.txt"; DestDir: "{app}"; Components: core
Source: "Files\LICENSE"; DestDir: "{app}"; Components: core
Source: "Files\Release notes.txt"; DestDir: "{app}"; Components: core

[Icons]
Name: "{group}\Computer Vision Sandbox"; Filename: "{app}\cvsandbox.exe"
Name: "{commondesktop}\Computer Vision Sandbox"; Filename: "{app}\cvsandbox.exe"
Name: "{group}\Project Home"; Filename: "http://www.cvsandbox.com/"
Name: "{group}\Release Notes"; Filename: "{app}\Release notes.txt"
Name: "{group}\{cm:UninstallProgram,Computer Vision Sandbox}"; Filename: "{uninstallexe}"
