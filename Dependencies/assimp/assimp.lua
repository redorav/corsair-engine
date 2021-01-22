require("../../premake/corsairengine/common")

ProjectName = "Assimp"

workspace(ProjectName)
	configurations { "Debug", "Release" }
	location ("Build/".._ACTION)
	architecture("x64")
	cppdialect("c++17")
	debugformat("c7") -- Do not create pdbs, instead store in lib
	
	filter ("system:windows")
		toolset("msc") -- Use default VS toolset
		--toolset("msc-llvm-vs2014") -- Use for Clang on VS
		flags { "multiprocessorcompile" }
		buildoptions { "/bigobj" } -- fails to compile on MSVC unless this is specified
	
	configuration "Debug"
		defines { "DEBUG", "_DEBUG" }
		symbols "on"
		editandcontinue("off")

	configuration "Release"
		defines { "NDEBUG" }
		optimize "on"
		--flags { "linktimeoptimization" }
	
project (ProjectName)
	kind("StaticLib")
	language("C++")

	targetdir("Binaries/")
	targetname("%{wks.name}.".._ACTION..".%{cfg.buildcfg:lower()}")
	
	-- Remove tons of importers/exporters we don't care about
	-- I don't understand this negative logic. Let me add what
	-- I want, not the other way around!
	defines
	{
		"ASSIMP_BUILD_NO_X_IMPORTER", -- DirectX file format
		"ASSIMP_BUILD_NO_OBJ_IMPORTER", -- Wavefront OBJ
		"ASSIMP_BUILD_NO_AMF_IMPORTER", -- 3D printing
		"ASSIMP_BUILD_NO_3DS_IMPORTER", -- 3D Studio Max
		"ASSIMP_BUILD_NO_MD3_IMPORTER", -- Quake 3
		"ASSIMP_BUILD_NO_MD2_IMPORTER", -- Quake 2
		"ASSIMP_BUILD_NO_PLY_IMPORTER", -- Standford Polygon Library
		"ASSIMP_BUILD_NO_MDL_IMPORTER", -- Quake 1	
		"ASSIMP_BUILD_NO_ASE_IMPORTER", -- 3D Studio Max ASE
		"ASSIMP_BUILD_NO_HMP_IMPORTER", -- 3D Gamestudio
		"ASSIMP_BUILD_NO_SMD_IMPORTER", -- Valve
		"ASSIMP_BUILD_NO_MDC_IMPORTER", -- Return to Castle Wolfenstein
		"ASSIMP_BUILD_NO_MD5_IMPORTER", -- Doom 3
		"ASSIMP_BUILD_NO_STL_IMPORTER", -- Stereolithography (3D printing)
		"ASSIMP_BUILD_NO_LWO_IMPORTER", -- Lightwave
		"ASSIMP_BUILD_NO_DXF_IMPORTER", -- AutoCAD
		"ASSIMP_BUILD_NO_NFF_IMPORTER", -- Neutral File Format
		"ASSIMP_BUILD_NO_RAW_IMPORTER", -- PovRAY
		"ASSIMP_BUILD_NO_SIB_IMPORTER", -- 
		"ASSIMP_BUILD_NO_OFF_IMPORTER", -- Object File Format
		"ASSIMP_BUILD_NO_AC_IMPORTER",  -- AC3D
		"ASSIMP_BUILD_NO_BVH_IMPORTER", -- Motion capture format
		"ASSIMP_BUILD_NO_IRRMESH_IMPORTER", -- Irrlicht Mesh
		"ASSIMP_BUILD_NO_IRR_IMPORTER", -- Irrlicht
		"ASSIMP_BUILD_NO_Q3D_IMPORTER", -- Quick3D	
		"ASSIMP_BUILD_NO_B3D_IMPORTER", -- BlitzBasic
		"ASSIMP_BUILD_NO_COLLADA_IMPORTER", -- Collada
		"ASSIMP_BUILD_NO_TERRAGEN_IMPORTER", -- Terragen
		"ASSIMP_BUILD_NO_CSM_IMPORTER", -- Motion capture format
		"ASSIMP_BUILD_NO_3D_IMPORTER", -- Unreal
		"ASSIMP_BUILD_NO_LWS_IMPORTER", -- Lightwave Scene
		"ASSIMP_BUILD_NO_OGRE_IMPORTER", -- Ogre
		"ASSIMP_BUILD_NO_OPENGEX_IMPORTER", -- Open Graphics Exchange Format
		"ASSIMP_BUILD_NO_MS3D_IMPORTER", -- Milkshake
		"ASSIMP_BUILD_NO_COB_IMPORTER", -- Truespace
		"ASSIMP_BUILD_NO_BLEND_IMPORTER", -- Blender 3D
		"ASSIMP_BUILD_NO_Q3BSP_IMPORTER", --
		"ASSIMP_BUILD_NO_NDO_IMPORTER", -- Izware Nendo
		"ASSIMP_BUILD_NO_IFC_IMPORTER", -- Industry Foundation Classes
		"ASSIMP_BUILD_NO_XGL_IMPORTER", -- OpenGL XML
		--"ASSIMP_BUILD_NO_FBX_IMPORTER", -- Autodesk FBX
		"ASSIMP_BUILD_NO_ASSBIN_IMPORTER", -- 
		--"ASSIMP_BUILD_NO_GLTF_IMPORTER", -- GLTF
		"ASSIMP_BUILD_NO_C4D_IMPORTER", -- Cinema 4D	
		"ASSIMP_BUILD_NO_3MF_IMPORTER", -- 3D printing
		"ASSIMP_BUILD_NO_X3D_IMPORTER", -- Standard XML based format
		"ASSIMP_BUILD_NO_MMD_IMPORTER", -- 
		
		"ASSIMP_BUILD_NO_X_EXPORTER",
		"ASSIMP_BUILD_NO_STEP_EXPORTER",
		"ASSIMP_BUILD_NO_STL_EXPORTER",
		"ASSIMP_BUILD_NO_PLY_EXPORTER",
		"ASSIMP_BUILD_NO_3DS_EXPORTER",
		"ASSIMP_BUILD_NO_COLLADA_EXPORTER",
		"ASSIMP_BUILD_NO_OBJ_EXPORTER",
		"ASSIMP_BUILD_NO_GLTF_EXPORTER",
		"ASSIMP_BUILD_NO_ASSBIN_EXPORTER",
		"ASSIMP_BUILD_NO_ASSXML_EXPORTER",
		"ASSIMP_BUILD_NO_X3D_EXPORTER",
		"ASSIMP_BUILD_NO_3MF_EXPORTER",		
		
		"ASSIMP_BUILD_NO_COMPRESSED_X",
		"ASSIMP_BUILD_NO_COMPRESSED_IFC",
		"ASSIMP_BUILD_NO_COMPRESSED_BLEND",
		"ASSIMP_BUILD_NO_OWN_ZLIB",
		
		"OPENDDL_STATIC_LIBARY", -- Or it will try to export functions for a DLL
		"_CRT_SECURE_NO_WARNINGS",
	}
	
	files 
	{
		"Source/code/**.cpp", "Source/code/**.h", "Source/code/**.hpp",
		"Source/include/**.cpp", "Source/include/**.h", "Source/include/**.hpp",
			
		--"Source/contrib/openddlparser/code/**.cpp",
		"Source/contrib/zlib/*.c", "Source/contrib/zlib/*.h",
		--"Source/contrib/irrXML/**.cpp", "Source/contrib/irrXML/**.h",
	}
	
	-- Remove a bunch of source files. Instead of defining them out (which produces warnings about .obj files not
	-- defining any symbols) we strip all files related to the feature
	removefiles
	{
		"Source/code/3ds*", -- 3D Studio Max
		"Source/code/*3mf*", -- 3D printing
		"Source/code/acloader*", -- 3D printing
		"Source/code/amf*", -- 3D printing
		"Source/code/ase*", -- 3D Studio Max ASE
		"Source/code/assbin*", 
		"Source/code/assxml*",
		"Source/code/b3d*", -- BlitzBasic
		"Source/code/blender*", -- Blender
		"Source/code/bvh*", -- Motion capture format
		"Source/code/c4d*", -- Cinema 4D
		"Source/code/cob*", -- Truespace
		"Source/code/collada*", -- COLLADA
		"Source/code/csm*", -- Motion capture format
		"Source/code/dxf*", -- AutoCAD		
		"Source/code/hmp*", -- 3D Gamestudio
		"Source/code/halflife*", -- Half Life
		"Source/code/fir*", -- X3D Standard XML based format
		"Source/code/irr*", -- Irrlicht
		"Source/code/lwo*", -- Lightwave
		"Source/code/lws*", -- Lightwave Scene		
		"Source/code/mdc*", -- Return to Castle Wolfenstein
		"Source/code/mmd*",
		"Source/code/mdl*", -- Quake 1
		"Source/code/md2*", -- Quake 2
		"Source/code/md3*", -- Quake 3
		"Source/code/md4*", 
		"Source/code/md5*", -- Doom 3
		"Source/code/ms3d*", -- Milkshake		
		"Source/code/ndo*", -- Izware Nendo
		"Source/code/nff*", -- Neutral File Format
		"Source/code/obj*", -- Wavefront OBJ
		"Source/code/off*", -- Object File Format
		"Source/code/ogre*", -- Ogre
		"Source/code/opengex*", -- Open Graphics Exchange Format
		"Source/code/ply*", -- Ogre
		"Source/code/raw*", -- PovRAY
		"Source/code/q3bsp*", -- Quake 3
		"Source/code/q3d*", -- Quick3D
		"Source/code/sib*", -- 
		"Source/code/smd*", -- Valve
		"Source/code/step*", 
		"Source/code/stl*", -- Stereolithography (3D printing)
		"Source/code/terragen*", -- Stereolithography (3D printing)
		"Source/code/unreal*", -- Unreal
		"Source/code/x3d*", -- Standard XML based format
		"Source/code/xfile*", -- DirectX file format
		"Source/code/xgl*", -- OpenGL XML
		"Source/code/importer/**" -- Industry Foundation Classes
	}
			
	sysincludedirs
	{				
		"Source",
		"Source/include",
		"Source/contrib/rapidjson/include",
		"Source/contrib/irrXML",
		"Source/contrib/unzip",
		"Source/contrib/utf8cpp/source",
		"Source/contrib/openddlparser/include",
		"Source/contrib/zlib",
	}