-- Dependencies for Corsair Engine

DependenciesDirectory   = 'Dependencies'
BinaryDirectory = '/Libraries/'
IncludeDirectory = '/Include/'

-- Library Directories
LibAgility            = DependenciesDirectory..'/agility'
LibAssimp             = DependenciesDirectory..'/assimp'
LibCGLTF              = DependenciesDirectory..'/cgltf'
LibCRSTL              = DependenciesDirectory..'/crstl'
LibDdspp              = DependenciesDirectory..'/ddspp'
LibDxc                = DependenciesDirectory..'/dxc'
LibEASTL              = DependenciesDirectory..'/eastl'
LibHlslpp             = DependenciesDirectory..'/hlslpp'
LibImGui              = DependenciesDirectory..'/imgui'
LibMeshOptimizer      = DependenciesDirectory..'/meshoptimizer'
LibMikkTSpace         = DependenciesDirectory..'/mikktspace'
LibRapidYAML          = DependenciesDirectory..'/rapidyaml'
LibRenderDoc          = DependenciesDirectory..'/renderdoc'
LibSDL2               = DependenciesDirectory..'/sdl2'
LibSPIRVReflect       = DependenciesDirectory..'/spirv-reflect'
LibStb                = DependenciesDirectory..'/stb'
LibUfbx               = DependenciesDirectory..'/ufbx'
LibVMA                = DependenciesDirectory..'/vma'
LibVulkan             = DependenciesDirectory..'/vulkan'
LibWinPixEventRuntime = DependenciesDirectory..'/winpixeventruntime'
LibWuffs              = DependenciesDirectory..'/wuffs'
LibxxHash             = DependenciesDirectory..'/xxHash'

AgilityLibrary =
{
	includeDirs = LibAgility..IncludeDirectory..'include',
	libDirs     = LibAgility..BinaryDirectory,
	dlls        =
	{
		LibAgility..BinaryDirectory..'x64/D3D12Core.dll',
		LibAgility..BinaryDirectory..'x64/d3d12SDKLayers.dll'
	}
}

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

CRSTLLibrary =
{
	includeDirs = LibCRSTL..IncludeDirectory..'include',
	natvis      = LibCRSTL..IncludeDirectory..'include/*.natvis',
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
		"CHAR8_T_DEFINED" -- We don't want EASTL to define char8_t regardless of compiler options
	},
	natvis      = LibEASTL..IncludeDirectory..'doc/**.natvis',
	libDirs     = LibEASTL..BinaryDirectory,
	libNames    = 'EASTL.vs2019.release'
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
	libNames    = 'ImGui.vs2022.release',
	defines = 'IMGUI_DISABLE_OBSOLETE_FUNCTIONS'
}

MeshOptimizerLibrary =
{
	includeDirs = LibMeshOptimizer..IncludeDirectory..'src',
	libDirs     = LibMeshOptimizer..BinaryDirectory,
	libNames    = 'MeshOptimizer.vs2022.release',
}

MikkTSpaceLibrary =
{
	includeDirs = LibMikkTSpace..IncludeDirectory,
	libDirs     = LibMikkTSpace..BinaryDirectory,
	libNames    = 'MikkTSpace.vs2022.release',
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
	includeDirs = LibStb..IncludeDirectory,
	libDirs     = LibStb..BinaryDirectory,
	libNames    = 'Stb.vs2022.release'
}

UfbxLibrary =
{
	includeDirs = LibUfbx..IncludeDirectory,
	libDirs     = LibUfbx..BinaryDirectory,
	libNames    = 'Ufbx.vs2022.release'
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

WuffsLibrary =
{
	includeDirs = { LibWuffs..IncludeDirectory },
	libDirs = { LibWuffs..BinaryDirectory },
	libNames = 'Wuffs.vs2022.release'
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