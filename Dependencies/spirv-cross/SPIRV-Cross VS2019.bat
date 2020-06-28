@echo off
start /B /W ../../Premake/premake5 --file=spirv-cross.lua vs2019
rem cd VSFiles
rem CorsairEngine.sln
echo Build finished succesfully
pause