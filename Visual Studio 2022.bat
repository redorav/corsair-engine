@echo off
start /B /W Premake/premake5 --file=corsairengine.lua vs2022
rem cd VSFiles
rem CorsairEngine.sln
echo Build finished succesfully
pause