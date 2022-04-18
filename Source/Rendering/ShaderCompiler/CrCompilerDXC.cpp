#include "CrCompilerDXC.h"

// SPIR-V Reflection
#include <spirv_reflect.h>

// DXIL Reflection
#pragma warning(push)
#pragma warning(disable : 5204)
#include <atlbase.h> // Common COM helpers.

// Make sure we use version in dependencies of these two files. Be sure to link with dxcompiler.lib
#include "inc/dxcapi.h"
#include "inc/d3d12shader.h"
#pragma warning(pop)

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
		case cr3d::ShaderStage::RootSignature:  return "rootsig_1_0";
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

void CrCompilerDXC::CreateCommonDXCCommandLine(const CompilationDescriptor& compilationDescriptor, CrFixedString2048& commandLine)
{
	CrString dxcPath = FindDXCPath();

	CrProcessDescriptor processDescriptor;
	commandLine += dxcPath.c_str();
	commandLine += " -O3 -WX ";

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

void InsertResourceIntoHeader(CrShaderReflectionHeader& reflectionHeader, const CrShaderReflectionResource& resource)
{
	switch (resource.type)
	{
		case cr3d::ShaderResourceType::ConstantBuffer:
			reflectionHeader.constantBuffers.push_back(resource);
			break;
		case cr3d::ShaderResourceType::Sampler:
			reflectionHeader.samplers.push_back(resource);
			break;
		case cr3d::ShaderResourceType::Texture:
			reflectionHeader.textures.push_back(resource);
			break;
		case cr3d::ShaderResourceType::RWTexture:
			reflectionHeader.rwTextures.push_back(resource);
			break;
		case cr3d::ShaderResourceType::StorageBuffer:
			reflectionHeader.storageBuffers.push_back(resource);
			break;
		case cr3d::ShaderResourceType::RWStorageBuffer:
			reflectionHeader.rwStorageBuffers.push_back(resource);
			break;
		case cr3d::ShaderResourceType::RWDataBuffer:
			reflectionHeader.rwDataBuffers.push_back(resource);
			break;
	}
}

bool CrCompilerDXC::HLSLtoSPIRV(const CompilationDescriptor& compilationDescriptor, CrString& compilationStatus)
{
	CrProcessDescriptor processDescriptor;
	CreateCommonDXCCommandLine(compilationDescriptor, processDescriptor.commandLine);

	processDescriptor.commandLine += "-spirv ";

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
			CrVector<uint8_t> bytecode;

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
					resource.bytecodeOffset = binding.word_offset.binding * 4; // Turn into a byte offset

					InsertResourceIntoHeader(reflectionHeader, resource);
				}
			}

			const auto ProcessInterfaceVariables = []
			(
				uint32_t variableCount, 
				SpvReflectInterfaceVariable** variables,
				CrVector<CrShaderInterfaceVariable>& interfaceVariables)
			{
				for (uint32_t i = 0; i < variableCount; ++i)
				{
					const SpvReflectInterfaceVariable& spvInterfaceVariable = *variables[i];
					CrShaderInterfaceVariable interfaceVariable;
					if (spvInterfaceVariable.name)
					{
						const char* lastDot = strrchr(spvInterfaceVariable.name, '.');
						interfaceVariable.name = lastDot ? lastDot + 1 : spvInterfaceVariable.name;
					}
					interfaceVariable.type = GetShaderInterfaceType(spvInterfaceVariable);
					interfaceVariable.bindPoint = (uint8_t)spvInterfaceVariable.location;
					interfaceVariables.push_back(interfaceVariable);
				}
			};

			ProcessInterfaceVariables(shaderModule.input_variable_count, shaderModule.input_variables, reflectionHeader.stageInputs);

			ProcessInterfaceVariables(shaderModule.output_variable_count, shaderModule.output_variables, reflectionHeader.stageOutputs);

			if (compilationDescriptor.shaderStage == cr3d::ShaderStage::Compute)
			{
				reflectionHeader.threadGroupSizeX = shaderModule.entry_points[0].local_size.x;
				reflectionHeader.threadGroupSizeY = shaderModule.entry_points[0].local_size.y;
				reflectionHeader.threadGroupSizeZ = shaderModule.entry_points[0].local_size.z;
			}

			// Write reflection header out
			CrWriteFileStream writeFileStream(compilationDescriptor.outputPath.c_str());
			writeFileStream << reflectionHeader;
			writeFileStream << bytecode;
		}

		return true;
	}
}

cr3d::ShaderResourceType::T GetShaderResourceType(const D3D12_SHADER_INPUT_BIND_DESC& resourceBindingDescriptor)
{
	switch (resourceBindingDescriptor.Type)
	{
		case D3D_SIT_CBUFFER:
			return cr3d::ShaderResourceType::ConstantBuffer;
		case D3D_SIT_TEXTURE:
			if (resourceBindingDescriptor.Dimension == D3D_SRV_DIMENSION_BUFFER)
			{
				return cr3d::ShaderResourceType::DataBuffer;
			}
			else
			{
				return cr3d::ShaderResourceType::Texture;
			}
		case D3D_SIT_SAMPLER:
			return cr3d::ShaderResourceType::Sampler;
		case D3D_SIT_UAV_RWTYPED:
			if (resourceBindingDescriptor.Dimension == D3D_SRV_DIMENSION_BUFFER)
			{
				return cr3d::ShaderResourceType::RWDataBuffer;
			}
			else
			{
				return cr3d::ShaderResourceType::RWTexture;
			}
		case D3D_SIT_STRUCTURED:
		case D3D_SIT_BYTEADDRESS:
			return cr3d::ShaderResourceType::StorageBuffer;
		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_UAV_RWBYTEADDRESS:
		case D3D_SIT_UAV_APPEND_STRUCTURED:
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			return cr3d::ShaderResourceType::RWStorageBuffer;
		default:
			return cr3d::ShaderResourceType::Count;
	}
}

cr3d::ShaderInterfaceBuiltinType::T GetShaderInterfaceType(const D3D12_SIGNATURE_PARAMETER_DESC& d3dSignatureParameter)
{
	switch (d3dSignatureParameter.SystemValueType)
	{
		case D3D_NAME_UNDEFINED: return cr3d::ShaderInterfaceBuiltinType::None;
		case D3D_NAME_POSITION: return cr3d::ShaderInterfaceBuiltinType::Position;
		case D3D_NAME_INSTANCE_ID: return cr3d::ShaderInterfaceBuiltinType::InstanceId;
		case D3D_NAME_VERTEX_ID: return cr3d::ShaderInterfaceBuiltinType::VertexId;
		case D3D_NAME_DEPTH: return cr3d::ShaderInterfaceBuiltinType::Depth;
		case D3D_NAME_IS_FRONT_FACE: return cr3d::ShaderInterfaceBuiltinType::IsFrontFace;

		default:
			return cr3d::ShaderInterfaceBuiltinType::None;
	}
}

bool CrCompilerDXC::HLSLtoDXIL(const CompilationDescriptor& compilationDescriptor, CrString& compilationStatus)
{
	CrProcessDescriptor processDescriptor;
	CreateCommonDXCCommandLine(compilationDescriptor, processDescriptor.commandLine);

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

			CrShaderReflectionHeader reflectionHeader;
			reflectionHeader.entryPoint = compilationDescriptor.entryPoint;
			reflectionHeader.shaderStage = compilationDescriptor.shaderStage;

			if (compilationDescriptor.shaderStage != cr3d::ShaderStage::RootSignature)
			{
				CComPtr<IDxcUtils> dxcUtils;
				DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));

				DxcBuffer buffer = { bytecode.data(), bytecode.size(), 0 };

				CComPtr<ID3D12ShaderReflection> pReflection;
				HRESULT hResult = dxcUtils->CreateReflection(&buffer, IID_PPV_ARGS(&pReflection));

				if (hResult != S_OK)
				{
					compilationStatus += "Error creating reflection data\n";
					return false;
				}

				// Get reflection interface
				D3D12_SHADER_DESC shaderDescriptor;
				pReflection->GetDesc(&shaderDescriptor);

				for (uint32_t i = 0; i < shaderDescriptor.BoundResources; ++i)
				{
					D3D12_SHADER_INPUT_BIND_DESC resourceDescriptor;
					pReflection->GetResourceBindingDesc(i, &resourceDescriptor);

					CrShaderReflectionResource resource;
					resource.name = resourceDescriptor.Name;
					resource.type = GetShaderResourceType(resourceDescriptor);
					resource.bindPoint = (uint8_t)resourceDescriptor.BindPoint;
					resource.bytecodeOffset = 0;

					InsertResourceIntoHeader(reflectionHeader, resource);
				}

				const auto ProcessInterfaceVariable = [](const D3D12_SIGNATURE_PARAMETER_DESC& parameterDescriptor, CrVector<CrShaderInterfaceVariable>& interfaceVariables)
				{
					if (parameterDescriptor.ReadWriteMask)
					{
						CrShaderInterfaceVariable interfaceVariable;
						if (parameterDescriptor.SystemValueType == D3D_NAME_UNDEFINED)
						{
							interfaceVariable.name = parameterDescriptor.SemanticName;
						}

						interfaceVariable.type = GetShaderInterfaceType(parameterDescriptor);
						interfaceVariable.bindPoint = (uint8_t)parameterDescriptor.Register;
						interfaceVariables.push_back(interfaceVariable);
					}
				};

				for (uint32_t i = 0; i < shaderDescriptor.InputParameters; ++i)
				{
					D3D12_SIGNATURE_PARAMETER_DESC inputParameterDescriptor;
					pReflection->GetInputParameterDesc(i, &inputParameterDescriptor);
					ProcessInterfaceVariable(inputParameterDescriptor, reflectionHeader.stageInputs);
				}

				for (uint32_t i = 0; i < shaderDescriptor.OutputParameters; ++i)
				{
					D3D12_SIGNATURE_PARAMETER_DESC outputParameterDescriptor;
					pReflection->GetOutputParameterDesc(i, &outputParameterDescriptor);
					ProcessInterfaceVariable(outputParameterDescriptor, reflectionHeader.stageOutputs);
				}

				if (compilationDescriptor.shaderStage == cr3d::ShaderStage::Compute)
				{
					pReflection->GetThreadGroupSize(&reflectionHeader.threadGroupSizeX, &reflectionHeader.threadGroupSizeY, &reflectionHeader.threadGroupSizeZ);
				}
			}

			// Write reflection header out
			CrWriteFileStream writeFileStream(compilationDescriptor.outputPath.c_str());
			writeFileStream << reflectionHeader;
			writeFileStream << bytecode;
		}

		return true;
	}
}