@echo off
start /B /W ../../Premake/premake5 --file=glslang.lua vs2019
rem cd Build
rem Gainput.sln
echo Build finished succesfully
pause