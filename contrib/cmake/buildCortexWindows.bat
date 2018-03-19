rem We hide the CMakeLists.txt here and copy it to root so it works correctly
set ROOT_DIR=%~dp0%..\..
cd %ROOT_DIR%
xcopy /s /y contrib\cmake\* .

rem it looks like this should be covered by Gaffer Dependencies buildResources.bat?
rem mkdir %BUILD_DIR%\resources\cortex
rem copy %~dp0%..\cortex-10.0.0-a8\tileset_2048.dat %BUILD_DIR%\resources\cortex

mkdir %BUILD_DIR%\doc\licenses
copy LICENSE %BUILD_DIR%\doc\licenses\cortex

mkdir gafferBuild
cd gafferBuild
del /f CMakeCache.txt

if "%ARNOLD_ROOT%" NEQ "" (
	set ARNOLD_OPTIONS="-DWITH_IECORE_ARNOLD=1 -DARNOLD_ROOT=%ARNOLD_ROOT%"
)

cmake -Wno-dev -G %CMAKE_GENERATOR% -DCMAKE_INSTALL_PREFIX=%BUILD_DIR% -DPYTHON_LIBRARY=%BUILD_DIR%\lib -DPYTHON_INCLUDE_DIR=%BUILD_DIR%\include -DILMBASE_LOCATION=%BUILD_DIR% -DOPENEXR_LOCATION=%BUILD_DIR% -DWITH_IECORE_GL=1 -DWITH_IECORE_IMAGE=1 -DWITH_IECORE_SCENE=1 %ARNOLD_OPTIONS% -DWITH_IECORE_ALEMBIC=1 -DALEMBIC_ROOT=%BUILD_DIR% -DWITH_IECORE_APPLESEED=1 -DAPPLESEED_INCLUDE_DIR=%BUILD_DIR%\appleseed\include -DAPPLESEED_LIBRARY=%BUILD_DIR%\appleseed\lib\appleseed.lib ..
if %ERRORLEVEL% NEQ 0 (exit /b %ERRORLEVEL%)
cmake --build . --config %BUILD_TYPE% --target install
if %ERRORLEVEL% NEQ 0 (exit /b %ERRORLEVEL%)

cd %ROOT_DIR%
