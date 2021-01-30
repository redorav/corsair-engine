@echo off
start /B /W ../../Premake/premake5 --file=spirv-reflect.lua vs2019
echo Build finished succesfully
pause