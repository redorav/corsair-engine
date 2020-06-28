@echo off
start /B /W ../../Premake/premake5 --file=eastl.lua vs2019
rem cd Build
rem Eastl.sln
echo Build finished succesfully
pause