-- Dependencies for Corsair Engine

DependenciesDirectory   = 'Dependencies'
BinaryDirectory = '/Libraries/'
IncludeDirectory = '/Include/'

-- Library Directories
LibAssimp       = DependenciesDirectory..'/assimp'
LibCGLTF        = DependenciesDirectory..'/cgltf'
LibDdspp        = DependenciesDirectory..'/ddspp'
LibDxc          = DependenciesDirectory..'/dxc'
LibEASTL        = DependenciesDirectory..'/eastl'
LibGlslang      = DependenciesDirectory..'/glslang'
LibHalf         = DependenciesDirectory..'/half'
LibHlslpp       = DependenciesDirectory..'/hlslpp'
LibImGui        = DependenciesDirectory..'/imgui'
LibRapidYAML    = DependenciesDirectory..'/rapidyaml'
LibRenderDoc    = DependenciesDirectory..'/renderdoc'
LibSDL2         = DependenciesDirectory..'/sdl2'
LibSPIRVReflect = DependenciesDirectory..'/spirv-reflect'
LibStb          = DependenciesDirectory..'/stb'
LibVMA          = DependenciesDirectory..'/vma'
LibVulkan       = DependenciesDirectory..'/vulkan'
LibWinPixEventRuntime = DependenciesDirectory..'/winpixeventruntime'
LibxxHash       = DependenciesDirectory..'/xxHash'

AssimpLibrary =
{
	includeDirs = LibAssimp..IncludeDirectory..'include',
	libDirs     = LibAssimp..BinaryDirectory,
	libNames    = 'Assimp.vs2019.release'
}

CGLTFLibrary =
{
	includeDirs = LibCGLTF..IncludeDirectory
}

D3D12Library =
{
	libNames = {'dxgi', 'd3d12'}
}

DdsppLibrary =
{
	includeDirs = LibDdspp..IncludeDirectory
}

-- https://github.com/microsoft/DirectXShaderCompiler
DxcLibrary =
{
	includeDirs = LibDxc..IncludeDirectory,
	libDirs     = LibDxc..BinaryDirectory,
	libNames    = { 'dxcompiler' },
	dlls        = { LibDxc..BinaryDirectory..'dxcompiler.dll', LibDxc..BinaryDirectory..'dxil.dll' }
}

EASTLLibrary =
{
	includeDirs =
	{ 
		LibEASTL..IncludeDirectory..'include',
		LibEASTL..IncludeDirectory..'test/packages/EAStdC/include', 
		LibEASTL..IncludeDirectory..'test/packages/EAAssert/include',
		LibEASTL..IncludeDirectory..'test/packages/EABase/include/Common'
	},
	defines =
	{
		"EASTL_ASSERT_ENABLED=1",
	},
	natvis      = LibEASTL..IncludeDirectory..'doc/**.natvis',
	libDirs     = LibEASTL..BinaryDirectory,
	libNames    = 'EASTL.vs2019.release'
}

GlslangLibrary =
{
	includeDirs = LibGlslang..IncludeDirectory,
	libDirs     = LibGlslang..BinaryDirectory,
	libNames    = 'Glslang.vs2019.release'
}

HalfLibrary =
{
	includeDirs = LibHalf..IncludeDirectory,
	defines     = 
	{
		'HALF_ENABLE_CPP11_NOEXCEPT=1',
		'HALF_ENABLE_CPP11_CONSTEXPR=1',
		'HALF_ENABLE_CPP11_CSTDINT=1',
		'HALF_ENABLE_CPP11_USER_LITERALS=1',
		'HALF_ENABLE_CPP11_STATIC_ASSERT=1',
		'HALF_ENABLE_CPP11_HASH=0',
		'HALF_ENABLE_CPP11_TYPE_TRAITS=0',
		'HALF_ENABLE_CPP11_CMATH=0',
		'HALF_ENABLE_CPP11_CFENV=0'
	}
}

HlslppLibrary =
{
	includeDirs = LibHlslpp..IncludeDirectory..'include',
	files = LibHlslpp..IncludeDirectory..'include/**.h',
	natvis = LibHlslpp..IncludeDirectory..'include/**.natvis',
	defines = 'HLSLPP_FEATURE_TRANSFORM'
}

ImguiLibrary =
{
	includeDirs = LibImGui..IncludeDirectory,
	libDirs     = LibImGui..BinaryDirectory,
	libNames    = 'ImGui.vs2019.release',
	defines = 'IMGUI_DISABLE_OBSOLETE_FUNCTIONS'
}

RapidYAMLLibrary =
{
	includeDirs = 
	{
		LibRapidYAML..IncludeDirectory..'src',
		LibRapidYAML..IncludeDirectory..'ext/c4core/src',
	},
	libDirs     = LibRapidYAML..BinaryDirectory,
	libNames    = 'rapidyaml.vs2019.release'
}

RenderDocLibrary =
{
	includeDirs = { LibRenderDoc..IncludeDirectory }
}

SDL2Library =
{
	includeDirs = LibSDL2..IncludeDirectory..'include',
	libDirs     = LibSDL2..BinaryDirectory,
	libNames    = 'SDL2',
	defines     = 'SDL_MAIN_HANDLED',
	dlls        = LibSDL2..BinaryDirectory..'SDL2.dll'
}

SPIRVReflectLibrary =
{
	includeDirs = LibSPIRVReflect..IncludeDirectory,
	libDirs     = LibSPIRVReflect..BinaryDirectory,
	libNames    = 'SPIRV-Reflect.vs2019.release'
}

StbLibrary =
{
	includeDirs = LibStb..IncludeDirectory
}

VulkanLibrary =
{
	includeDirs = { LibVulkan..IncludeDirectory..'include', LibVMA..IncludeDirectory..'include' },
	libDirs     = LibVulkan..BinaryDirectory,
	natvis      = LibVMA..IncludeDirectory..'src/**.natvis',
	libNames    = 'vulkan-1'
}

WinPixEventRuntimeLibrary =
{
	includeDirs = { LibWinPixEventRuntime..IncludeDirectory },
	libDirs = { LibWinPixEventRuntime..BinaryDirectory },
	libNames = 'WinPixEventRuntime',
	dlls = LibWinPixEventRuntime..BinaryDirectory..'WinPixEventRuntime.dll'
}

xxHashLibrary =
{
	includeDirs = LibxxHash..IncludeDirectory
}

function AddLibraryIncludes(library)
	includedirs(library.includeDirs)
	if(library['defines']) then
		defines(library.defines)
	end
end

-- Add files to the solution. This should generally not include cpp files
-- and most of the time is to aid intellisense and parsing of includes
function AddLibraryFiles(library)
	files(library.files)
end

function AddLibraryNatvis(library)
	files(library.natvis)
end

function LinkLibrary(library)
	libdirs(library.libDirs)
	links(library.libNames)
end