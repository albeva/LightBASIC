@echo off

set VERSION=llvm-12.0.1

set SRC=%cd%\%VERSION%.src
set BUILD=%cd%\%VERSION%-build
set INSTALL=%cd%\%VERSION%-dist

if not exist "%BUILD%" mkdir %BUILD%

cmake -G "Ninja" -S "%SRC%" -B "%BUILD%" ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL%" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_CXX_STANDARD=17 ^
    -DLLVM_ENABLE_ASSERTIONS=ON ^
    -DLLVM_TARGETS_TO_BUILD=X86 ^
    -DLLVM_OPTIMIZED_TABLEGEN=ON ^
    -DLLVM_INSTALL_UTILS=ON

ninja -C "%BUILD%" install

setx LLVM_DIR "%INSTALL%\lib\cmake\llvm"
