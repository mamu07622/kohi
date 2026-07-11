REM Build script for engine
@ECHO OFF
SetLocal EnableDelayedExpansion

PUSHD "%~dp0"

IF "%VULKAN_SDK%"=="" (
    ECHO Vulkan SDK is not configured. Set VULKAN_SDK before building.
    POPD
    EXIT /B 1
)

REM Get a list of all the .c files.
SET cFilenames=
FOR /R src %%f in (*.c) do (
    SET cFilenames=!cFilenames! "%%f"
)

REM echo "Files:" %cFilenames%

IF "%cFilenames%"=="" (
    ECHO No C source files found under src.
    POPD
    EXIT /B 1
)

SET assembly=engine
SET compilerFlags=-g -shared -Wvarargs -Wall -Werror
REM -Wall -Werror
SET includeFlags=-Isrc -I"%VULKAN_SDK%/Include"
SET linkerFlags=-L"%VULKAN_SDK%/Lib" -luser32 -lvulkan-1 -Wl,--out-implib,"..\bin\engine.lib"
SET defines=-D_DEBUG -DKEXPORT -D_CRT_SECURE_NO_WARNINGS

IF NOT EXIST "..\bin" MKDIR "..\bin"

ECHO "Building %assembly%%..."
clang %cFilenames% %compilerFlags% -o "..\bin\%assembly%.dll" %defines% %includeFlags% %linkerFlags%
IF ERRORLEVEL 1 (
    ECHO Build failed.
    POPD
    EXIT /B 1
)

ECHO Build succeeded.
POPD
EXIT /B 0