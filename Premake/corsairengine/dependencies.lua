-- Dependencies for Corsair Engine

DependenciesDirectory   = 'Dependencies'
BinaryDirectory = '/Binaries/'
IncludeDirectory = '/Source/'

-- Library Directories
LibArgh         = DependenciesDirectory..'/argh'
LibAssimp       = DependenciesDirectory..'/assimp'
LibDdspp        = DependenciesDirectory..'/ddspp'
LibEASTL        = DependenciesDirectory..'/eastl'
LibGlslang      = DependenciesDirectory..'/glslang'
LibHalf         = DependenciesDirectory..'/half'
LibHlslpp       = DependenciesDirectory..'/hlslpp'
LibImGui        = DependenciesDirectory..'/imgui'
LibSDL2         = DependenciesDirectory..'/sdl2'
LibSPIRVReflect = DependenciesDirectory..'/spirv-reflect'
LibStb          = DependenciesDirectory..'/stb'
LibTinyGLTF     = DependenciesDirectory..'/tinygltf'
LibVulkan       = DependenciesDirectory..'/vulkan'
LibxxHash       = DependenciesDirectory..'/xxHash'

ArghLibrary =
{
	includeDirs = LibArgh..IncludeDirectory
}

AssimpLibrary =
{
	includeDirs = LibAssimp..IncludeDirectory..'include',
	libDirs     = LibAssimp..BinaryDirectory,
	libNames    = 'Assimp.'.._ACTION..".release"
}

VulkanLibrary =
{
	includeDirs = LibVulkan..IncludeDirectory..'include',
	libDirs = LibVulkan..BinaryDirectory,
	libNames = 'vulkan-1'
}

D3D12Library =
{
	libNames = {'dxgi', 'd3d12'}
}

DdsppLibrary =
{
	includeDirs = LibDdspp..IncludeDirectory
}

EASTLLibrary =
{
	includeDirs =
	{ 
		LibEASTL..IncludeDirectory..'include', LibEASTL..IncludeDirectory..'test/packages/EAStdC/include', 
		LibEASTL..IncludeDirectory..'test/packages/EAAssert/include', LibEASTL..IncludeDirectory..'test/packages/EABase/include/Common'
	},
	libDirs     = LibEASTL..BinaryDirectory,
	libNames    = 'EASTL.'.._ACTION..".release"
}

GlslangLibrary =
{
	includeDirs = LibGlslang..IncludeDirectory,
	libDirs     = LibGlslang..BinaryDirectory,
	libNames    = 'Glslang.'.._ACTION..".release"
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
	defines = 'HLSLPP_FEATURE_TRANSFORM'
}

ImguiLibrary =
{
	includeDirs = LibImGui..IncludeDirectory,
	libDirs     = LibImGui..BinaryDirectory,
	libNames    = 'ImGui.'.._ACTION..".release"
}

SDL2Library =
{
	includeDirs = LibSDL2..IncludeDirectory..'include',
	libDirs     = LibSDL2..BinaryDirectory,
	libNames    = 'SDL2',
	defines     = 'SDL_MAIN_HANDLED'
}

SPIRVReflectLibrary =
{
	includeDirs = LibSPIRVReflect..IncludeDirectory,
	libDirs     = LibSPIRVReflect..BinaryDirectory,
	libNames    = 'SPIRV-Reflect.'.._ACTION..".release"
}

StbLibrary =
{
	includeDirs = LibStb..IncludeDirectory
}

TinyGLTFLibrary = 
{
	includeDirs = LibTinyGLTF..IncludeDirectory
}

xxHashLibrary =
{
	includeDirs = LibxxHash..IncludeDirectory
}

function AddLibraryIncludes(library)
	includedirs(library.includeDirs)
	if(library["defines"]) then defines(library.defines) end
end

function LinkLibrary(library)
	libdirs(library.libDirs)
	links(library.libNames)
end