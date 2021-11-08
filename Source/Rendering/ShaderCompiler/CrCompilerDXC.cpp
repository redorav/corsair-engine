#include "CrCompilerDXC.h"

#include <spirv_reflect.h>

#include "CrShaderCompiler.h"

#include "Rendering/CrRendering.h"
#include "Rendering/CrShaderReflectionHeader.h"

#include "Core/Process/CrProcess.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/SmartPointers/CrUniquePtr.h"
#include "Core/Containers/CrArray.h"
#include "Core/CrGlobalPaths.h"
#include "Core/Streams/CrFileStream.h"

const char* GetDXCShaderProfile(cr3d::ShaderStage::T shaderStage)
{
	switch (shaderStage)
	{
		case cr3d::ShaderStage::Vertex:   return "vs_6_0";
		case cr3d::ShaderStage::Geometry: return "gs_6_0";
		case cr3d::ShaderStage::Hull:     return "hs_6_0";
		case cr3d::ShaderStage::Domain:   return "ds_6_0";
		case cr3d::ShaderStage::Pixel:    return "ps_6_0";
		case cr3d::ShaderStage::Compute:  return "cs_6_0";
		default: return "";
	}
}

static CrString FindDXCPath()
{
	CrArray<CrString, 2> candidatePaths = 
	{
		CrShaderCompiler::GetExecutableDirectory() + "DXC/DXC.exe",
		CrGlobalPaths::GetShaderCompilerDirectory() + "DXC/DXC.exe"
	};

	for (const CrString& dxcPath : candidatePaths)
	{
		if (ICrFile::FileExists(dxcPath.c_str()))
		{
			return dxcPath;
		}
	}

	return "";
}

void CrCompilerDXC::CreateCommonCommandLine(const CompilationDescriptor& compilationDescriptor, CrFixedString512& commandLine)
{
	CrString dxcPath = FindDXCPath();

	CrProcessDescriptor processDescriptor;
	commandLine += dxcPath.c_str();
	commandLine += " -O3 ";

	commandLine += "-E ";
	commandLine += compilationDescriptor.entryPoint.c_str();
	commandLine += " ";

	commandLine += "-T ";
	commandLine += GetDXCShaderProfile(compilationDescriptor.shaderStage);
	commandLine += " ";

	commandLine += "-Fo \"";

	// If we're building reflection data, the untouched shader goes to the temporary
	// file, which we load back, modify and write out again
	if (compilationDescriptor.buildReflection)
	{
		commandLine += compilationDescriptor.tempPath.c_str();
	}
	else
	{
		commandLine += compilationDescriptor.outputPath.c_str();
	}

	commandLine += "\" ";

	commandLine += "\"";
	commandLine += compilationDescriptor.inputPath.c_str();
	commandLine += "\" ";

	for (uint32_t i = 0; i < compilationDescriptor.defines.size(); ++i)
	{
		const CrString& define = compilationDescriptor.defines[i];
		commandLine += "-D ";
		commandLine += define.c_str();
		commandLine += " ";
	}
}

cr3d::ShaderResourceType::T GetShaderResourceType(const SpvReflectDescriptorBinding& spvDescriptorBinding)
{
	switch (spvDescriptorBinding.descriptor_type)
	{
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			return cr3d::ShaderResourceType::ConstantBuffer;
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			return cr3d::ShaderResourceType::Texture;
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
			return cr3d::ShaderResourceType::Sampler;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		{
			if (spvDescriptorBinding.resource_type == SPV_REFLECT_RESOURCE_FLAG_UAV)
			{
				return cr3d::ShaderResourceType::RWStorageBuffer;
			}
			else
			{
				return cr3d::ShaderResourceType::StorageBuffer;
			}
		}
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			return cr3d::ShaderResourceType::RWTexture;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			return cr3d::ShaderResourceType::DataBuffer;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			return cr3d::ShaderResourceType::RWDataBuffer;
		default:
			return cr3d::ShaderResourceType::Count;
	}
}

cr3d::ShaderInterfaceBuiltinType::T GetShaderInterfaceType(const SpvReflectInterfaceVariable& spvInterfaceVariable)
{
	switch (spvInterfaceVariable.built_in)
	{
		case SpvBuiltIn::SpvBuiltInPosition: return cr3d::ShaderInterfaceBuiltinType::Position;
		case SpvBuiltIn::SpvBuiltInFragCoord: return cr3d::ShaderInterfaceBuiltinType::Position;
		case SpvBuiltIn::SpvBuiltInBaseInstance: return cr3d::ShaderInterfaceBuiltinType::BaseInstance;
		case SpvBuiltIn::SpvBuiltInInstanceIndex: return cr3d::ShaderInterfaceBuiltinType::InstanceId;
		case SpvBuiltIn::SpvBuiltInVertexIndex: return cr3d::ShaderInterfaceBuiltinType::VertexId;
		case SpvBuiltIn::SpvBuiltInFragDepth: return cr3d::ShaderInterfaceBuiltinType::Depth;
		case SpvBuiltIn::SpvBuiltInFrontFacing: return cr3d::ShaderInterfaceBuiltinType::IsFrontFace;

		case SpvBuiltIn::SpvBuiltInWorkgroupId: return cr3d::ShaderInterfaceBuiltinType::GroupId;
		case SpvBuiltIn::SpvBuiltInLocalInvocationId: return cr3d::ShaderInterfaceBuiltinType::GroupThreadId;
		case SpvBuiltIn::SpvBuiltInLocalInvocationIndex: return cr3d::ShaderInterfaceBuiltinType::GroupIndex;
		case SpvBuiltIn::SpvBuiltInGlobalInvocationId: return cr3d::ShaderInterfaceBuiltinType::DispatchThreadId;

		default:
			return cr3d::ShaderInterfaceBuiltinType::None;
	}
}

bool CrCompilerDXC::HLSLtoSPIRV(const CompilationDescriptor& compilationDescriptor, CrString& compilationStatus)
{
	CrProcessDescriptor processDescriptor;
	CreateCommonCommandLine(compilationDescriptor, processDescriptor.commandLine);

	processDescriptor.commandLine += " -spirv ";

	CrProcess compilerProcess(processDescriptor);
	compilerProcess.Wait();

	if (compilerProcess.GetReturnValue() != 0)
	{
		CrArray<char, 2048> processOutput;
		compilerProcess.ReadStdOut(processOutput.data(), processOutput.size());
		compilationStatus += processOutput.data();
		return false;
	}
	else
	{
		if (compilationDescriptor.buildReflection)
		{
			CrVector<char> bytecode;

			// Read data in
			CrFileUniqueHandle tempFile = ICrFile::OpenUnique(compilationDescriptor.tempPath.c_str(), FileOpenFlags::Read);
			bytecode.resize(tempFile->GetSize());
			tempFile->Read(bytecode.data(), bytecode.size());
			tempFile = nullptr; // Close the file

			// Delete the temporary file
			ICrFile::FileDelete(compilationDescriptor.tempPath.c_str());

			// Create platform-independent reflection data from the platform-specific reflection
			SpvReflectShaderModule shaderModule;
			spvReflectCreateShaderModule(bytecode.size(), bytecode.data(), &shaderModule);

			CrShaderReflectionHeader reflectionHeader;
			reflectionHeader.entryPoint = shaderModule.entry_point_name;
			reflectionHeader.shaderStage = compilationDescriptor.shaderStage;

			for (uint32_t i = 0; i < shaderModule.descriptor_binding_count; ++i)
			{
				const SpvReflectDescriptorBinding& binding = shaderModule.descriptor_bindings[i];

				if (binding.accessed)
				{
					CrShaderReflectionResource resource;
					resource.name = binding.name;
					resource.type = GetShaderResourceType(binding);
					resource.bindPoint = (uint8_t)binding.binding;
					resource.bytecodeOffset = binding.word_offset.binding;
					reflectionHeader.resources.push_back(resource);
				}
			}

			for (uint32_t i = 0; i < shaderModule.input_variable_count; ++i)
			{
				const SpvReflectInterfaceVariable& spvInterfaceVariable = *shaderModule.input_variables[i];
				CrShaderInterfaceVariable interfaceVariable;
				interfaceVariable.name = spvInterfaceVariable.built_in == -1 ? spvInterfaceVariable.name : "";
				interfaceVariable.type = GetShaderInterfaceType(spvInterfaceVariable);
				interfaceVariable.bindPoint = (uint8_t)spvInterfaceVariable.location;
				reflectionHeader.stageInputs.push_back(interfaceVariable);
			}

			for (uint32_t i = 0; i < shaderModule.output_variable_count; ++i)
			{
				const SpvReflectInterfaceVariable& spvInterfaceVariable = *shaderModule.output_variables[i];
				CrShaderInterfaceVariable interfaceVariable;
				interfaceVariable.name = spvInterfaceVariable.built_in == -1 ? spvInterfaceVariable.name : "";
				interfaceVariable.type = GetShaderInterfaceType(spvInterfaceVariable);
				interfaceVariable.bindPoint = (uint8_t)spvInterfaceVariable.location;
				reflectionHeader.stageOutputs.push_back(interfaceVariable);
			}

			if (compilationDescriptor.shaderStage == cr3d::ShaderStage::Compute)
			{
				reflectionHeader.threadGroupSizeX = shaderModule.entry_points[0].local_size.x;
				reflectionHeader.threadGroupSizeY = shaderModule.entry_points[0].local_size.y;
				reflectionHeader.threadGroupSizeZ = shaderModule.entry_points[0].local_size.z;
			}

			// Write reflection header out
			CrWriteFileStream writeFileStream(compilationDescriptor.outputPath.c_str());
			writeFileStream << reflectionHeader;

			// Write bytecode back out
			writeFileStream << bytecode;
		}

		return true;
	}
}

bool CrCompilerDXC::HLSLtoDXIL(const CompilationDescriptor& compilationDescriptor, CrString& compilationStatus)
{
	CrProcessDescriptor processDescriptor;
	CreateCommonCommandLine(compilationDescriptor, processDescriptor.commandLine);

	CrProcess compilerProcess(processDescriptor);
	compilerProcess.Wait();

	if (compilerProcess.GetReturnValue() != 0)
	{
		CrArray<char, 2048> processOutput;
		compilerProcess.ReadStdOut(processOutput.data(), processOutput.size());
		compilationStatus += processOutput.data();
		return false;
	}
	else
	{
		return true;
	}
}