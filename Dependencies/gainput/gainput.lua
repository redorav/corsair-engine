require("../../premake/corsairengine/common")

ProjectName = "Gainput"

workspace(ProjectName)
	configurations { "Debug", "Release" }
	location ("Build/".._ACTION)
	architecture("x64")
	cppdialect("c++17")
	debugformat("c7") -- Do not create pdbs, instead store in lib
		
	if _ACTION == "vs2017" then
		system("windows")
		toolset("msc")
	elseif _ACTION == "xcode4" then
		system("macosx")
	end
	
	filter("toolset:msc")
		flags { "multiprocessorcompile" }
	
	targetdir("Binaries/")
	targetname("%{wks.name}.".._ACTION..".%{cfg.buildcfg:lower()}")
	
	configuration "Debug"
		--, "GAINPUT_DEBUG" 
		defines { "DEBUG", "_DEBUG" }
		symbols "on"

	configuration "Release"
		defines { "NDEBUG" }
		optimize "on"
	
project (ProjectName)
	kind("StaticLib")
	language("C++")
	
	files { "Source/lib/**.cpp", "Source/lib/**.hpp", "Source/lib/**.h" }
	sysincludedirs { "Source/lib/include" }
	
	removefiles { "**netconnection**", "**netaddress**", "**netlistener**" }
	removefiles { "Source/**Mac.cpp" }
	
	filter("system:macosx")
		files { "Source/**Mac.cpp" }
	
	filter { "configurations:*" } -- Workaround for MacOS nil in cfg