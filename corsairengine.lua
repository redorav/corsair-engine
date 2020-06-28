require ('premake-qt/qt')

-- Directories
dependencies = "Dependencies"
src = "Source"
data = "Data"

srcMath = src.."/Math"
srcQt = src.."/Qt"

-- Platforms
VulkanWin64 	= "Vulkan Win64"
VulkanARM 		= "Vulkan ARM"
D3D12 			= "D3D12 Win64"
VulkanOSX		= "Vulkan OSX"

-- Make this configuration-dependent
Workspace = "Workspace/".._ACTION

-- Project Names
ProjectCorsairEngine 	= "Corsair Engine"
ProjectCrMath 			= "CrMath"
ProjectCrRendering 		= "CrRendering"
ProjectShaderCompiler	= "CrShaderCompiler"
ProjectShaders			= "CrShaders"
ProjectCrCore 			= "CrCore"
ProjectCrDebug 			= "CrDebug"
ProjectCrResource		= "CrResource"
ProjectCrImport			= "CrImport"

-- Main executable folder
MainBuildFolder = path.getabsolute(Workspace.."/Build")
MainExecutableFolder = "%{cfg.buildtarget.directory}"

-- Library Names

LibVulkan 				= dependencies.."/vulkan"
LibEASTL 				= dependencies.."/EASTL"
LibGlslang				= dependencies.."/glslang"
LibGainput 				= dependencies.."/gainput"
LibSPIRVCross 			= dependencies.."/SPIRV-Cross"
LibSPIRVTools 			= dependencies.."/SPIRV-Tools"
LibHlslpp	 			= dependencies.."/hlslpp"
LibStb					= dependencies.."/stb"
LibxxHash				= dependencies.."/xxHash"
LibAssimp				= dependencies.."/Assimp"
LibArgh					= dependencies.."/Argh"
LibHalf					= dependencies.."/Half"
LibQt					= dependencies.."/qt"
LibDdspp				= dependencies.."/ddspp"

libConfig = ".".._ACTION..".%{cfg.buildcfg:lower()}" -- Careful, the names are debug and release but this depends on this project's naming as well

shaderGenDir = Workspace.."/BuiltShaders"

binaryDir = "/Binaries/"

-- todo put in common
-- Adds a library to a project
function AddLibrary(includeDirs, libDirs, libNames)

	includedirs(includeDirs)
	libdirs (libDirs)
	links(libNames)

end

function AddAssimpLibrary()
	AddLibrary(LibAssimp.."/Source/include", LibAssimp..binaryDir, "Assimp"..libConfig)
end

function AddVulkanLibrary()
	AddLibrary(LibVulkan.."/Source/include", LibVulkan..binaryDir, "vulkan-1")
end

function AddXinputLibrary()
	AddLibrary("", "", "Xinput9_1_0")
end

function AddSpirvCrossLibrary()
	AddLibrary({ LibSPIRVCross.."/Source", LibSPIRVCross.."/Source/include" }, LibSPIRVCross..binaryDir, "SPIRV-Cross"..libConfig)
	defines { "SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS" }
end

function AddGlslangLibrary()
	AddLibrary({ LibGlslang.."/Source/" }, LibGlslang..binaryDir, "Glslang"..libConfig)
end

function AddGainputLibrary()
	AddLibrary(LibGainput.."/Source/lib/include", LibGainput..binaryDir, "Gainput"..libConfig)
end

function AddSPIRVToolsLibrary()
	AddLibrary({ LibSPIRVTools.."/Source/source", LibSPIRVTools.."/Source/include" }, LibSPIRVTools..binaryDir, "SPIRV-Tools"..libConfig)
end

function AddEASTLLibrary()
	AddLibrary({ LibEASTL.."/Source/include", LibEASTL.."/Source/test/packages/EAStdC/include", LibEASTL.."/Source/test/packages/EAAssert/include", LibEASTL.."/Source/test/packages/EABase/include/Common" },
	LibEASTL..binaryDir, "EASTL"..libConfig)
end

function AddQtLibrary()
	
	local qt = premake.extensions.qt
	QT_PATH = LibQt

	qtpath (QT_PATH)

	qtgenerateddir(srcQt.."/Generated")
	qtprefix("Qt5")
	configuration "Debug"
		qtsuffix("d")
	configuration {}
	qt.enable() -- TODO Enable only in appropriate (non-master) configuration

	qtmodules { "core", "gui", "widgets" }

end

-- Keep in mind many platforms can have different OSs
-- LinuxVulkan
-- AndroidVulkan
-- WinVulkan
-- etc.

-- Note on precompiled header files
-- On Visual Studio, the header file needs to be the exact string as it appears in your include, 
-- e.g. if your cpp says #include "Foo_pch.h" then pchheader("Foo_pch.h")
-- However, the pchsource file has to be the exact path.

workspace "Corsair Engine"
	configurations { "Debug", "Release" }
	platforms { VulkanWin64, D3D12, VulkanOSX }
	location (Workspace)
	preferredtoolarchitecture("x86_64") -- Prefer this toolset on MSVC as it can handle more memory for multiprocessor compiles
	toolset("msc") -- Use default VS toolset TODO do this platform specific
	--toolset("msc-llvm") -- Use for Clang on VS
	warnings("extra")
	startproject(ProjectCorsairEngine)
	language("C++")
	cppdialect("C++17")
	rtti("off")
	exceptionhandling ("off") -- Don't enable this setting
	
	-- For best configuration, see https://blogs.msdn.microsoft.com/vcblog/2017/07/13/precompiled-header-pch-issues-and-recommendations/
	
	flags { "fatalcompilewarnings" }
	vectorextensions("sse4.1")
	
	filter("toolset:msc")
		flags
		{
			"multiprocessorcompile", -- /MP
		}
		
		buildoptions
		{
			"/experimental:module"
		}
		
	filter("toolset:msc-llvm")
		buildoptions { "-Wno-reorder -msse4.1" }
		
	filter("system:windows")
		defines
		{
			-- Define this system-wide so we have no more unexpected surprises
			"NOMINMAX", 
			"WIN32_LEAN_AND_MEAN", 
			"VC_EXTRALEAN"
		}
		
	filter{}
	
	includedirs	{ src }

	platformInfo = {}
	
	filter { "platforms:"..VulkanWin64 }
		system("windows")
		architecture("x64")
		defines { "VULKAN_API", "VK_USE_PLATFORM_WIN32_KHR", "PC_TARGET" }
		platformInfo[VulkanWin64] = { "vulkan", "vk" }
		
	filter { "platforms:"..D3D12 }
		system("windows")
		architecture "x64"
		defines { "D3D12_API" }
		platformInfo[D3D12] = { "d3d12", "d3d12" }
		
	filter { "platforms:"..VulkanOSX }
		system("macosx")
		architecture "x64"		
		defines { "VULKAN_API", "VK_USE_PLATFORM_MACOS_MVK", "MAC_TARGET" }
		includedirs	{ LibVulkan.."/Source/include" }
		platformInfo[VulkanOSX] = { "vulkan", "vk" }
		
	--filter { "platforms:"..VulkanAndroid }
		--system "android"
		--architecture "x64"
		--defines { "VULKAN_API", "VK_USE_PLATFORM_ANDROID_KHR", "ANDROID_TARGET" }
		
	--filter { "platforms:"..VulkanLinux }
		--system "linux"
		--architecture "x64"
		--defines { "VULKAN_API", "VK_USE_PLATFORM_XCB_KHR", "LINUX_TARGET" }
		
	--filter { "platforms:"..VulkanIOS }
		--system "ios"
		--architecture "x64"
		--defines { "VULKAN_API", "VK_USE_PLATFORM_IOS_MVK", "IOS_TARGET" }
		
	--filter { "platforms:"..VulkanSwitch }
		--system "Linux"
		--architecture "x64"
		--defines { "VULKAN_API", "VK_USE_PLATFORM_VI_NN", "SWITCH_TARGET" }
		
	filter{}
	
	AddEASTLLibrary()
			
	-- Setup include directories
			
	includedirs	{
					LibHlslpp.."/Source/include",
					LibStb.."/Source",
					LibxxHash.."/Source",
					LibArgh.."/Source/",
					LibHalf.."/Source",
					LibDdspp.."/Source",
					shaderGenDir
				}
	
	-- Set up defines
	
	-- TODO Make this command line based or something better basically
	defines ("IN_SRC_PATH=\"".. path.getabsolute(src).."/\"")

   -- All defines from different libraries that need exceptions disabled go here.
    defines
	{ 
		"_HAS_EXCEPTIONS=0",
		
		"HALF_ENABLE_CPP11_NOEXCEPT=1",
		"HALF_ENABLE_CPP11_CONSTEXPR=1",
		"HALF_ENABLE_CPP11_TYPE_TRAITS=1",
		"HALF_ENABLE_CPP11_CSTDINT=1",
		"HALF_ENABLE_CPP11_USER_LITERALS=1",
		"HALF_ENABLE_CPP11_STATIC_ASSERT=1",
		"HALF_ENABLE_CPP11_CMATH=1",
	}

	filter("system:windows")
		--systemversion(os.winSdkVersion())
		entrypoint "mainCRTStartup"
		defines { "_CRT_SECURE_NO_WARNINGS" }
		
	filter{}
	
	-- This allows the PATH macros to work
	defines { "GRAPHICS_API_DIR=%{platformInfo[cfg.platform][1]}", "GRAPHICS_API_SUFFIX=%{platformInfo[cfg.platform][2]}" }
	
	configuration "Debug"
		defines { "DEBUG" }
		optimize("off")
		--symbols("on")
		symbols("fastlink")
		--inlining("auto")
		runtime("debug")

	configuration "Release"
		defines { "NDEBUG" }
		optimize("full")
		symbols("on")
		inlining("auto")
		flags { "linktimeoptimization" }
		runtime("release")
	
-- Project definitions

project (ProjectCorsairEngine)
	kind("WindowedApp")
	files	
	{	
		src.."/*.h", src.."/*.cpp",
		srcQt.."/**.ui", srcQt.."/**.cpp", srcQt.."/**.h", 
	}
	
	-- TODO HACK Copied from CrRendering
	-- The files below are autogenerated, but are manually specified so they take part in the build process
	files { shaderGenDir.."/ShaderResources.h" }
	
	links { ProjectCrCore, ProjectCrRendering }
	
	AddAssimpLibrary()
	AddGainputLibrary()
	
	AddVulkanLibrary() -- TODO remove
	AddSpirvCrossLibrary() -- TODO remove
	AddGlslangLibrary() -- TODO remove	
	AddQtLibrary() -- TODO move
	
	filter("system:windows")
		AddXinputLibrary() -- Needed for gainput on Windows
		
	filter{}
	
	-- TODO This is Windows-specific. If we need to do this on the Mac update the function
	function CopyToTargetDir(sourceFileDir)
		local name = path.getname(sourceFileDir)
		-- The asterisk at the end is essential, although I don't understand why
		return "{copy} \""..sourceFileDir.."\" \""..MainExecutableFolder..name.."*\""
	end
	
	postbuildcommands 
	{
		--CopyToTargetDir(path.getabsolute(LibAssimp).."/assimp-vc140-mt.dll"),
	}
	
group("Rendering")

srcRendering = src.."/Rendering"
srcShaders = srcRendering.."/Shaders"

project(ProjectCrRendering)	
	kind("StaticLib")
	pchheader("CrRendering_pch.h")
	pchsource(srcRendering.."/CrRendering_pch.cpp")
	dependson { ProjectShaders } -- This depends on the shaders. Shaders in turn depends on the shader compiler
	
	files	{
				srcRendering.."/*.h", srcRendering.."/*.cpp",

				-- The files below are autogenerated, but are manually specified so they take part in the build process
				shaderGenDir.."/ShaderResources.h", shaderGenDir.."/ShaderResources.cpp"
			}
			
	includedirs	{ srcRendering }

	AddAssimpLibrary()
	AddSpirvCrossLibrary()
	AddGlslangLibrary()
	AddGainputLibrary() -- TODO remove
	
	filter { "platforms:"..VulkanWin64 }
		files { srcRendering.."/vulkan/*" }		
		AddVulkanLibrary()
		
	filter { "platforms:"..VulkanOSX }
		files	{ srcRendering.."/vulkan/*" }
		includedirs	{ srcRendering.."/vulkan/" }
		
	filter { "platforms:"..D3D12 }
		files	{ srcRendering.."/d3d12/*" }
		includedirs	{ srcRendering.."/d3d12/" }
	
	filter{}

project(ProjectShaders)
	kind("StaticLib")
	files { srcShaders.."/**.glsl", srcShaders.."/**.spv", srcShaders.."/**.hlsl" }
	dependson { ProjectShaderCompiler }

	local shaderGenDirAbsolute = path.getabsolute(shaderGenDir)
	local metadataFile = path.getabsolute(srcShaders).."/metadata.hlsl"
	local outputFile = shaderGenDirAbsolute.."/ShaderResources"
	local shaderGenCommandLine = "\"%{cfg.buildtarget.directory}"..ProjectShaderCompiler..".exe\" -input \""..metadataFile.."\" -metadata \""..outputFile.."\""

	buildcommands
	{
		"{mkdir} ".."\""..shaderGenDirAbsolute.."\"", 	-- Create the output folder
		"{echo} "..shaderGenCommandLine,				-- Echo the command line
		shaderGenCommandLine,							-- Run
	}
	
	buildoutputs
	{
		outputFile..".uptodate"
	}
	
	local shaderFiles = os.matchfiles(srcShaders.."/*.hlsl")
	
	buildinputs
	{
		shaderFiles
	}
	
	buildmessage("")

	-- Let Visual Studio know we don't want to compile shaders through the built-in compiler
	-- In the future built in shaders could be compiled this way
	filter { "files:**.hlsl" }
		buildaction("none")
		
	filter{}

project(ProjectShaderCompiler)
	kind("ConsoleApp")
	--pchheader("CrShaderCompiler_pch.h")
	--pchsource(srcShaders.."/CrShaderCompiler_pch.cpp")
	files { srcShaders.."/*.cpp", srcShaders.."/*.h" }
	
	AddSpirvCrossLibrary()
	AddGlslangLibrary()

group("Math")
	
project(ProjectCrMath)
	kind("StaticLib")
	--pchheader("CrMath_pch.h")
	--pchsource(srcMath.."/CrMath_pch.cpp")
	files	{ srcMath.."/**.h", srcMath.."/**.cpp", LibHlslpp.."/Source/include/**.h", LibHlslpp.."/Source/include/**.natvis" }
	includedirs	{ srcMath, LibHlslpp.."/Source/include/" }	

group("Resource")

srcResource = src.."/Resource"

project(ProjectCrResource)
	kind("StaticLib")
	--pchheader("CrResource_pch.h")
	--pchsource(srcResource.."/CrResource_pch.cpp")
	files{ 	srcResource.."/**.h", srcResource.."/**.cpp" }

srcImport = src.."/Import"
	
project(ProjectCrImport)
	kind("StaticLib")
	--pchheader("CrImport_pch.h")
	--pchsource(srcImport.."/CrImport_pch.cpp")
	files{ 	srcImport.."/**.h", srcImport.."/**.cpp" }
	
	AddAssimpLibrary()
	
group("Core")

srcCore = src.."/Core"

project(ProjectCrCore)
	kind("StaticLib")
	--pchheader("CrCore_pch.h")
	--pchsource(srcCore.."/CrCore_pch.cpp")
	files { srcCore.."/**.h", srcCore.."/**.cpp", LibxxHash.."/Source/xxhash.h" }
	includedirs	{ srcCore }

srcDebug = src.."/Debug"
	
project(ProjectCrDebug)
	kind("StaticLib")
	files	{ srcDebug.."/**.h", srcDebug.."/**.cpp" }
	includedirs	{ srcDebug }

-- Create a dummy project for all the natvis files to be in, instead of the individual project folders
project("Natvis")
	kind("StaticLib")
	files{ LibEASTL.."/Source/doc/**.natvis" }

--[[group(".Solution Generation")

project("Generate Solution")
	kind("StaticLib")
	files{ "*.lua", "*.bat", "*.command" }

	local rootPathAbsolute = path.getabsolute("")
	local generateSolutionCommandLine = "{chdir} \""..rootPathAbsolute.."\""

	postbuildcommands
	{
		generateSolutionCommandLine,						-- Run
		"\"Visual Studio 2017.bat\"",
	}]]--