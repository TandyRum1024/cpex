@echo off
SETLOCAL

SET CMAKE_CONFIG=Debug
SET ARCHITECTURE=x64
SET RUN=1
SET CLEAN=0
SET SKIP_CONF=0

:: Check for no args
if "%1" == "-h" (
    ECHO "USAGE: %0 --config [Debug|Release] (default: Debug) --architecture x64 (default: x64) [-skip-config] [-run] [-clean]"
    EXIT /B
)
if "%1" == "--help" (
    ECHO "USAGE: %0 --config [Debug|Release] (default: Debug) --architecture x64 (default: x64) [-skip-config] [-run] [-clean]"
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

if "%1" == "-run" (
    SET RUN=1
    SHIFT
    goto param_parse
)

if "%1" == "-clean" (
    SET CLEAN=1
    SHIFT
    goto param_parse
)

if "%1" == "-skip-config" (
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
    cmake --build ./out/gen --config %CMAKE_CONFIG% --target CLEAN_APP_DATA
    cmake --build ./out/gen --config %CMAKE_CONFIG% --target clean
    rd /s /q "./out/gen"
)

:: Actual building
:: (configure)
IF NOT %SKIP_CONF% == 1 (
    cmake ^
        -G "Visual Studio 17 2022" ^
        -B ./out/gen ^
        -T host=%ARCHITECTURE% ^
        -A %ARCHITECTURE%
)

:: (build)
cmake --build ./out/gen --config %CMAKE_CONFIG%

IF %RUN% == 1 (
    :: Run!
    CALL "./out/bin/%CMAKE_CONFIG%/cpex_t1.exe"
)

ENDLOCAL