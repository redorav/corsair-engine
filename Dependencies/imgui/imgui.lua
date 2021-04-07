require("../../premake/corsairengine/common")

ProjectName = "ImGui"

workspace(ProjectName)
	configurations { "Debug", "Release" }
	location ("Build/".._ACTION)
	architecture("x64")
	cppdialect("c++17")
	
	if _ACTION == "vs2017" then
		toolset("msc") -- Use default VS toolset TODO do this platform specific
		--toolset("msc-llvm-vs2014") -- Use for Clang on VS
	end

	editandcontinue("off")
	
	configuration "Debug"
		defines { "DEBUG" }
		debugformat("c7") -- Do not create pdbs, instead store in lib
		symbols ("on")

	configuration "Release"
		defines { "NDEBUG" }
		optimize ("on")
	
project (ProjectName)
	kind("StaticLib")
	language("C++")
	
	targetdir("Binaries/")
	targetname("%{wks.name}.".._ACTION..".%{cfg.buildcfg:lower()}")

	files
	{
		"Source/*.cpp",
		"Source/*.hpp",
		"Source/*.h",
	}
	
	removefiles {}
	
	defines {}
	
	filter { "configurations:*" } -- Workaround for MacOS nil in cfg