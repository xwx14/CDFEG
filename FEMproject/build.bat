@echo off
setlocal

set BUILD_DIR=build
set CONFIG=Release

if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cmake -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=%CONFIG%

endlocal
