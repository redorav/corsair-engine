require("../../premake/corsairengine/common")

ProjectName = "Glslang"

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
	
	filter("toolset:msc")
		flags { "multiprocessorcompile" }
	
	editandcontinue("off")
	
	configuration "Debug"
		defines { "DEBUG", "_DEBUG" }
		symbols("on")

	configuration "Release"
		defines { "NDEBUG" }
		optimize("on")
	
project (ProjectName)
	kind("StaticLib")
	language("C++")

	targetname("%{wks.name}.".._ACTION..".%{cfg.buildcfg:lower()}")
	targetdir("Binaries/")

	files
	{
		"Source/glslang/GenericCodeGen/**.cpp", "Source/glslang/GenericCodeGen/**.h",
		"Source/glslang/Include/**.cpp", "Source/glslang/Include/**.h",
		"Source/glslang/MachineIndependent/**.cpp", "Source/glslang/MachineIndependent/**.h",
		"Source/glslang/HLSL/**.cpp", "Source/glslang/HLSL/**.h",
		"Source/glslang/Public/**.cpp", "Source/glslang/Public/**.h",
		"Source/glslang/OSDependent/*.h",
		"Source/hlsl/**.cpp", "Source/hlsl/**.h",
		"Source/SPIRV/**.cpp", "Source/SPIRV/**.h",
		"Source/OGLCompilersDLL/**.cpp", "Source/OGLCompilersDLL/**.h",
	}	
			
	sysincludedirs
	{
		"Source/", 
		"Source/OGLCompilersDLL"
	}
	
	filter("system:windows")
		files { "Source/glslang/OSDependent/windows/**.cpp", "Source/glslang/OSDependent/windows/**.h" }
	
	filter("system:not windows")
		files { "Source/glslang/OSDependent/unix/**.cpp", "Source/glslang/OSDependent/unix/**.h" }
	
	filter {}
	
	defines { "ENABLE_HLSL" }
	
	filter { "configurations:*" } -- Workaround for MacOS nil in cfg