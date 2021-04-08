-- Dependencies for Corsair Engine

DependenciesDirectory   = 'Dependencies'
BinaryDirectory = '/Binaries/'

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
	libNames    = 'Assimp.'.._ACTION..".release"
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
	libNames    = 'Glslang.'.._ACTION..".release"
}

EASTLLibrary =
{
	includeDirs =
	{ 
		LibEASTL..'/Source/include', LibEASTL..'/Source/test/packages/EAStdC/include', 
		LibEASTL..'/Source/test/packages/EAAssert/include', LibEASTL..'/Source/test/packages/EABase/include/Common'
	},
	libDirs     = LibEASTL..BinaryDirectory,
	libNames    = 'EASTL.'.._ACTION..".release"
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
	libNames    = 'ImGui.'.._ACTION..".release"
}

SPIRVReflectLibrary =
{
	includeDirs = LibSPIRVReflect..'/Source/',
	libDirs     = LibSPIRVReflect..BinaryDirectory,
	libNames    = 'SPIRV-Reflect.'.._ACTION..".release"
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