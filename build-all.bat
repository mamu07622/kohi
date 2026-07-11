@ECHO OFF
REM Build Everything
SetLocal

PUSHD "%~dp0"

ECHO "Building everything..."

IF NOT EXIST "engine\build.bat" (
	ECHO Missing engine\build.bat
	POPD
	EXIT /B 1
)

PUSHD engine
CALL .\build.bat
SET BUILD_ERROR=%ERRORLEVEL%
POPD
IF %BUILD_ERROR% NEQ 0 (
	ECHO Engine build failed with error %BUILD_ERROR%.
	POPD
	EXIT /B %BUILD_ERROR%
)

IF NOT EXIST "testbed\build.bat" (
	ECHO Missing testbed\build.bat
	POPD
	EXIT /B 1
)

PUSHD testbed
CALL .\build.bat
SET BUILD_ERROR=%ERRORLEVEL%
POPD
IF %BUILD_ERROR% NEQ 0 (
	ECHO Testbed build failed with error %BUILD_ERROR%.
	POPD
	EXIT /B %BUILD_ERROR%
)

ECHO "All assemblies built successfully."
POPD
EXIT /B 0