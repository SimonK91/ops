@rem NOTE requires CMAKE and a MSBUILD command shell to work

@rem define build directories
@set DBG_DIR=%~dp0\build.debug
@set OPT_DIR=%~dp0\build.opt
@set BOOTSTRAP_DIR=%~dp0\build.bootstrap
@set INSTALL_PREFIX=%~dp0\deploy

@rem Perform the bootstrap process that generates source files neded by the other targets
@pushd %~dp0
@IF NOT EXIST %BOOTSTRAP_DIR% mkdir %BOOTSTRAP_DIR%
@cd %BOOTSTRAP_DIR%
cmake -DCMAKE_BUILD_TYPE=Bootstrap -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX% ..
cmake --build . --target opsidls --config Debug
@popd

@rem build and install Debug
@pushd %~dp0
@IF NOT EXIST %DBG_DIR% mkdir %DBG_DIR%
@cd %DBG_DIR%
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX% ..
cmake --build . --target ALL_BUILD --config Debug
cmake -DBUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX% -P cmake_install.cmake
@popd

@rem build and install Release
@pushd %~dp0
@IF NOT EXIST %OPT_DIR% mkdir %OPT_DIR%
@cd %OPT_DIR%
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX% ..
cmake --build . --target ALL_BUILD --config Release
cmake -DBUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX% -P cmake_install.cmake
@popd
