@echo off

pushd %~dp0

rem Checks if cl is defined. If not call vcvars64.bat.
where /q cl
if errorlevel 1 (
    call "run_vcvar.bat"
)

rem call "copy_asan_dll.bat"

pushd ..

mkdir Bin
copy DLL\* Bin\

setlocal enabledelayedexpansion

IF "%~1"=="" (
    set outputTarget=""
) ELSE (
    set outputTarget=> scripts/logfile.txt 2>&1
)

rem /O2
@set CC=cl /fsanitize=address
@set FLAGS=/std:c++17 /D_CRT_SECURE_NO_WARNINGS /DSDL_MAIN_HANDLED /Z7 /W4 /WX /MP /EHsc /I "./Include"
@set "SRC_FILES="
rem set "SRC_FILES=!SRC_FILES! "Src/main.cpp""
for /r "Src" %%I in (*.c) do (
    set "SRC_FILES=!SRC_FILES! "%%~fI""
)
for /r "Src" %%I in (*.cpp) do (
    set "SRC_FILES=!SRC_FILES! "%%~fI""
)

for /r "Include" %%I in (*.c) do (
    set "SRC_FILES=!SRC_FILES! "%%~fI""
)
for /r "Include" %%I in (*.cpp) do (
    set "SRC_FILES=!SRC_FILES! "%%~fI""
)

rem vcperf /start SessionName

%CC% %FLAGS% %SRC_FILES% /MP /link /out:Site/site_generator.exe /LIBPATH:"./Lib" Shell32.lib %outputTarget%

rem vcperf /stop SessionName /timetrace outputFile.json

del /S *.obj 1>nul 2>nul
del /S vc140.pdb 1>nul 2>nul

endlocal

popd
popd