-- Dependencies for Corsair Engine

DependenciesDirectory   = 'Dependencies'
BinaryDirectory = '/Libraries/'
IncludeDirectory = '/Include/'

-- Library Directories
LibAssimp       = DependenciesDirectory..'/assimp'
LibDdspp        = DependenciesDirectory..'/ddspp'
LibEASTL        = DependenciesDirectory..'/eastl'
LibGlslang      = DependenciesDirectory..'/glslang'
LibHalf         = DependenciesDirectory..'/half'
LibHlslpp       = DependenciesDirectory..'/hlslpp'
LibImGui        = DependenciesDirectory..'/imgui'
LibRapidYAML    = DependenciesDirectory..'/rapidyaml'
LibSDL2         = DependenciesDirectory..'/sdl2'
LibSPIRVReflect = DependenciesDirectory..'/spirv-reflect'
LibStb          = DependenciesDirectory..'/stb'
LibTinyGLTF     = DependenciesDirectory..'/tinygltf'
LibVMA          = DependenciesDirectory..'/vma'
LibVulkan       = DependenciesDirectory..'/vulkan'
LibWinPixEventRuntime = DependenciesDirectory..'/winpixeventruntime'
LibxxHash       = DependenciesDirectory..'/xxHash'

AssimpLibrary =
{
	includeDirs = LibAssimp..IncludeDirectory..'include',
	libDirs     = LibAssimp..BinaryDirectory,
	libNames    = 'Assimp.'.._ACTION..'.release'
}

D3D12Library =
{
	libNames = {'dxgi', 'd3d12'}
}

DdsppLibrary =
{
	includeDirs = LibDdspp..IncludeDirectory
}

-- Vulkan comes with the dxc parsing library, and it is regularly updated. To avoid extra complexity,
-- we'll source it from there
DxcLibrary =
{
	includeDirs = LibVulkan..IncludeDirectory..'include',
	libDirs = LibVulkan..BinaryDirectory,
	libNames = {'dxcompiler'}
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
	libNames    = 'EASTL.'.._ACTION..'.release'
}

GlslangLibrary =
{
	includeDirs = LibGlslang..IncludeDirectory,
	libDirs     = LibGlslang..BinaryDirectory,
	libNames    = 'Glslang.'.._ACTION..'.release'
}

HalfLibrary =
{
	includeDirs = LibHalf..IncludeDirectory,
	defines     = 
	{
		'HALF_ENABLE_CPP11_NOEXCEPT=1',
		'HALF_ENABLE_CPP11_CONSTEXPR=1',
		'HALF_ENABLE_CPP11_TYPE_TRAITS=1',
		'HALF_ENABLE_CPP11_CSTDINT=1',
		'HALF_ENABLE_CPP11_USER_LITERALS=1',
		'HALF_ENABLE_CPP11_STATIC_ASSERT=1',
		'HALF_ENABLE_CPP11_CMATH=1'
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
	libNames    = 'ImGui.'.._ACTION..'.release'
}

LibRapidYAML =
{
	includeDirs = 
	{
		LibRapidYAML..IncludeDirectory..'src',
		LibRapidYAML..IncludeDirectory..'ext/c4core/src',
	},
	libDirs     = LibRapidYAML..BinaryDirectory,
	libNames    = 'rapidyaml.'.._ACTION..'.release'
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
	libNames    = 'SPIRV-Reflect.'.._ACTION..'.release'
}

StbLibrary =
{
	includeDirs = LibStb..IncludeDirectory
}

TinyGLTFLibrary = 
{
	includeDirs = LibTinyGLTF..IncludeDirectory
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