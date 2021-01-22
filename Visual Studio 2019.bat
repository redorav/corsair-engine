@echo off
start /B /W Premake/premake5 --file=corsairengine.lua vs2019
rem cd VSFiles
rem CorsairEngine.sln
mkdir "Tools/Shader Compiler"
echo Build finished succesfully
pause