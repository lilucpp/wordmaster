:: 设置和当前bat所在目录为basepath
set basepath=%~dp0
cd %basepath%
echo %basepath%

cd %basepath%/..
mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -T v142 -A WIN32
cmake --build . --config Release