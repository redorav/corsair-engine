require("premake/corsairengine/dependencies")

-- Directories
SourceDirectory         = 'Source'
ToolsDirectory          = 'Tools'
ShaderCompilerDirectory = ToolsDirectory..'/Shader Compiler'
MathDirectory = SourceDirectory..'/Math'

-- Platforms
DesktopWin64 = 'Desktop Win64'
VulkanOSX    = 'Vulkan OSX'

-- Make this configuration-dependent
WorkspaceDirectory = 'Workspace/'.._ACTION

-- Project Names
ProjectCorsairEngine    = 'Corsair Engine'
ProjectCrMath           = 'CrMath'
ProjectCrRendering      = 'CrRendering'
ProjectShaderCompiler   = 'CrShaderCompiler'
ProjectShaders          = 'CrShaders'
ProjectCrCore           = 'CrCore'
ProjectCrInput          = 'CrInput'
ProjectCrDebug          = 'CrDebug'
ProjectCrImage          = 'CrImage'
ProjectCrModel          = 'CrModel'

-- Generated Code Directories
GeneratedShadersDirectory = WorkspaceDirectory..'/GeneratedShaders'
GeneratedCodeDirectory = WorkspaceDirectory..'/GeneratedCode'

ShaderCompilerAbsolutePath = path.getabsolute(ShaderCompilerDirectory)..'/'..ProjectShaderCompiler..'.exe'

function ExcludePlatformSpecificCode(rootPath)
	excludes { rootPath..'**/platform/**' }
end

function CopyFileCommand(filePath, destinationPath)
	return '{copyfile} "'..filePath..'" "'..destinationPath..'"'
end

-- Note on precompiled header files
-- On Visual Studio, the header file needs to be the exact string as it appears in your include,
-- e.g. if your cpp says #include 'Foo_pch.h' then pchheader('Foo_pch.h')
-- However, the pchsource file has to be the exact path.

workspace 'Corsair Engine'
	configurations { 'Debug', 'Release' }
	platforms { DesktopWin64 }
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

	filter { 'platforms:'..DesktopWin64 }
		system('windows')
		architecture('x64')
		defines { 'WINDOWS_TARGET' }
		defines { 'VULKAN_API', 'VK_USE_PLATFORM_WIN32_KHR' }
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
	
	-- Global library includes. Very few things should go here, basically things
	-- that are used in every possible project like math and containers
	
	AddLibraryIncludes(ArghLibrary)
	AddLibraryIncludes(DdsppLibrary)
	AddLibraryIncludes(EASTLLibrary)
	AddLibraryIncludes(HalfLibrary)
	AddLibraryIncludes(HlslppLibrary)
	AddLibraryIncludes(xxHashLibrary)

	-- Setup include directories
	includedirs
	{
		WorkspaceDirectory,
		GeneratedCodeDirectory
	}
	
	-- Set up defines
	
	-- TODO Make this command line based or something better basically
	defines ('IN_SRC_PATH="'.. path.getabsolute(SourceDirectory)..'/"')

   -- All defines from different libraries that need exceptions disabled go here.
    defines
	{ 
		'_HAS_EXCEPTIONS=0', -- Disable STL exceptions
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

		-- we force the release runtime to be able to link against
		-- release external libraries to speed up this config
		runtime('release')

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

	links { ProjectCrCore, ProjectCrRendering, ProjectCrImage, ProjectCrInput, ProjectCrModel }
	
	-- Only executables should link to any libraries
	-- Otherwise we'll get bloated libs and slow link times
	-- Project libraries have slimmed by about ~140MB
	AddLibraryIncludes(AssimpLibrary)
	LinkLibrary(AssimpLibrary)

	AddLibraryIncludes(SDL2Library)
	LinkLibrary(SDL2Library)
	
	LinkLibrary(EASTLLibrary)
	LinkLibrary(ImguiLibrary)

	-- todo platform filters
	LinkLibrary(VulkanLibrary)
	LinkLibrary(SPIRVReflectLibrary)
	LinkLibrary(D3D12Library)

	-- Copy necessary files or DLLs
	postbuildcommands
	{
		-- TODO Make sure this copy command 
		CopyFileCommand(path.getabsolute(SDL2Library.dlls), '%{cfg.buildtarget.directory}')
	}
	
	filter{}

group('Rendering')

SourceRenderingDirectory = SourceDirectory..'/Rendering'
SourceShadersDirectory = SourceRenderingDirectory..'/Shaders'
SourceShaderCompilerDirectory = SourceRenderingDirectory..'/ShaderCompiler'
ShaderMetadataFilename = "ShaderMetadata"

project(ProjectCrRendering)
	kind('StaticLib')
	pchheader('CrRendering_pch.h')
	pchsource(SourceRenderingDirectory..'/CrRendering_pch.cpp')
	dependson { ProjectShaders } -- This depends on the shaders. Shaders in turn depends on the shader compiler

	local ShaderMetadataHeader = GeneratedShadersDirectory..'/'..ShaderMetadataFilename..'.h'
	local ShaderMetadataCpp = GeneratedShadersDirectory..'/'..ShaderMetadataFilename..'.cpp'

	files
	{
		SourceRenderingDirectory..'/*',
		SourceRenderingDirectory..'/UI/*',

		-- The files below are autogenerated, but are manually specified so they take part in the build process
		ShaderMetadataHeader,
		ShaderMetadataCpp
	}
	
	AddLibraryIncludes(AssimpLibrary)
	AddLibraryIncludes(SPIRVReflectLibrary)
	AddLibraryIncludes(ImguiLibrary)
	
	filter { 'platforms:'..DesktopWin64 }
		files { SourceRenderingDirectory..'/vulkan/*' }
		files { SourceRenderingDirectory..'/d3d12/*' }
		AddLibraryIncludes(VulkanLibrary)
		
	filter { 'platforms:'..VulkanOSX }
		files { SourceRenderingDirectory..'/vulkan/*' }
	
	filter{}

project(ProjectShaders)
	kind('StaticLib')
	files { SourceShadersDirectory..'/**' }
	dependson { ProjectShaderCompiler }

	local GeneratedShadersDirectoryAbsolute = path.getabsolute(GeneratedShadersDirectory)
	local metadataFile = path.getabsolute(SourceShadersDirectory)..'/metadata.hlsl'
	local outputFile = GeneratedShadersDirectoryAbsolute..'/'..ShaderMetadataFilename
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

	AddLibraryIncludes(SPIRVReflectLibrary)
	LinkLibrary(SPIRVReflectLibrary)

	AddLibraryIncludes(GlslangLibrary)
	LinkLibrary(GlslangLibrary)

	-- Copy the shader compiler into a known directory
	postbuildcommands
	{
		'{mkdir} \"'..path.getabsolute(ShaderCompilerDirectory).."\"",
		CopyFileCommand('%{cfg.buildtarget.abspath}', path.getabsolute(ShaderCompilerDirectory))
	}

group('Math')
	
project(ProjectCrMath)
	kind('StaticLib')
	files { MathDirectory..'/**' }
	includedirs { MathDirectory }
	
	AddLibraryFiles(HlslppLibrary)

group('Image')

SourceImageDirectory = SourceDirectory..'/Image'

project(ProjectCrImage)
	kind('StaticLib')
	files{ 	SourceImageDirectory..'/**' }

	AddLibraryIncludes(StbLibrary)
	
group('Input')

SourceInputDirectory = SourceDirectory..'/Input'

project(ProjectCrInput)
	kind('StaticLib')

	files { SourceInputDirectory..'/**' }
	
	AddLibraryIncludes(SDL2Library)

	filter {}
	
group('Model')

SourceModelDirectory = SourceDirectory..'/Model'

project(ProjectCrModel)
	kind('StaticLib')
	dependson { ProjectShaders }
	files { SourceModelDirectory..'/**' }

	AddLibraryIncludes(TinyGLTFLibrary)
	AddLibraryIncludes(AssimpLibrary)

group('Core')

SourceCoreDirectory = SourceDirectory..'/Core'

project(ProjectCrCore)
	kind('StaticLib')
	
	files
	{
		SourceCoreDirectory..'/**',
		xxHashLibrary.includeDirs..'/xxhash.h'
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