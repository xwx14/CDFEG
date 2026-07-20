@echo off
rem ============================================================
rem  Launch the pyTool GUI (pyTool\gui\main.py).
rem  Uses the system Python environment.
rem ============================================================

rem  Always run from this script's folder so paths resolve.
cd /d "%~dp0"

python "pyTool\gui\main.py"

rem  Keep the window open if the GUI fails to start.
if errorlevel 1 (
    echo.
    echo [ERROR] pyTool GUI failed to start.
    pause
)
