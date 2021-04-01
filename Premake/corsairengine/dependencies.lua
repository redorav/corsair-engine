-- Dependencies for Corsair Engine

DependenciesDirectory   = 'Dependencies'
BinaryDirectory = '/Binaries/'

-- Careful, the names are debug and release but this depends on this project's naming as well
LibConfig = '.'.._ACTION..'.%{cfg.buildcfg:lower()}'

-- Library Directories
LibVulkan       = DependenciesDirectory..'/vulkan'
LibEASTL        = DependenciesDirectory..'/eastl'
LibGlslang      = DependenciesDirectory..'/glslang'
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
LibImGui        = DependenciesDirectory..'/imgui'
LibTinyGLTF     = DependenciesDirectory..'/tinygltf'

TinyGLTFLibrary = 
{
	includeDirs = LibTinyGLTF..'/Source'
}

AssimpLibrary =
{
	includeDirs = LibAssimp..'/Source/include',
	libDirs     = LibAssimp..BinaryDirectory,
	libNames    = 'Assimp'..LibConfig
}

VulkanLibrary =
{
	includeDirs = LibVulkan..'/Source/include',
	libDirs = LibVulkan..BinaryDirectory,
	libNames = 'vulkan-1'
}

D3D12Library =
{
	libNames = {'dxgi', 'd3d12'}
}

XInputLibrary =
{
	libNames = 'Xinput9_1_0'
}

GlslangLibrary =
{
	includeDirs = LibGlslang..'/Source/',
	libDirs     = LibGlslang..BinaryDirectory,
	libNames    = 'Glslang'..LibConfig
}

EASTLLibrary =
{
	includeDirs =
	{ 
		LibEASTL..'/Source/include', LibEASTL..'/Source/test/packages/EAStdC/include', 
		LibEASTL..'/Source/test/packages/EAAssert/include', LibEASTL..'/Source/test/packages/EABase/include/Common'
	},
	libDirs     = LibEASTL..BinaryDirectory,
	libNames    = 'EASTL'..LibConfig
}

SDL2Library =
{
	includeDirs = LibSDL2..'/Source/include',
	libDirs     = LibSDL2..BinaryDirectory,
	libNames    = 'SDL2',
	defines     = 'SDL_MAIN_HANDLED'
}

ImguiLibrary =
{
	includeDirs = LibImGui..'/Source/',
	libDirs     = LibImGui..BinaryDirectory,
	libNames    = 'ImGui'..LibConfig
}

SPIRVReflectLibrary =
{
	includeDirs = LibSPIRVReflect..'/Source/',
	libDirs     = LibSPIRVReflect..BinaryDirectory,
	libNames    = 'SPIRV-Reflect'..LibConfig
}

HlslppLibrary =
{
	includeDirs = LibHlslpp..'/Source/include',
	defines = 'HLSLPP_FEATURE_TRANSFORM'
}

StbLibrary =
{
	includeDirs = LibStb..'/Source'
}

xxHashLibrary =
{
	includeDirs = LibxxHash..'/Source'
}

function AddLibraryHeaders(library)
	includedirs(library.includeDirs)
	if(library["defines"]) then defines(library.defines) end
end

function LinkLibrary(library)
	libdirs(library.libDirs)
	links(library.libNames)
end