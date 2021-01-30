@echo off
start /B /W ../../Premake/premake5 --file=imgui.lua vs2019
rem cd VSFiles
rem CorsairEngine.sln
echo Build finished succesfully
pause