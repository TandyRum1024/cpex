@echo off
SETLOCAL

SET CMAKE_CONFIG=Debug
SET ARCHITECTURE=x64
SET RUN=1
SET CLEAN=0
SET SKIP_CONF=0

:: Check for no args
if "%1" == "-h" (
    ECHO "USAGE: %0 --config [Debug|Release] (default: Debug) --architecture x64 (default: x64) [-skip-config] [-norun] [-clean]"
    EXIT /B
)
if "%1" == "--help" (
    ECHO "USAGE: %0 --config [Debug|Release] (default: Debug) --architecture x64 (default: x64) [-skip-config] [-norun] [-clean]"
    EXIT /B
)

:: Use SHIFT to parse POSIX like params
:param_parse
if "%1" == "" goto param_after

if "%1" == "--config" (
    SET CMAKE_CONFIG=%2
    SHIFT
    SHIFT
    goto param_parse
)

if "%1" == "--architecture" (
    SET ARCHITECTURE=%2
    SHIFT
    SHIFT
    goto param_parse
)

if "%1" == "-norun" (
    SET RUN=0
    SHIFT
    goto param_parse
)

if "%1" == "-clean" (
    SET CLEAN=1
    SHIFT
    goto param_parse
)

if "%1" == "-skip-config" (
    ECHO [!] TRY TO SKIP CONFIG...

    SET SKIP_CONF=1
    SHIFT
    goto param_parse
)

ECHO INVALID PARAMETER `%1`
SHIFT
goto param_parse

:param_after

:: Default values for params
IF NOT "%CMAKE_CONFIG%" == "Debug" (
    IF NOT "%CMAKE_CONFIG%" == "Release" (
        ECHO INVALID CONFIG %CMAKE_CONFIG%!
        EXIT /B
    )
)
:: IF "%ARCHITECTURE%" == "" SET ARCHITECTURE=x64
:: IF "%OP%" == "" SET OP=build

IF %CLEAN% == 1 (
    :: (clean assets)
    :: cmake --build ./out/gen --config %CMAKE_CONFIG% --target CLEAN_APP_DATA
    :: cmake --build ./out/gen --config %CMAKE_CONFIG% --target clean
    IF EXIST "%~dp0out" (
        ECHO [+] REMOVING "%~dp0out"...
        RD /s /q "%~dp0out"
    )
)

IF NOT EXIST "%~dp0out" (
    MKDIR "%~dp0out"
)

IF NOT EXIST "%~dp0out/gen" (
    ECHO [!] "%~dp0out/gen" DOES NOT EXIST, FORCING CONFIG...

    SET SKIP_CONF=0
)

:: Actual building
:: (configure)
IF %SKIP_CONF% NEQ 1 (
    ECHO [+] CONFIGURING...
    cmake -B "%~dp0out/gen" -T host=%ARCHITECTURE% -A %ARCHITECTURE%

    if %ERRORLEVEL% NEQ 0 (
        ECHO CONFIGURE FAILED!
        EXIT /B
    )
)

:: (build)
ECHO [+] BUILDING...
cmake --build "%~dp0out/gen" --config %CMAKE_CONFIG%

if %ERRORLEVEL% NEQ 0 (
    ECHO BUILD FAILED!
    EXIT /B
)

IF %RUN% == 1 (
    :: Run!
    ECHO [+] RUNNING EXECUTABLE AT %~dp0 ...
    CALL "%~dp0out/bin/%CMAKE_CONFIG%/cpex_t1.exe"
)

ENDLOCAL