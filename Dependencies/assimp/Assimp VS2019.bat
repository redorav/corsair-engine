@echo off
start /B /W ../../Premake/premake5 --file=assimp.lua vs2019
rem cd Build
rem Assimp.sln
echo Build finished succesfully
pause