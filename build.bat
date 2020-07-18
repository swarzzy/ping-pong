@echo off

set ObjOutDir=build\obj\
set BinOutDir=build\

IF NOT EXIST %BinOutDir% mkdir %BinOutDir%
IF NOT EXIST %ObjOutDir% mkdir %ObjOutDir%

del %BinOutDir%*.pdb >NUL 2>&1
set PdbMangleVal=%date:~6,4%%date:~3,2%%date:~0,2%%time:~1,1%%time:~3,2%%time:~6,2%

set CommonDefines=/Iext\SDL2-2.0.12\include /D_CRT_SECURE_NO_WARNINGS /DWIN32_LEAN_AND_MEAN /DPLATFORM_WINDOWS /DUNICODE /D_UNICODE
set CommonCompilerFlags=/Gm- /fp:fast /GR- /nologo /diagnostics:classic /WX /std:c++17 /Zi /W3 /FS
set DebugCompilerFlags=/Od /RTC1 /MTd /Fd%BinOutDir%\
set ReleaseCompilerFlags=/O2 /MT
set SDLDependencies=build/SDL2d.lib shell32.lib version.lib imm32.lib ole32.lib oleaut32.lib advapi32.lib setupapi.lib
set PlatformLinkerFlags=/INCREMENTAL:NO /OPT:REF /MACHINE:X64 /NOIMPLIB %SDLDependencies% user32.lib gdi32.lib opengl32.lib winmm.lib /OUT:%BinOutDir%\win32_pong.exe /PDB:%BinOutDir%\win32_pong.pdb
rem set GameLinkerFlags=/INCREMENTAL:NO /OPT:REF /MACHINE:X64 /DLL /OUT:%BinOutDir%\pong.dll  /PDB:%BinOutDir%\pong_%PdbMangleVal%.pdb
set GameLinkerFlags=/INCREMENTAL:NO /OPT:REF /MACHINE:X64 /DLL /OUT:%BinOutDir%\pong.dll  /PDB:%BinOutDir%\pong.pdb



set ConfigCompilerFlags=%DebugCompilerFlags%

echo Building platform...
start /b "__platform_compilation__" cmd /c cl /DPLATFORM_CODE /Fo%ObjOutDir% %CommonDefines% %CommonCompilerFlags% %ConfigCompilerFlags% src/platform/SDLWin32Platform.cpp /link %PlatformLinkerFlags%

echo Building game...
start /b /wait "__game_compilation__" cmd /c cl /Fo%ObjOutDir% %CommonDefines% %CommonCompilerFlags% %ConfigCompilerFlags% src/Game.cpp /link %GameLinkerFlags%
