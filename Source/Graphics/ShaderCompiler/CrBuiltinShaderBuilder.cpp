#include "Graphics/ShaderCompiler/CrShaderCompiler_pch.h"

#include "CrShaderCompiler.h"
#include "CrBuiltinShaderBuilder.h"
#include "CrShaderCompilerUtilities.h"

#include "Core/Logging/ICrDebug.h"

#include "crstl/filesystem.h"

// RapidYAML
#define RYML_USE_ASSERT 0
#include <ryml.hpp>

void CrBuiltinShaderBuilder::ProcessBuiltinShaders(const CrBuiltinShadersDescriptor& builtinShadersDescriptor)
{
	crstl::vector<crstl::vector<CrShaderCompilationJob>> compilationJobs;
	compilationJobs.resize(builtinShadersDescriptor.graphicsApis.size());

	crstl::vector<CrShaderInfo> shaderInfos;

	// Find every .shaders file in the specified directory
	crstl::for_each_directory_entry(builtinShadersDescriptor.inputPath.c_str(), true, [&compilationJobs, &builtinShadersDescriptor, &shaderInfos](const crstl::directory_entry& entry)
	{
		// TODO Replace extension with a path_view
		CrFixedPath filename = entry.filename;

		if (filename.extension() == ".shaders")
		{
			CrFixedPath shadersFilePath = CrFixedPath(entry.directory) / entry.filename;

			// Load .shaders file
			crstl::file file = crstl::file(shadersFilePath.c_str(), crstl::file_flags::read);

			// Read in data
			crstl::string shadersFile;
			shadersFile.resize(file.get_size());
			file.read(shadersFile.data(), shadersFile.size());

			// Assume we have an hlsl file with the same name where the entry points defined in the .shaders file live
			CrFixedPath hlslFilePath = shadersFilePath;
			hlslFilePath.replace_extension(".hlsl");

			// Set the error callback to print out 
			c4::yml::Callbacks callbacks;
			callbacks.m_error = [](const char* msg, size_t /*msg_len*/, c4::yml::Location /*location*/, void* /*user_data*/)
			{
				CrLog(msg);
			};

			c4::yml::set_callbacks(callbacks);

			// Parse YAML
			ryml::Tree tree = ryml::parse(ryml::substr(shadersFile.data(), shadersFile.size()));
			ryml::NodeRef root = tree.rootref();

			// For every shader declared in the document, process the yaml and construct a compilation descriptor
			// to process the appropriate shader
			for (ryml::NodeRef shader : root.children())
			{
				const c4::csubstr& shaderKey = shader.key();
				crstl::string shaderName(shaderKey.str, shaderKey.len);

				if (shaderName.find(" ") != shaderName.npos)
				{
					crstl::string errorMessage;
					errorMessage.append_sprintf("Invalid shader name %s. Did you miss a ':'? Make sure YAML syntax is followed", shaderName.c_str());
					CrShaderCompilerUtilities::QuitWithMessage(errorMessage.c_str());
				}

				crstl::string stageName;

				CrShaderCompilationJob compilationJob;
				CompilationDescriptor& compilationDescriptor = compilationJob.compilationDescriptor;
				compilationDescriptor.inputPath = hlslFilePath;
				compilationDescriptor.platform = builtinShadersDescriptor.platform;

				crgfx::ShaderStage::T shaderStage = crgfx::ShaderStage::Count;

				ryml::NodeRef entryPointNode = shader["entrypoint"];
				if (entryPointNode.is_keyval())
				{
					c4::csubstr entryPointValue = entryPointNode.val();
					compilationDescriptor.entryPoint = crstl::string(entryPointValue.str, entryPointValue.len);
				}
				else
				{
					continue;
				}

				ryml::NodeRef stageNode = shader["stage"];
				if (stageNode.is_keyval())
				{
					c4::csubstr stageValue = stageNode.val();
					if (stageValue == "Vertex")        { shaderStage = crgfx::ShaderStage::Vertex; }
					else if (stageValue == "Pixel")    { shaderStage = crgfx::ShaderStage::Pixel; }
					else if (stageValue == "Geometry") { shaderStage = crgfx::ShaderStage::Geometry; }
					else if (stageValue == "Hull")     { shaderStage = crgfx::ShaderStage::Hull; }
					else if (stageValue == "Domain")   { shaderStage = crgfx::ShaderStage::Domain; }
					else if (stageValue == "Compute")  { shaderStage = crgfx::ShaderStage::Compute; }
					else if (stageValue == "RootSignature") { shaderStage = crgfx::ShaderStage::RootSignature; }
					else
					{
						crstl::string errorMessage;
						crstl::string invalidStageString(stageValue.str, stageValue.len);
						errorMessage.append_sprintf("Invalid shader stage '%s' in shader %s. Remember that stage must be upper case", invalidStageString.c_str(), shaderName.c_str());
						CrShaderCompilerUtilities::QuitWithMessage(errorMessage.c_str());
					}

					compilationDescriptor.shaderStage = shaderStage;

					stageName = crgfx::ShaderStage::ToString(shaderStage);
				}
				else
				{
					continue;
				}

				ryml::NodeRef definesNode = shader["defines"];
				if (definesNode.is_keyval())
				{
					const c4::csubstr& defineValue = definesNode.val();
					compilationDescriptor.defines.push_back(crstl::string(defineValue.str, defineValue.len));
				}
				else if (definesNode.is_seq())
				{
					for (const ryml::NodeRef& defineNode : definesNode.children())
					{
						c4::csubstr defineValue = defineNode.val();
						compilationDescriptor.defines.push_back(crstl::string(defineValue.str, defineValue.len));
					}
				}
				else
				{
					// This is a valid case, there are just no defines present
				}

				CrShaderInfo shaderInfo;
				shaderInfo.name = shaderName;
				shaderInfo.pipelineType = shaderStage == crgfx::ShaderStage::Compute ? CrPipelineType::Compute : CrPipelineType::Graphics;
				shaderInfos.push_back(shaderInfo);
				compilationJob.name = shaderName;

				for(uint32_t i = 0; i < builtinShadersDescriptor.graphicsApis.size(); ++i)
				{
					crgfx::GraphicsApi::T graphicsApi = builtinShadersDescriptor.graphicsApis[i];
					
					const char* shaderBinaryExtension = ".bin";

					crstl::string uniqueShaderName =
						shaderName + "_" + 
						compilationDescriptor.entryPoint + "_" + 
						stageName + "_" + 
						crgfx::GraphicsApi::ToString(graphicsApi);

					CrFixedPath binaryFilePath = builtinShadersDescriptor.outputPath;
					binaryFilePath /= uniqueShaderName.c_str();
					binaryFilePath.replace_extension(shaderBinaryExtension);

					CrFixedPath tempPath = builtinShadersDescriptor.outputPath;
					tempPath /= uniqueShaderName.c_str();
					tempPath.replace_extension(".temp");

					compilationDescriptor.uniqueBinaryName = uniqueShaderName + shaderBinaryExtension;
					compilationDescriptor.outputPath = binaryFilePath;
					compilationDescriptor.tempPath = tempPath;
					compilationDescriptor.graphicsApi = graphicsApi;

					compilationJobs[i].push_back(compilationJob);
				}
			}
		}

		return true;
	});

	// Execute all compilation jobs.
	// TODO This part can be multithreaded very easily
	for (const auto& graphicsApiCompilationJobs : compilationJobs)
	{
		for (const CrShaderCompilationJob& compilationJob : graphicsApiCompilationJobs)
		{
			const CompilationDescriptor& compilationDescriptor = compilationJob.compilationDescriptor;

			// Add conditions here under which a job cannot be compiled. We still need to process it to add the entry
			// to the builtin shader table. This means e.g. the Vulkan backend has a dummy entry for a root signature
			// This generally only happens on Desktop, where we support multiple APIs
			bool excludeCompilationJob =
				compilationDescriptor.shaderStage == crgfx::ShaderStage::RootSignature && compilationDescriptor.graphicsApi != crgfx::GraphicsApi::D3D12;

			if (!excludeCompilationJob)
			{
				crstl::string compilationStatus;
				bool success = CrShaderCompiler::Compile(compilationDescriptor, compilationStatus);

				if (!success)
				{
					CrShaderCompilerUtilities::QuitWithMessage(compilationStatus.c_str());
				}
			}
		}
	}
	
	if (builtinShadersDescriptor.buildBuiltinHeaders)
	{
		// Once all compilation jobs are finished and successful, build binary and metadata from them
		BuildBuiltinShaderMetadataAndHeaderFiles(builtinShadersDescriptor, shaderInfos, compilationJobs);
	}
}

void CrBuiltinShaderBuilder::BuildBuiltinShaderMetadataAndHeaderFiles
(
	const CrBuiltinShadersDescriptor& builtinShadersDescriptor, 
	const crstl::vector<CrShaderInfo>& shaderInfos,
	const crstl::vector<crstl::vector<CrShaderCompilationJob>>& compilationJobs
)
{
	crstl::string builtinShadersGenericHeader;
	crstl::string builtinShadersGenericCpp;

	crstl::string builtinShadersGenericHeaderGetFunction;
	crstl::string builtinComputeGenericHeaderGetFunction;

	crstl::string builtinShadersEnum = "namespace CrBuiltinShaders\n{\n\tenum T : uint32_t\n\t{";
	crstl::string builtinComputeShadersEnum = "namespace CrBuiltinCompute\n{\n\tenum T : uint32_t\n\t{";

	uint32_t graphicShaderCount = 0;
	uint32_t computeShaderCount = 0;

	// Add shader entry to enum
	for (const CrShaderInfo& shaderInfo : shaderInfos)
	{
		if (shaderInfo.pipelineType == CrPipelineType::Graphics)
		{
			builtinShadersEnum += "\n\t\t" + shaderInfo.name + ",";
			graphicShaderCount++;
		}
		else
		{
			builtinComputeShadersEnum += "\n\t\t" + shaderInfo.name + ",";
			computeShaderCount++;
		}
	}

	builtinShadersEnum += "\n\t\tCount\n\t};\n};";
	builtinComputeShadersEnum += "\n\t\tCount\n\t};\n};";

	builtinShadersGenericHeader += "#pragma once\n\n";
	builtinShadersGenericHeader += "#include \"Core/CrCoreForwardDeclarations.h\"\n";
	builtinShadersGenericHeader += "#include \"crstl/string.h\"\n";
	builtinShadersGenericHeader += "#include \"Graphics/CrGraphics.h\"\n";
	builtinShadersGenericHeader += "\n";
	builtinShadersGenericHeader += builtinShadersEnum;
	builtinShadersGenericHeader += "\n\n";
	builtinShadersGenericHeader += builtinComputeShadersEnum;
	builtinShadersGenericHeader += "\n\n";

	builtinShadersGenericHeader += "struct CrBuiltinShaderMetadata\n"
		"{\n"
		"\tCrBuiltinShaderMetadata(const crstl::string& name, const crstl::string& entryPoint, const crstl::string& uniqueBinaryName, crgfx::ShaderStage::T shaderStage, uint8_t* shaderCode, uint32_t shaderCodeSize)\n"
		"\t: name(name), entryPoint(entryPoint), uniqueBinaryName(uniqueBinaryName), shaderStage(shaderStage), shaderCode(shaderCode), shaderCodeSize(shaderCodeSize) {}\n\n"
		"\tCrBuiltinShaderMetadata() : CrBuiltinShaderMetadata(\"\", \"\", \"\", crgfx::ShaderStage::Count, nullptr, 0) {}\n\n"
		"\tcrstl::string name;\n"
		"\tcrstl::string entryPoint;\n"
		"\tcrstl::string uniqueBinaryName;\n"
		"\tcrgfx::ShaderStage::T shaderStage;\n"
		"\tuint8_t* shaderCode;\n"
		"\tuint32_t shaderCodeSize;\n"
		"};\n\n";

	builtinShadersGenericHeader += "struct CrBuiltinComputeMetadata\n"
	"{\n"
		"\tCrBuiltinComputeMetadata(const crstl::string& name, const crstl::string& entryPoint, const crstl::string& uniqueBinaryName, uint8_t* shaderCode, uint32_t shaderCodeSize)\n"
		"\t: name(name), entryPoint(entryPoint), uniqueBinaryName(uniqueBinaryName), shaderCode(shaderCode), shaderCodeSize(shaderCodeSize) {}\n\n"
		"\tCrBuiltinComputeMetadata() : CrBuiltinComputeMetadata(\"\", \"\", \"\", nullptr, 0) {}\n\n"
		"\tcrstl::string name;\n"
		"\tcrstl::string entryPoint;\n"
		"\tcrstl::string uniqueBinaryName;\n"
		"\tuint8_t* shaderCode;\n"
		"\tuint32_t shaderCodeSize;\n"
	"};\n\n";

	builtinShadersGenericHeader += "const CrBuiltinShaderMetadata InvalidBuiltinShaderMetadata;\n\n";
	builtinShadersGenericHeader += "const CrBuiltinComputeMetadata InvalidBuiltinComputeMetadata;\n\n";

	builtinShadersGenericHeaderGetFunction += "namespace CrBuiltinShaders\n{\n";
	builtinShadersGenericHeaderGetFunction += "\tinline const CrBuiltinShaderMetadata& GetMetadata(CrBuiltinShaders::T builtinShader, crgfx::GraphicsApi::T graphicsApi)\n\t{\n";

	builtinComputeGenericHeaderGetFunction += "namespace CrBuiltinCompute\n{\n";
	builtinComputeGenericHeaderGetFunction += "\tinline const CrBuiltinComputeMetadata& GetMetadata(CrBuiltinCompute::T builtinCompute, crgfx::GraphicsApi::T graphicsApi)\n\t{\n";
	
	builtinShadersGenericCpp += "#include \"Graphics/CrRendering_pch.h\"\n";
	builtinShadersGenericCpp += "\n";

	for(crgfx::GraphicsApi::T graphicsApi : builtinShadersDescriptor.graphicsApis)
	{
		const char* graphicsApiString = crgfx::GraphicsApi::ToString(graphicsApi);

		const crstl::vector<CrShaderCompilationJob>& graphicsApiCompilationJobs = compilationJobs[graphicsApi];

		builtinShadersGenericHeaderGetFunction += "\t\tif(graphicsApi == crgfx::GraphicsApi::";
		builtinShadersGenericHeaderGetFunction += graphicsApiString;
		builtinShadersGenericHeaderGetFunction += ")\n\t\t{\n";
		builtinShadersGenericHeaderGetFunction += "\t\t\treturn ";
		builtinShadersGenericHeaderGetFunction += graphicsApiString;
		builtinShadersGenericHeaderGetFunction += "::GetMetadata(builtinShader);\n\t\t}\n\n";

		builtinComputeGenericHeaderGetFunction += "\t\tif(graphicsApi == crgfx::GraphicsApi::";
		builtinComputeGenericHeaderGetFunction += graphicsApiString;
		builtinComputeGenericHeaderGetFunction += ")\n\t\t{\n";
		builtinComputeGenericHeaderGetFunction += "\t\t\treturn ";
		builtinComputeGenericHeaderGetFunction += graphicsApiString;
		builtinComputeGenericHeaderGetFunction += "::GetMetadata(builtinCompute);\n\t\t}\n\n";

		crstl::string builtinShadersMetadataTable = "\t\tcrstl::array<CrBuiltinShaderMetadata, " + crstl::string(graphicShaderCount) + "> BuiltinShaderMetadataTable =\n\t\t{\n";
		crstl::string builtinComputeMetadataTable = "\t\tcrstl::array<CrBuiltinComputeMetadata, " + crstl::string(computeShaderCount) + "> BuiltinComputeMetadataTable =\n\t\t{\n";

		crstl::string builtinShaderDataCpp;
		crstl::string builtinComputeDataCpp;

		// Load every file we produced and turn it into metadata (unique shader name, length, shaders source file) plus a C array that contains the binary
		// We need to include all the necessary metadata to be able to recompile it on demand
		for (const auto& shaderJob : graphicsApiCompilationJobs)
		{
			const crstl::string& shaderName = shaderJob.name;
			const CompilationDescriptor& compilationDescriptor = shaderJob.compilationDescriptor;

			// Load binary file
			crstl::string shaderStageString = "crgfx::ShaderStage::";
			shaderStageString += crgfx::ShaderStage::ToString(compilationDescriptor.shaderStage);

			if (crstl::file file = crstl::file(compilationDescriptor.outputPath.c_str(), crstl::file_flags::read))
			{
				uint64_t codeSize = file.get_size();

				crstl::vector<uint8_t> shaderBinaryData;
				shaderBinaryData.resize(codeSize);
				file.read(shaderBinaryData.data(), (uint32_t)shaderBinaryData.size());

				crstl::string shaderBinaryName = "\t\tuint8_t " + shaderName + "ShaderCode[" + crstl::string(codeSize) + "]";

				const auto CopyBytecodeData = [codeSize, &shaderBinaryData](crstl::string& destination)
				{
					for (uint32_t i = 0; i < codeSize; ++i)
					{
						// Every 48 bytes, jump to a new line
						if (i % 48 == 0)
						{
							destination += "\n\t\t\t";
						}

						// Convert byte to text. Don't add spaces or convert to hex, it just bloats the file
						destination += crstl::string(shaderBinaryData[i]) + ",";
					}
				};

				if (compilationDescriptor.shaderStage == crgfx::ShaderStage::Compute)
				{
					builtinComputeDataCpp += shaderBinaryName + " =\n\t\t{";
					builtinComputeMetadataTable +=
						"\t\t\tCrBuiltinComputeMetadata(\"" + shaderName + "\", \"" + compilationDescriptor.entryPoint + "\", \"" + compilationDescriptor.uniqueBinaryName + "\", " +
						shaderName + "ShaderCode, " + crstl::string(codeSize) + "),\n";

					CopyBytecodeData(builtinComputeDataCpp);

					builtinComputeDataCpp += "\n\t\t};\n\n";
				}
				else
				{
					builtinShaderDataCpp += shaderBinaryName + " =\n\t\t{";
					builtinShadersMetadataTable +=
						"\t\t\tCrBuiltinShaderMetadata(\"" + shaderName + "\", \"" + compilationDescriptor.entryPoint + "\", \"" + compilationDescriptor.uniqueBinaryName + "\", " +
						shaderStageString.c_str() + ", " + shaderName + "ShaderCode, " + crstl::string(codeSize) + "),\n";

					CopyBytecodeData(builtinShaderDataCpp);

					builtinShaderDataCpp += "\n\t\t\t};\n\n";
				}
			}
			else
			{
				if (compilationDescriptor.shaderStage == crgfx::ShaderStage::Compute)
				{
					builtinComputeMetadataTable += "\t\t\tCrBuiltinComputeMetadata(\"" + shaderName + "\", \"\", \"" + compilationDescriptor.uniqueBinaryName + "\", nullptr, " + crstl::string(0) + "), \n";
				}
				else
				{
					builtinShadersMetadataTable += "\t\t\tCrBuiltinShaderMetadata(\"" + shaderName + "\", \"\", \"" + compilationDescriptor.uniqueBinaryName + "\", " + 
						shaderStageString.c_str() + ", nullptr, " + crstl::string(0) + "), \n";
				}
			}
		}

		builtinShadersMetadataTable += "\t\t};\n\n";
		builtinComputeMetadataTable += "\t\t};\n\n";

		crstl::string graphicsNamespaceDeclarationBegin;
		graphicsNamespaceDeclarationBegin += "namespace CrBuiltinShaders\n{\n\tnamespace ";
		graphicsNamespaceDeclarationBegin += graphicsApiString;
		graphicsNamespaceDeclarationBegin += "\n\t{\n";

		crstl::string computeNamespaceDeclarationBegin;
		computeNamespaceDeclarationBegin += "namespace CrBuiltinCompute\n{\n\tnamespace ";
		computeNamespaceDeclarationBegin += graphicsApiString;
		computeNamespaceDeclarationBegin += "\n\t{\n";

		crstl::string namespaceDeclarationEnd;
		namespaceDeclarationEnd += "\t}\n}\n";

		crstl::string builtinShadersHeader;
		builtinShadersHeader += "#pragma once\n";
		builtinShadersHeader += "\n";

		builtinShadersHeader += graphicsNamespaceDeclarationBegin;
		builtinShadersHeader += "\t\tconst CrBuiltinShaderMetadata& GetMetadata(CrBuiltinShaders::T builtinShader);\n";
		builtinShadersHeader += namespaceDeclarationEnd;

		builtinShadersHeader += "\n";

		builtinShadersHeader += computeNamespaceDeclarationBegin;
		builtinShadersHeader += "\t\tconst CrBuiltinComputeMetadata& GetMetadata(CrBuiltinCompute::T builtinCompute);\n";
		builtinShadersHeader += namespaceDeclarationEnd;

		crstl::string builtinShadersCpp;
		builtinShadersCpp += "#include \"BuiltinShaders.h\"\n";
		builtinShadersCpp += "#include \"crstl/array.h\"\n";
		builtinShadersCpp += "\n";

		builtinShadersCpp += graphicsNamespaceDeclarationBegin;
		builtinShadersCpp += builtinShaderDataCpp;
		builtinShadersCpp += builtinShadersMetadataTable;
		builtinShadersCpp += "\t\tconst CrBuiltinShaderMetadata& GetMetadata(CrBuiltinShaders::T builtinShader)\n"
		"\t\t{\n"
			"\t\t\treturn BuiltinShaderMetadataTable[builtinShader];\n"
		"\t\t}\n";
		builtinShadersCpp += namespaceDeclarationEnd;

		builtinShadersCpp += "\n";

		builtinShadersCpp += computeNamespaceDeclarationBegin;
		builtinShadersCpp += builtinComputeDataCpp;
		builtinShadersCpp += builtinComputeMetadataTable;
		builtinShadersCpp += "\t\tconst CrBuiltinComputeMetadata& GetMetadata(CrBuiltinCompute::T builtinCompute)\n"
		"\t\t{\n"
			"\t\t\treturn BuiltinComputeMetadataTable[builtinCompute];\n"
		"\t\t}\n";
		builtinShadersCpp += namespaceDeclarationEnd;

		// Create header and cpp filenames
		CrFixedPath headerPath = builtinShadersDescriptor.outputPath + crgfx::GraphicsApi::ToString(graphicsApi);
		headerPath.replace_extension("h");
		CrShaderCompilerUtilities::WriteToFileIfChanged(headerPath.c_str(), builtinShadersHeader);

		builtinShadersGenericHeader += "#include \"";
		builtinShadersGenericHeader += headerPath.filename().c_str();
		builtinShadersGenericHeader += "\"\n";

		CrFixedPath cppPath = builtinShadersDescriptor.outputPath + crgfx::GraphicsApi::ToString(graphicsApi);
		cppPath.replace_extension("cpp");
		CrShaderCompilerUtilities::WriteToFileIfChanged(cppPath.c_str(), builtinShadersCpp);

		builtinShadersGenericCpp += "#include \"";
		builtinShadersGenericCpp += cppPath.filename().c_str();
		builtinShadersGenericCpp += "\"\n";
	}

	builtinShadersGenericHeaderGetFunction += "\t\treturn InvalidBuiltinShaderMetadata;\n\t}\n}\n";
	builtinShadersGenericHeaderGetFunction += "\n";
	builtinComputeGenericHeaderGetFunction += "\t\treturn InvalidBuiltinComputeMetadata;\n\t}\n}\n";

	builtinShadersGenericHeader += "\n";
	builtinShadersGenericHeader += builtinShadersGenericHeaderGetFunction;
	builtinShadersGenericHeader += builtinComputeGenericHeaderGetFunction;

	// Create header and cpp filenames
	CrFixedPath headerPath = builtinShadersDescriptor.outputPath.c_str();
	CrShaderCompilerUtilities::WriteToFileIfChanged(headerPath.replace_extension("h").c_str(), builtinShadersGenericHeader);

	CrFixedPath cppPath = builtinShadersDescriptor.outputPath.c_str();
	CrShaderCompilerUtilities::WriteToFileIfChanged(cppPath.replace_extension("cpp").c_str(), builtinShadersGenericCpp);

	// Write dummy file that tells the build system dependency tracker that files are up to date
	CrFixedPath uptodatePath = builtinShadersDescriptor.outputPath.c_str();
	CrShaderCompilerUtilities::WriteToFile(uptodatePath.replace_extension("uptodate").c_str(), "");
}
