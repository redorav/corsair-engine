require("../../premake/corsairengine/common")

ProjectName = "SPIRV-Tools"

local objDir =  "%{cfg.objdir}/generated/"

workspace(ProjectName)
	configurations { "Debug", "Release" }
	location ("Build/".._ACTION)
	cppdialect("c++17")
	
	if _ACTION == "vs2017" then
		architecture("x64")
		toolset("msc")
	end
	
	targetdir("Binaries/")
	targetname("%{wks.name}.".._ACTION..".%{cfg.buildcfg:lower()}")

	-- The code doesn't compile with edit and continue on, complains about narrowing conversions.
	-- I don't quite know why turning them off fixes it.
	editandcontinue("off")
	
	configuration "Debug"
		defines { "DEBUG" }
		symbols ("on")

	configuration "Release"
		defines { "NDEBUG" }
		optimize ("on")
	
project (ProjectName)
	kind("StaticLib")
	language("C++")
	
	filter { "configurations:*" } -- Workaround for MacOS nil in cfg
	
	includedirs
	{
		"Source",
		"Source/include",
		"Source/source",
		"Source/external/SPIRV-Headers/include",
		objDir
	}
		
	files
	{
		"Source/source/**.cpp", 
		"Source/source/**.hpp", 
		"Source/source/**.h", 
		"Source/include/**.h", 
		"Source/include/**.hpp"
	}
	
	defines { "_CRT_SECURE_NO_WARNINGS", "SPIRV_WINDOWS" }
	
	local sourceDir = path.getabsolute("Source/source")
	local utilsDir = path.getabsolute("Source/utils")
	local grammarDir = path.getabsolute("Source/external/spirv-headers/include/spirv/1.2")
	
	local pythonDir = "\""..path.getabsolute("WinPython/python-3.6.5.amd64/python.exe").."\""
	
	local grammarTablePyPath = utilsDir.."/generate_grammar_tables.py"
	local buildVersionPyPath = utilsDir.."/update_build_version.py"
	local generatorsPyPath = utilsDir.."/generate_registry_tables.py"
	
	-- SPIRV enum-string mapping	
	local spirvCoreGrammarPath = grammarDir.."/spirv.core.grammar.json"
	local extensionEnumPath = objDir.."extension_enum.inc"
	local enumStringMappingPath = objDir.."enum_string_mapping.inc"
	
	-- GLSL Info Tables
	local extGlslGrammarPath = grammarDir.."/extinst.glsl.std.450.grammar.json"
	local glslInstructionsPath = objDir.."glsl.std.450.insts-1.0.inc"
	
	-- SPIRV Info Tables
	local spirvInstructions10TablePath = objDir.."core.insts-1.0.inc"
	local spirvOperandsTable10Path = objDir.."operand.kinds-1.0.inc"
	local spirvInstructions11TablePath = objDir.."core.insts-1.1.inc"
	local spirvOperandsTable11Path = objDir.."operand.kinds-1.1.inc"
	local spirvInstructions12TablePath = objDir.."core.insts-1.2.inc"
	local spirvOperandsTable12Path = objDir.."operand.kinds-1.2.inc"
	
	-- OpenCL Info Tables
	local extOpenCLGrammarPath = grammarDir.."/extinst.opencl.std.100.grammar.json"
	local openclInstructionsPath = objDir.."opencl.std.insts-1.0.inc"
	
	-- AMD GCN Info Tables
	local extAmdGcnGrammarPath = sourceDir.."/extinst.spv-amd-gcn-shader.grammar.json"
	local amdGcnInstructionsPath = objDir.."spv-amd-gcn-shader.insts.inc"
	
	-- Debug Info Tables
	local debugInfoTablesPath = sourceDir.."/extinst.debuginfo.grammar.json"
	
	local extAmdShaderBallotGrammarPath = sourceDir.."/extinst.spv-amd-shader-ballot.grammar.json"
	local extAmdShaderBallotInstructionsPath = objDir.."spv-amd-shader-ballot.insts.inc"
	
	local extAmdExplicitVertexParamGrammarPath = sourceDir.."/extinst.spv-amd-shader-explicit-vertex-parameter.grammar.json"
	local extAmdExplicitVertexParamInstructionsPath = objDir.."spv-amd-shader-explicit-vertex-parameter.insts.inc"
	
	local extAmdTrinaryMinMaxGrammarPath = sourceDir.."/extinst.spv-amd-shader-trinary-minmax.grammar.json"
	local extAmdTrinaryMinMaxInstructionsPath = objDir.."spv-amd-shader-trinary-minmax.insts.inc"
	
	local debugInfoOption = "\"--extinst-debuginfo-grammar="..debugInfoTablesPath.."\""
	
	local enumStringMappingPath = objDir.."enum_string_mapping.inc"
	local spirvGrammarCommandLineBase 	= pythonDir.." \""..grammarTablePyPath.."\"".." \"--spirv-core-grammar="..spirvCoreGrammarPath.."\" "..debugInfoOption
	local glslGrammarCommandLineBase 	= pythonDir.." \""..grammarTablePyPath.."\"".." \"--extinst-glsl-grammar="..extGlslGrammarPath.."\""
	local enumStringMappingCmdLine =	glslGrammarCommandLineBase.." \"--spirv-core-grammar="..spirvCoreGrammarPath.."\" "..debugInfoOption.." \"--extension-enum-output="..extensionEnumPath.."\""..
										" \"--enum-string-mapping-output="..enumStringMappingPath.."\""..
										" \"--glsl-insts-output="..glslInstructionsPath.."\""
		
	local spirv10TablesCmdLine			= spirvGrammarCommandLineBase.." \"--core-insts-output="..spirvInstructions10TablePath.."\"".." \"--operand-kinds-output="..spirvOperandsTable10Path.."\""
	local spirv11TablesCmdLine			= spirvGrammarCommandLineBase.." \"--core-insts-output="..spirvInstructions11TablePath.."\"".." \"--operand-kinds-output="..spirvOperandsTable11Path.."\""
	local spirv12TablesCmdLine			= spirvGrammarCommandLineBase.." \"--core-insts-output="..spirvInstructions12TablePath.."\"".." \"--operand-kinds-output="..spirvOperandsTable12Path.."\""
	
	-- Build version
	local buildVersionPath = objDir.."build-version.inc"
	local buildVersionCmdLine =			pythonDir.." \""..buildVersionPyPath.."\"".." \""..path.getabsolute("Source").."\"".." \""..buildVersionPath.."\""
	
	-- Generators
	local generatorsPath 	= objDir.."generators.inc"
	local generatorsCmdLine	= pythonDir.." \""..generatorsPyPath.."\"".." --xml \""..path.getabsolute("Source/external/spirv-headers/include/spirv").."/spir-v.xml\"".." \"--generator-output="..generatorsPath.."\""
	
	-- OpenCL
	local openclCmdLine	= spirvGrammarCommandLineBase.." \"--extinst-opencl-grammar="..extOpenCLGrammarPath.."\"".." \"--opencl-insts-output="..openclInstructionsPath.."\""	
	
	-- AMD GCN
	local amdGcnCmdLine	= spirvGrammarCommandLineBase.." \"--extinst-vendor-grammar="..extAmdGcnGrammarPath.."\"".." \"--vendor-insts-output="..amdGcnInstructionsPath.."\""
	
	-- AMD Shader Ballot
	local amdShaderBallotCmdLine	= spirvGrammarCommandLineBase.." \"--extinst-vendor-grammar="..extAmdShaderBallotGrammarPath.."\"".." \"--vendor-insts-output="..extAmdShaderBallotInstructionsPath.."\""
	
	-- AMD Explicit Vertex Param
	local amdExplicitVertexParamCmdLine	= spirvGrammarCommandLineBase.." \"--extinst-vendor-grammar="..extAmdExplicitVertexParamGrammarPath.."\"".." \"--vendor-insts-output="..extAmdExplicitVertexParamInstructionsPath.."\""
	
	-- AMD Trinary MinMax
	local amdTrinaryMinMaxCmdLine	= spirvGrammarCommandLineBase.." \"--extinst-vendor-grammar="..extAmdTrinaryMinMaxGrammarPath.."\"".." \"--vendor-insts-output="..extAmdTrinaryMinMaxInstructionsPath.."\""
	
	prebuildcommands
	{
		"{echo} Generate enum-string mapping for SPIR-V v1.2", 
		"{echo} Generate info tables for GLSL extended instructions and operands v1.0",
		"{echo} Generate info tables for SPIR-V v1.0 core instructions and operands.",
		"{echo} "..enumStringMappingCmdLine, enumStringMappingCmdLine,
		"{echo} "..spirv10TablesCmdLine, spirv10TablesCmdLine,
		"{echo} "..spirv11TablesCmdLine, spirv11TablesCmdLine,
		"{echo} "..spirv12TablesCmdLine, spirv12TablesCmdLine,
		"{echo} Update build-version.inc in the SPIRV-Tools build directory",
		"{echo} "..buildVersionCmdLine,	buildVersionCmdLine,
		"{echo} Generate tables based on the SPIR-V XML registry",
		"{echo} "..generatorsCmdLine, generatorsCmdLine,
		"{echo} Generate info tables for OpenCL extended instructions and operands v1.0",
		"{echo} "..openclCmdLine, openclCmdLine,
		"{echo} Generate extended instruction tables for spv-amd-gcn-shader",
		"{echo} "..amdGcnCmdLine, amdGcnCmdLine,
		"{echo} Generate extended instruction tables for spv-amd-shader-ballot",
		"{echo} "..amdShaderBallotCmdLine, amdShaderBallotCmdLine,
		"{echo} Generate extended instruction tables for spv-amd-shader-explicit-vertex-parameter",
		"{echo} "..amdExplicitVertexParamCmdLine, amdExplicitVertexParamCmdLine,
		"{echo} Generate extended instruction tables for spv-amd-shader-trinary-minmax",
		"{echo} "..amdTrinaryMinMaxCmdLine, amdTrinaryMinMaxCmdLine,
	}