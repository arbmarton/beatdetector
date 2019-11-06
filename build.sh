mkdir -p out
cd out

cmake -G"Visual Studio 15 Win64" ${COMMON_CMAKE_CONFIG_PARAMS} ../
cmake --build . --config Debug
cmake --build . --config Release