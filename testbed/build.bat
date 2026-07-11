REM Build script for testbed
@ECHO OFF
SetLocal EnableDelayedExpansion

PUSHD "%~dp0"

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

SET assembly=testbed
SET compilerFlags=-g -Wall -Wextra
REM -Wall -Werror
SET includeFlags=-Isrc -I../engine/src/
SET linkerFlags="..\bin\engine.lib"
SET defines=-D_DEBUG -DKIMPORT

IF NOT EXIST "..\bin" MKDIR "..\bin"

ECHO "Building %assembly%%..."
clang %cFilenames% %compilerFlags% -o "..\bin\%assembly%.exe" %defines% %includeFlags% %linkerFlags%
IF ERRORLEVEL 1 (
    ECHO Build failed.
    POPD
    EXIT /B 1
)

ECHO Build succeeded.
POPD
EXIT /B 0