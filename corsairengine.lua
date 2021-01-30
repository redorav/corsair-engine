-- Directories
DependenciesDirectory   = 'Dependencies'
SourceDirectory         = 'Source'
ToolsDirectory          = 'Tools'
ShaderCompilerDirectory = ToolsDirectory..'/Shader Compiler'

MathDirectory = SourceDirectory..'/Math'

-- Platforms
VulkanWin64 = 'Vulkan Win64'
VulkanARM   = 'Vulkan ARM'
D3D12       = 'D3D12 Win64'
VulkanOSX   = 'Vulkan OSX'

-- Make this configuration-dependent
WorkspaceDirectory = 'Workspace/'.._ACTION

-- Project Names
ProjectCorsairEngine    = 'Corsair Engine'
ProjectCrMath           = 'CrMath'
ProjectCrRendering      = 'CrRendering'
ProjectShaderCompiler   = 'CrShaderCompiler'
ProjectShaders          = 'CrShaders'
ProjectCrCore           = 'CrCore'
ProjectCrDebug          = 'CrDebug'
ProjectCrImage          = 'CrImage'

-- Library Directories
LibVulkan       = DependenciesDirectory..'/vulkan'
LibEASTL        = DependenciesDirectory..'/eastl'
LibGlslang      = DependenciesDirectory..'/glslang'
LibGainput      = DependenciesDirectory..'/gainput'
LibSPIRVCross   = DependenciesDirectory..'/spirv-cross'
LibHlslpp       = DependenciesDirectory..'/hlslpp'
LibStb          = DependenciesDirectory..'/stb'
LibxxHash       = DependenciesDirectory..'/xxHash'
LibAssimp       = DependenciesDirectory..'/assimp'
LibArgh         = DependenciesDirectory..'/argh'
LibHalf         = DependenciesDirectory..'/half'
LibDdspp        = DependenciesDirectory..'/ddspp'
LibSDL2         = DependenciesDirectory..'/sdl2'
LibSPIRVReflect = DependenciesDirectory..'/spirv-reflect'
LibImGui      = DependenciesDirectory..'/imgui'

LibConfig = '.'.._ACTION..'.%{cfg.buildcfg:lower()}' -- Careful, the names are debug and release but this depends on this project's naming as well

-- Generated Code Directories
GeneratedShadersDirectory = WorkspaceDirectory..'/GeneratedShaders'
GeneratedCodeDirectory = WorkspaceDirectory..'/GeneratedCode'

BinaryDirectory = '/Binaries/'

ShaderCompilerAbsolutePath = path.getabsolute(ShaderCompilerDirectory)..'/'..ProjectShaderCompiler..'.exe'

-- todo put in common
-- Adds a library to a project
function AddLibrary(includeDirs, libDirs, libNames)

	includedirs(includeDirs)
	libdirs (libDirs)
	links(libNames)

end

function AddAssimpLibrary()
	AddLibrary(LibAssimp..'/Source/include', LibAssimp..BinaryDirectory, 'Assimp'..LibConfig)
end

function AddVulkanLibrary()
	AddLibrary(LibVulkan..'/Source/include', LibVulkan..BinaryDirectory, 'vulkan-1')
end

function AddXinputLibrary()
	AddLibrary('', '', 'Xinput9_1_0')
end

function AddGlslangLibrary()
	AddLibrary({ LibGlslang..'/Source/' }, LibGlslang..BinaryDirectory, 'Glslang'..LibConfig)
end

function AddGainputLibrary()
	AddLibrary(LibGainput..'/Source/lib/include', LibGainput..BinaryDirectory, 'Gainput'..LibConfig)
end

function AddEASTLLibrary()
	AddLibrary({ LibEASTL..'/Source/include', LibEASTL..'/Source/test/packages/EAStdC/include', LibEASTL..'/Source/test/packages/EAAssert/include', LibEASTL..'/Source/test/packages/EABase/include/Common' },
	LibEASTL..BinaryDirectory, 'EASTL'..LibConfig)
end

function AddHlslppLibrary()
	defines { 'HLSLPP_FEATURE_TRANSFORM' }
end

function AddSDL2Library()
	AddLibrary(LibSDL2..'/Source/include', LibSDL2..BinaryDirectory, 'SDL2')
	defines { 'SDL_MAIN_HANDLED' }
end

function AddImGuiLibrary()
	AddLibrary(LibImGui..'/Source/', LibImGui..BinaryDirectory, 'ImGui'..LibConfig)
end

function AddSpirvReflectLibrary()
	AddLibrary(LibSPIRVReflect..'/Source', LibSPIRVReflect..BinaryDirectory, 'SPIRV-Reflect'..LibConfig)
end

function ExcludePlatformSpecificCode(rootPath)

	excludes { rootPath..'**/platform/**' }

end

function CopyFileCommand(filePath, destinationPath)
	return '{copyfile} "'..filePath..'" "'..destinationPath..'"'
end

-- Keep in mind many platforms can have different OSs
-- LinuxVulkan
-- AndroidVulkan
-- WinVulkan
-- etc.

-- Note on precompiled header files
-- On Visual Studio, the header file needs to be the exact string as it appears in your include,
-- e.g. if your cpp says #include 'Foo_pch.h' then pchheader('Foo_pch.h')
-- However, the pchsource file has to be the exact path.

workspace 'Corsair Engine'
	configurations { 'Debug', 'Release' }
	platforms { VulkanWin64, D3D12, VulkanOSX }
	location (WorkspaceDirectory)
	preferredtoolarchitecture('x86_64') -- Prefer this toolset on MSVC as it can handle more memory for multiprocessor compiles
	toolset('msc') -- Use default VS toolset TODO do this platform specific
	--toolset('msc-llvm') -- Use for Clang on VS
	warnings('extra')
	startproject(ProjectCorsairEngine)
	language('C++')
	cppdialect('C++17')
	rtti('off')
	exceptionhandling ('off') -- Don't enable this setting
	
	-- For best configuration, see https://blogs.msdn.microsoft.com/vcblog/2017/07/13/precompiled-header-pch-issues-and-recommendations/
	
	flags { 'fatalcompilewarnings' }
	vectorextensions('sse4.1')
	
	filter('toolset:msc')
		flags
		{
			'multiprocessorcompile', -- /MP
		}
		
		buildoptions
		{
			'/experimental:module'
		}
		
	filter('toolset:msc-llvm')
		buildoptions { '-Wno-reorder -msse4.1' }
		
	filter('system:windows')
		defines
		{
			-- Define this system-wide so we have no more unexpected surprises
			'NOMINMAX', 
			'WIN32_LEAN_AND_MEAN', 
			'VC_EXTRALEAN'
		}
		
	filter{}
	
	includedirs	{ SourceDirectory }

	filter { 'platforms:'..VulkanWin64 }
		system('windows')
		architecture('x64')
		defines { 'VULKAN_API', 'VK_USE_PLATFORM_WIN32_KHR', 'WINDOWS_TARGET' }
		
	filter { 'platforms:'..D3D12 }
		system('windows')
		architecture 'x64'
		defines { 'D3D12_API' }
		
	filter { 'platforms:'..VulkanOSX }
		system('macosx')
		architecture 'x64'		
		defines { 'VULKAN_API', 'VK_USE_PLATFORM_MACOS_MVK', 'MAC_TARGET' }
		
	--filter { 'platforms:'..VulkanAndroid }
		--system 'android'
		--architecture 'x64'
		--defines { 'VULKAN_API', 'VK_USE_PLATFORM_ANDROID_KHR', 'ANDROID_TARGET' }
		
	--filter { 'platforms:'..VulkanLinux }
		--system 'linux'
		--architecture 'x64'
		--defines { 'VULKAN_API', 'VK_USE_PLATFORM_XCB_KHR', 'LINUX_TARGET' }
		
	--filter { 'platforms:'..VulkanIOS }
		--system 'ios'
		--architecture 'x64'
		--defines { 'VULKAN_API', 'VK_USE_PLATFORM_IOS_MVK', 'IOS_TARGET' }
		
	--filter { 'platforms:'..VulkanSwitch }
		--system 'Linux'
		--architecture 'x64'
		--defines { 'VULKAN_API', 'VK_USE_PLATFORM_VI_NN', 'SWITCH_TARGET' }
		
	filter{}
	
	AddEASTLLibrary()
	AddHlslppLibrary()
	AddImGuiLibrary()

	-- Setup include directories
			
	includedirs
	{
		LibHlslpp..'/Source/include',
		LibStb..'/Source',
		LibxxHash..'/Source',
		LibArgh..'/Source/',
		LibHalf..'/Source',
		LibDdspp..'/Source',
		GeneratedShadersDirectory,
		GeneratedCodeDirectory
	}
	
	-- Set up defines
	
	-- TODO Make this command line based or something better basically
	defines ('IN_SRC_PATH="'.. path.getabsolute(SourceDirectory)..'/"')

   -- All defines from different libraries that need exceptions disabled go here.
    defines
	{ 
		'_HAS_EXCEPTIONS=0',
		
		'HALF_ENABLE_CPP11_NOEXCEPT=1',
		'HALF_ENABLE_CPP11_CONSTEXPR=1',
		'HALF_ENABLE_CPP11_TYPE_TRAITS=1',
		'HALF_ENABLE_CPP11_CSTDINT=1',
		'HALF_ENABLE_CPP11_USER_LITERALS=1',
		'HALF_ENABLE_CPP11_STATIC_ASSERT=1',
		'HALF_ENABLE_CPP11_CMATH=1',
	}
	
	-- Generate the global paths file
	globalVariableHeader = io.open(path.getabsolute(GeneratedCodeDirectory)..'/GlobalVariables.h', 'wb');
	globalVariableHeader:write('static const char* ShaderCompilerPath = "'..ShaderCompilerAbsolutePath..'";');
	globalVariableHeader:close();

	filter('system:windows')
		--systemversion(os.winSdkVersion())
		entrypoint 'mainCRTStartup'
		defines { '_CRT_SECURE_NO_WARNINGS' }
		
	filter{}
	
	configuration 'Debug'
		defines { 'DEBUG' }
		optimize('off')
		--symbols('on')
		symbols('fastlink')
		--inlining('auto')
		runtime('debug')

	configuration 'Release'
		defines { 'NDEBUG' }
		optimize('full')
		symbols('on')
		inlining('auto')
		flags { 'linktimeoptimization' }
		runtime('release')
	
-- Project definitions

project (ProjectCorsairEngine)
	kind('WindowedApp')
	files	
	{	
		SourceDirectory..'/*.h', SourceDirectory..'/*.cpp'
	}

	links { ProjectCrCore, ProjectCrRendering }
	
	AddAssimpLibrary()
	AddGainputLibrary()
	AddSDL2Library()

	-- Copy necessary files or DLLs
	postbuildcommands
	{
		CopyFileCommand(path.getabsolute(LibSDL2)..'/Binaries/SDL2.dll', '%{cfg.buildtarget.directory}')
	}
	
	filter('system:windows')
		AddXinputLibrary() -- Needed for gainput on Windows

	filter{}

group('Rendering')

SourceRenderingDirectory = SourceDirectory..'/Rendering'
SourceShadersDirectory = SourceRenderingDirectory..'/Shaders'
SourceShaderCompilerDirectory = SourceRenderingDirectory..'/ShaderCompiler'

project(ProjectCrRendering)	
	kind('StaticLib')
	pchheader('CrRendering_pch.h')
	pchsource(SourceRenderingDirectory..'/CrRendering_pch.cpp')
	dependson { ProjectShaders } -- This depends on the shaders. Shaders in turn depends on the shader compiler
	
	files	
	{
		SourceRenderingDirectory..'/*',
		SourceRenderingDirectory..'/UI/*',

		-- The files below are autogenerated, but are manually specified so they take part in the build process
		GeneratedShadersDirectory..'/ShaderResources.h',
		GeneratedShadersDirectory..'/ShaderResources.cpp'
	}
	
	links { ProjectCrImage } -- TODO Delete

	AddAssimpLibrary()
	AddSpirvReflectLibrary()
	AddGainputLibrary() -- TODO Remove
	
	filter { 'platforms:'..VulkanWin64 }
		files { SourceRenderingDirectory..'/vulkan/*' }
		AddVulkanLibrary()
		
	filter { 'platforms:'..VulkanOSX }
		files	{ SourceRenderingDirectory..'/vulkan/*' }
		
	filter { 'platforms:'..D3D12 }
		files	{ SourceRenderingDirectory..'/d3d12/*' }
	
	filter{}

project(ProjectShaders)
	kind('StaticLib')
	files { SourceShadersDirectory..'/**' }
	dependson { ProjectShaderCompiler }

	local GeneratedShadersDirectoryAbsolute = path.getabsolute(GeneratedShadersDirectory)
	local metadataFile = path.getabsolute(SourceShadersDirectory)..'/metadata.hlsl'
	local outputFile = GeneratedShadersDirectoryAbsolute..'/ShaderResources'
	local shaderGenCommandLine = 
	'"'..ShaderCompilerAbsolutePath..'" '..
	'-metadata ' ..
	'-input "'..metadataFile..'" ' ..
	'-output "'..outputFile..'" ' ..
	'-entrypoint metadata'

	buildcommands
	{
		'{mkdir} '..'"'..GeneratedShadersDirectoryAbsolute..'"', 	-- Create the output folder
		'{echo} '..shaderGenCommandLine,				-- Echo the command line
		shaderGenCommandLine,							-- Run
	}
	
	buildoutputs
	{
		outputFile..'.uptodate'
	}
	
	local shaderFiles = os.matchfiles(SourceShadersDirectory..'/*.hlsl')
	
	buildinputs
	{
		shaderFiles,
		ShaderCompilerAbsolutePath
	}
	
	buildmessage('')

	-- Let Visual Studio know we don't want to compile shaders through the built-in compiler
	-- In the future built in shaders could be compiled this way
	filter { 'files:**.hlsl' }
		buildaction('none')
		
	filter{}

project(ProjectShaderCompiler)
	kind('ConsoleApp')
	files
	{
		SourceShaderCompilerDirectory..'/**',
	}
	
	links { ProjectCrCore }

	AddSpirvReflectLibrary()
	AddGlslangLibrary()

	-- Copy the shader compiler into a known directory
	postbuildcommands
	{
		'{mkdir} \"'..path.getabsolute(ShaderCompilerDirectory).."\"",
		CopyFileCommand('%{cfg.buildtarget.abspath}', path.getabsolute(ShaderCompilerDirectory))
	}

group('Math')
	
project(ProjectCrMath)
	kind('StaticLib')
	files
	{
		MathDirectory..'/**',
		LibHlslpp..'/Source/include/**'
	}

	includedirs
	{
		MathDirectory
	}

group('Image')

SourceImageDirectory= SourceDirectory..'/Image'

project(ProjectCrImage)
	kind('StaticLib')
	files{ 	SourceImageDirectory..'/**' }
	
group('Core')

SourceCoreDirectory = SourceDirectory..'/Core'

project(ProjectCrCore)
	kind('StaticLib')
	
	files
	{
		SourceCoreDirectory..'/**',
		LibxxHash..'/Source/xxhash.h'
	}

	ExcludePlatformSpecificCode(SourceCoreDirectory)
	
	filter { 'system:windows' }
		files { SourceCoreDirectory..'/**/windows/**' }

	filter { 'system:linux' }
		files { SourceCoreDirectory..'/**/ansi/**' }

	filter {}

SourceDebugDirectory = SourceDirectory..'/Debug'
	
project(ProjectCrDebug)
	kind('StaticLib')
	files
	{
		SourceDebugDirectory..'/**'
	}

-- Create a dummy project for all the natvis files to be in, instead of the individual project folders
project('Natvis')
	kind('StaticLib')
	files{ LibEASTL..'/Source/doc/**.natvis' }

--[[group('.Solution Generation')

project('Generate Solution')
	kind('StaticLib')
	files{ '*.lua', '*.bat', '*.command' }

	local rootPathAbsolute = path.getabsolute('')
	local generateSolutionCommandLine = '{chdir} "'..rootPathAbsolute..'"'

	postbuildcommands
	{
		generateSolutionCommandLine,						-- Run
		'"Visual Studio 2017.bat"',
	}]]--