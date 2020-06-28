require("../../premake/corsairengine/common")

ProjectName = "SPIRV-Cross"

workspace(ProjectName)
	configurations { "Debug", "Release" }
	location ("Build/".._ACTION)
	architecture("x64")
	cppdialect("c++17")
	debugformat("c7") -- Do not create pdbs, instead store in lib
	
	if _ACTION == "vs2017" then
		toolset("msc") -- Use default VS toolset TODO do this platform specific
		--toolset("msc-llvm-vs2014") -- Use for Clang on VS
	end
	
	targetdir("Binaries/")
	targetname("%{wks.name}.".._ACTION..".%{cfg.buildcfg:lower()}")

	editandcontinue("off")
	
	configuration "Debug"
		defines { "DEBUG" }
		symbols ("on")

	configuration "Release"
		defines { "NDEBUG" }
		optimize ("on")
	
project (ProjectName)
	kind("StaticLib")
	language("C++")
	
	files { "Source/*.cpp", "Source/*.hpp", "Source/*.h", "Source/include/**.hpp" }
	
	defines { "SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS" }
	
	filter { "configurations:*" } -- Workaround for MacOS nil in cfg