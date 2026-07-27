@echo off
setlocal enabledelayedexpansion

REM ===================================================================
REM  buildvs.bat
REM  Build the FEMproject Visual Studio solution located in .\buildvs.
REM
REM  Usage:
REM    buildvs.bat                 incremental build, Release config
REM    buildvs.bat Debug           incremental build, given config
REM    buildvs.bat Release clean   clean-then-build (full rebuild)
REM    buildvs.bat clean           clean build with default (Release) config
REM
REM  Output: buildvs\output\<CONFIG>\  (CDFEG.dll + sample executables)
REM ===================================================================

set "SCRIPT_DIR=%~dp0"
set "BUILD_DIR=%SCRIPT_DIR%buildvs"
set "CONFIG=Release"
set "CLEAN_FLAG="

REM ---- Parse arguments: first non-keyword arg = config, "clean" = rebuild ----
:parse_args
if "%~1"=="" goto args_done
set "ARG=%~1"
if /I "!ARG!"=="clean" (
    set "CLEAN_FLAG=--clean-first"
) else (
    set "CONFIG=!ARG!"
)
shift
goto parse_args
:args_done

REM ---- Locate cmake ----
where cmake >nul 2>nul
if errorlevel 1 (
    echo [ERROR] cmake not found in PATH.
    echo         Add D:\greensoft\cmake-4.1.1\bin to PATH.
    exit /b 1
)

REM ---- Verify VS solution exists ----
if not exist "%BUILD_DIR%\CDFEG.sln" (
    echo [ERROR] VS solution not found: %BUILD_DIR%\CDFEG.sln
    echo         Configure first, e.g.:
    echo         cmake -B buildvs -G "Visual Studio 17 2022"
    exit /b 1
)

echo [INFO] Solution : %BUILD_DIR%\CDFEG.sln
echo [INFO] Config    : %CONFIG%
if defined CLEAN_FLAG (
    echo [INFO] Mode      : clean-then-build
) else (
    echo [INFO] Mode      : incremental
)

REM ---- Build ----
cmake --build "%BUILD_DIR%" --config %CONFIG% %CLEAN_FLAG%
set "RC=%errorlevel%"

if "%RC%"=="0" (
    echo [OK]   Build succeeded. Output: %BUILD_DIR%\output\%CONFIG%
) else (
    echo [FAIL] Build failed with exit code %RC%.
)

exit /b %RC%
