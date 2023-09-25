require("premake/corsairengine/dependencies")

-- Directories
SourceDirectory          = 'Source'
ToolsDirectory           = 'Tools'
MathDirectory            = SourceDirectory..'/Math'
SourceUnitTestsDirectory = SourceDirectory..'/UnitTests'
SourceEditorDirectory    = SourceDirectory..'/Editor'
SourceRenderingDirectory = SourceDirectory..'/Rendering'
SourceShadersDirectory   = SourceRenderingDirectory..'/Shaders'
WorkspaceDirectory       = 'Workspace/'.._ACTION

-- IDE Platform Names
MSVCWin64    = 'MSVC Win64'
ClangWin64   = 'Clang Win64'
VulkanOSX    = 'Vulkan OSX'

PlatformWindows = "Windows"
GraphicsApiVulkan = "Vulkan"
GraphicsApiD3D12 = "D3D12"

Platform = nil
GraphicsApis = {}

Win64PlatformFilter = "platforms:"..MSVCWin64..' or '..ClangWin64

-- Project Names
ProjectCorsairEngine    = 'Corsair Engine'
ProjectMath             = 'CrMath'
ProjectRendering        = 'CrRendering'
ProjectShaderCompiler   = 'CrShaderCompiler'
ProjectShaders          = 'CrShaders'
ProjectBuiltinShaders   = 'CrBuiltinShaders'
ProjectResource         = 'CrResource'
ProjectCore             = 'CrCore'
ProjectUnitTests        = 'CrUnitTests'
ProjectEditor           = 'CrEditor'

-- Generated Code Directories
GeneratedShadersDirectory = WorkspaceDirectory..'/GeneratedShaders'
GeneratedCodeDirectory = WorkspaceDirectory..'/GeneratedCode'

ShaderCompilerExecutableName = ProjectShaderCompiler..'.exe'

-- Path is resolved during the parsing pass. It's meant to be in the same directory as the main executable
ShaderCompilerPath = '%{cfg.buildtarget.directory}/'..ShaderCompilerExecutableName

IsVisualStudio = string.sub(_ACTION, 1, string.len('vs')) == 'vs'
VisualStudioVersion = ''
if IsVisualStudio then
	VisualStudioVersion = string.sub(_ACTION, string.len('vs') + 1)
end

-- Utility Functions

function ExcludePlatformSpecificCode(rootPath)
	excludes { rootPath..'**/platform/**' }
end

function CopyFileCommand(filePath, destinationPath)
	return '{copyfile} "'..filePath..'" "'..destinationPath..'"'
end

function MakeDirCommand(directoryPath)
	return '{mkdir} "'..directoryPath..'"'
end

-- Add warnings globally to fix serious issues that could cause incorrect
-- runtime behavior or crashes.
-- Remove warnings globally only if it makes sense to do so. For example,
-- we are interested in using a specific extension or feature.
function HandleGlobalWarnings()

	filter('toolset:msc*')

		fatalwarnings
		{
			'4263', -- member function does not override any base class virtual member function
			'4264', -- no override available for virtual member function from base 'class'; function is hidden
			'4265', -- class has virtual functions, but destructor is not virtual
			'5204', -- class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed correctly
			'4555', -- result of expression not used
		}
		
		disablewarnings
		{
			'4201' -- nonstandard extension used: nameless struct/union
		}
	
	filter {}

end

-- Note on precompiled header files
-- On Visual Studio, the header file needs to be the exact string as it appears in your include,
-- e.g. if your cpp says #include 'Foo_pch.h' then pchheader('Foo_pch.h')
-- However, the pchsource file has to be the exact path.

workspace 'Corsair Engine'
	configurations { 'Debug', 'Profiling', 'Final' }
	platforms { MSVCWin64, ClangWin64 }
	location (WorkspaceDirectory)
	preferredtoolarchitecture('x86_64') -- Prefer this toolset on MSVC as it can handle more memory for multiprocessor compiles
	warnings('extra')
	startproject(ProjectCorsairEngine)
	language('C++')
	cppdialect('C++17')
	rtti('off')
	exceptionhandling ('off') -- Don't enable this setting
	objdir ("%{wks.location}/Object")
	targetdir ("%{wks.location}/Binaries/%{cfg.platform}/%{cfg.buildcfg}")
	
	-- For best configuration, see https://blogs.msdn.microsoft.com/vcblog/2017/07/13/precompiled-header-pch-issues-and-recommendations/
	
	flags { 'fatalcompilewarnings' }
	vectorextensions('sse4.1')
	
	filter('platforms:'..MSVCWin64)
		toolset('msc')

	filter('platforms:'..ClangWin64)
		toolset('msc-clangcl')
		--toolset('msc-llvm') -- Older versions of Clang in VS

	filter('toolset:msc*')
		flags
		{
			'multiprocessorcompile', -- /MP
		}
		
		buildoptions
		{
			'/permissive-'
		}
		
	filter('system:windows')
		--systemversion(os.winSdkVersion())
		entrypoint 'mainCRTStartup'
		defines
		{
			-- Define this system-wide so we have no more unexpected surprises
			'NOMINMAX', 
			'WIN32_LEAN_AND_MEAN', 
			'VC_EXTRALEAN',
			'_CRT_SECURE_NO_WARNINGS'
		}
		
	HandleGlobalWarnings()
		
	filter {}
	
	includedirs	{ SourceDirectory }

	filter { Win64PlatformFilter }
		system('windows')
		architecture('x64')
		Platform = PlatformWindows
		GraphicsApis = { GraphicsApiVulkan, GraphicsApiD3D12 }
		defines
		{
			'WINDOWS_PLATFORM',
			'VULKAN_API', 'VK_USE_PLATFORM_WIN32_KHR',
			'D3D12_API'
		}

	filter { 'platforms:'..VulkanOSX }
		system('macosx')
		architecture 'x64'
		defines { 'VULKAN_API', 'VK_USE_PLATFORM_MACOS_MVK', 'MAC_PLATFORM' }
		
	--filter { 'platforms:'..VulkanAndroid }
		--system 'android'
		--architecture 'x64'
		--defines { 'VULKAN_API', 'VK_USE_PLATFORM_ANDROID_KHR', 'ANDROID_PLATFORM' }
		
	--filter { 'platforms:'..VulkanLinux }
		--system 'linux'
		--architecture 'x64'
		--defines { 'VULKAN_API', 'VK_USE_PLATFORM_XCB_KHR', 'LINUX_PLATFORM' }
		
	--filter { 'platforms:'..VulkanIOS }
		--system 'ios'
		--architecture 'x64'
		--defines { 'VULKAN_API', 'VK_USE_PLATFORM_IOS_MVK', 'IOS_PLATFORM' }
		
	--filter { 'platforms:'..VulkanSwitch }
		--system 'linux'
		--architecture 'x64'
		--defines { 'VULKAN_API', 'VK_USE_PLATFORM_VI_NN', 'SWITCH_PLATFORM' }
		
	filter {}
	
	-- Global library includes. Very few things should go here, basically things
	-- that are used in every possible project like math and containers
	
	AddLibraryIncludes(CRSTLLibrary)
	AddLibraryIncludes(DdsppLibrary)
	AddLibraryIncludes(EASTLLibrary)
	AddLibraryIncludes(HlslppLibrary)
	AddLibraryIncludes(xxHashLibrary)

	-- Setup global include directories
	includedirs
	{
		WorkspaceDirectory,
		GeneratedCodeDirectory
	}
	
	defines
	{ 
		'_HAS_EXCEPTIONS=0', -- Disable STL exceptions
	}
	
	-- Generate the global paths file
	globalVariableHeader = io.open(path.getabsolute(GeneratedCodeDirectory)..'/GlobalVariables.h', 'wb');
	globalVariableHeader:write('namespace GlobalPaths\n{\n');
	globalVariableHeader:write('\tstatic const char* ShaderCompilerExecutableName = "'..ShaderCompilerExecutableName..'";\n');
	globalVariableHeader:write('\tstatic const char* ShaderSourceDirectory = "'..path.getabsolute(SourceShadersDirectory)..'/";\n');
	globalVariableHeader:write('};\n');
	globalVariableHeader:close();
		
	filter{}
	
	filter { 'configurations:Debug' }
		defines { 'CR_CONFIG_DEBUG' }
		optimize('off')
		--symbols('on')
		symbols('fastlink')
		--inlining('auto')

		-- We force the release runtime to be able to link against
		-- release external libraries to speed up this config
		runtime('release')

	filter { 'configurations:Profiling or Final' }
		defines
		{
			'NDEBUG', -- Disables assert
			'EASTL_ASSERT_ENABLED=0'
		}
		optimize('speed')
		symbols('on')
		inlining('auto')
		flags { 'linktimeoptimization' }
		runtime('release')

	filter { 'configurations:Profiling' }
		defines { 'CR_CONFIG_PROFILING', }

	filter { 'configurations:Final' }
		defines { 'CR_CONFIG_FINAL', }
	
	filter { 'configurations:not Final' }
		defines { 'CR_EDITOR', }

-- Project definitions

project (ProjectCorsairEngine)
	kind('WindowedApp')
	files	
	{	
		SourceDirectory..'/*.h', SourceDirectory..'/*.cpp'
	}

	-- Libraries from other projects
	links
	{
		ProjectCore,
		ProjectRendering,
		ProjectResource,
		ProjectUnitTests,
		ProjectEditor
	}
	
	-- Only executables should link to any libraries
	-- Otherwise we'll get bloated libs and slow link times
	-- Project libraries have slimmed by about ~140MB
	AddLibraryIncludes(AssimpLibrary)
	LinkLibrary(AssimpLibrary)
	LinkLibrary(StbLibrary)

	AddLibraryIncludes(SDL2Library)
	LinkLibrary(SDL2Library)
	
	LinkLibrary(EASTLLibrary)
	
	AddLibraryIncludes(ImguiLibrary)
	LinkLibrary(ImguiLibrary)

	-- todo platform filters
	LinkLibrary(VulkanLibrary)
	LinkLibrary(D3D12Library)
	LinkLibrary(WinPixEventRuntimeLibrary)

	-- Copy necessary files or DLLs
	postbuildcommands
	{
		CopyFileCommand(path.getabsolute(SDL2Library.dlls), '%{cfg.buildtarget.directory}')
	}
	
	filter {}

group('Rendering')

SourceShaderCompilerDirectory = SourceRenderingDirectory..'/ShaderCompiler'
ShaderMetadataFilename = "ShaderMetadata"
BuiltinShadersFilename = "BuiltinShaders"

project(ProjectRendering)
	kind('StaticLib')
	pchheader('Rendering/CrRendering_pch.h')
	pchsource(SourceRenderingDirectory..'/CrRendering_pch.cpp')
	dependson { ProjectShaders } -- This depends on the shaders. Shaders in turn depends on the shader compiler
	dependson { ProjectBuiltinShaders }

	local ShaderMetadataHeader = GeneratedShadersDirectory..'/'..ShaderMetadataFilename..'.h'
	local ShaderMetadataCpp = GeneratedShadersDirectory..'/'..ShaderMetadataFilename..'.cpp'

	local BuiltinShaderHeader = GeneratedShadersDirectory..'/'..BuiltinShadersFilename..'.h'
	local BuiltinShaderCpp = GeneratedShadersDirectory..'/'..BuiltinShadersFilename..'.cpp'

	files
	{
		SourceRenderingDirectory..'/*',
		SourceRenderingDirectory..'/UI/*',
		SourceRenderingDirectory..'/FrameCapture/*',
		SourceRenderingDirectory..'/RenderWorld/*',

		-- The files below are autogenerated, but are manually specified so they take part in the build process
		ShaderMetadataHeader,
		ShaderMetadataCpp,
		BuiltinShaderHeader,
		BuiltinShaderCpp
	}
	
	AddLibraryIncludes(AgilityLibrary)
	AddLibraryIncludes(AssimpLibrary)
	AddLibraryIncludes(ImguiLibrary)
	AddLibraryIncludes(SPIRVReflectLibrary)
	AddLibraryIncludes(RenderDocLibrary)
	
	filter { Win64PlatformFilter }
		files { SourceRenderingDirectory..'/Vulkan/*' }
		files { SourceRenderingDirectory..'/D3D12/*' }
		AddLibraryIncludes(VulkanLibrary)
		AddLibraryIncludes(WinPixEventRuntimeLibrary)
		
		postbuildcommands
		{
			CopyFileCommand(path.getabsolute(WinPixEventRuntimeLibrary.dlls), '%{cfg.buildtarget.directory}'),
			MakeDirCommand('%{cfg.buildtarget.directory}/D3D12/'),
			CopyFileCommand(path.getabsolute(AgilityLibrary.dlls[1]), '%{cfg.buildtarget.directory}/D3D12/'),
			CopyFileCommand(path.getabsolute(AgilityLibrary.dlls[2]), '%{cfg.buildtarget.directory}/D3D12/'),
		}
		
	filter { 'platforms:'..VulkanOSX }
		files { SourceRenderingDirectory..'/Vulkan/*' }
	
	filter {}

------------------------------------
-- Shader metadata generation job --
------------------------------------

local GeneratedShadersDirectoryAbsolute = path.getabsolute(GeneratedShadersDirectory)

local hlslFiles = os.matchfiles(SourceShadersDirectory..'/**.hlsl')
local shadersFiles = os.matchfiles(SourceShadersDirectory..'/**.shaders')

project(ProjectShaders)
	kind('StaticLib')
	files { SourceShadersDirectory..'/**.hlsl', SourceShadersDirectory..'/**.shaders' }
	dependson { ProjectShaderCompiler }

	local metadataFile = path.getabsolute(SourceShadersDirectory)..'/Metadata.hlsl'
	local outputFile = GeneratedShadersDirectoryAbsolute..'/'..ShaderMetadataFilename
	local shaderMetadataCommandLine = 
	'"'..ShaderCompilerPath..'" '..
	'-metadata -input "'..metadataFile..'" ' ..
	'-output "'..outputFile..'" -entrypoint metadata'

	buildcommands
	{
		'{mkdir} '..'"'..GeneratedShadersDirectoryAbsolute..'"', -- Create the output folder
		'{echo} '..shaderMetadataCommandLine, -- Echo the command line
		shaderMetadataCommandLine, -- Run
	}
	
	buildinputs { hlslFiles, shadersFiles, ShaderCompilerPath }
	
	buildoutputs { outputFile..'.uptodate' }
	
	buildmessage('')

	-- Let Visual Studio know we don't want to compile shaders through the built-in compiler
	-- In the future built in shaders could be compiled this way
	filter { 'files:**.hlsl' }
		buildaction('none')
		
	filter {}

-----------------------------------
-- Builtin shader generation job --
-----------------------------------

project(ProjectBuiltinShaders)
	kind('StaticLib')
	dependson { ProjectShaderCompiler }

	local outputFile = GeneratedShadersDirectoryAbsolute..'/BuiltinShaders'
	local builtinShaderCommandLine =
	'"'..ShaderCompilerPath..'" '..
	'-builtin -builtin-headers '..
	'-input "'..path.getabsolute(SourceShadersDirectory)..'" '..
	'-output "'..outputFile..'" '..
	'-platform '..Platform:lower()
	
	for i, graphicsApi in ipairs(GraphicsApis) do
		builtinShaderCommandLine = builtinShaderCommandLine..' -graphicsapi '..graphicsApi:lower()
	end

	buildcommands
	{
		'{echo} Compiling builtin shaders',
		'{echo} '..builtinShaderCommandLine,
		builtinShaderCommandLine,
	}

	buildinputs { hlslFiles, shadersFiles, ShaderCompilerPath }

	buildoutputs { outputFile..'.uptodate' }
	
	buildmessage('')
		
	filter {}

project(ProjectShaderCompiler)
	kind('ConsoleApp')
	files { SourceShaderCompilerDirectory..'/**' }
	
	pchheader('Rendering/ShaderCompiler/CrShaderCompiler_pch.h')
	pchsource(SourceShaderCompilerDirectory..'/CrShaderCompiler_pch.cpp')
	
	links { ProjectCore }

	AddLibraryIncludes(SPIRVReflectLibrary)
	LinkLibrary(SPIRVReflectLibrary)
	
	AddLibraryIncludes(DxcLibrary)
	LinkLibrary(DxcLibrary)

	AddLibraryIncludes(GlslangLibrary)
	LinkLibrary(GlslangLibrary)
	
	LinkLibrary(EASTLLibrary)
	
	AddLibraryIncludes(RapidYAMLLibrary)
	LinkLibrary(RapidYAMLLibrary)

	postbuildcommands
	{
		-- Copy DLLs that the shader compiler needs
		CopyFileCommand(path.getabsolute(DxcLibrary.dlls[1]), '%{cfg.buildtarget.directory}'),
		CopyFileCommand(path.getabsolute(DxcLibrary.dlls[2]), '%{cfg.buildtarget.directory}')
	}

group('Resource')

SourceResourceDirectory = SourceDirectory..'/Resource'
SourceImageDirectory = SourceResourceDirectory..'/Image'
SourceModelDirectory = SourceResourceDirectory..'/Model'

project(ProjectResource)
	kind('StaticLib')
	pchheader('Resource/CrResource_pch.h')
	pchsource(SourceResourceDirectory..'/CrResource_pch.cpp')
	dependson { ProjectShaders }
	files
	{
		SourceResourceDirectory..'/**'
	}

	AddLibraryIncludes(StbLibrary)
	AddLibraryIncludes(CGLTFLibrary)
	AddLibraryIncludes(AssimpLibrary)

group('Core')

SourceCoreDirectory = SourceDirectory..'/Core'

project(ProjectCore)
	kind('StaticLib')
	
	pchheader('Core/CrCore_pch.h')
	pchsource(SourceCoreDirectory..'/CrCore_pch.cpp')
	
	files
	{
		SourceCoreDirectory..'/**',
		xxHashLibrary.includeDirs..'/xxhash.h'
	}

	AddLibraryNatvis(CRSTLLibrary)
	AddLibraryNatvis(EASTLLibrary)
	AddLibraryIncludes(SDL2Library)

	ExcludePlatformSpecificCode(SourceCoreDirectory)
	
	filter { 'system:windows' }
		files { SourceCoreDirectory..'/**/windows/**' }

	filter { 'system:linux' }
		files { SourceCoreDirectory..'/**/ansi/**' }

	filter {}

project(ProjectMath)
	kind('StaticLib')
	files { MathDirectory..'/**' }
	includedirs { MathDirectory }
	
	AddLibraryFiles(HlslppLibrary)
	AddLibraryNatvis(HlslppLibrary)

group('UnitTests')

project(ProjectUnitTests)
	kind('StaticLib')
	
	files
	{
		SourceUnitTestsDirectory..'/**'
	}

group('Editor')

project(ProjectEditor)
	kind('StaticLib')
	
	files
	{
		SourceEditorDirectory..'/**'
	}

group('.Solution Generation')

project('Generate Solution')
	kind('StaticLib')
	files{ '*.lua', '*.bat', '*.command' }

	local rootPathAbsolute = path.getabsolute('')
	local generateSolutionCommandLine = '{chdir} "'..rootPathAbsolute..'"'

	if IsVisualStudio then
		rebuildProjectCommand = '"Visual Studio '..VisualStudioVersion..'.bat"'
		print(rebuildProjectCommand)
	end

	postbuildcommands
	{
		generateSolutionCommandLine, -- Run
		rebuildProjectCommand,
	}