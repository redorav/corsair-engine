#include "Rendering/ShaderCompiler/CrShaderCompiler_pch.h"

#include "CrCompilerDXC.h"

// SPIR-V Reflection
#include <spirv_reflect.h>

// DXIL Reflection
#pragma warning(push)
#pragma warning(disable : 5204)
#include <atlbase.h> // Common COM helpers.

// Make sure we use version in dependencies of these two files
#include "inc/dxcapi.h"
#include "inc/d3d12shader.h"
#pragma warning(pop)

#include "CrShaderCompiler.h"

#include "Rendering/CrRendering.h"
#include "Rendering/CrShaderReflectionHeader.h"

#include "Core/FileSystem/ICrFile.h"
#include "Core/Containers/CrHashMap.h"
#include "Core/Streams/CrFileStream.h"
#include "Core/CrHash.h"

#include "CrShaderCompilerUtilities.h"

#include <atomic>

// DXC Argument handling
static const wchar_t* DXCArgumentWarningsAsErrors  = L"-WX";
//static const wchar_t* DXCArgumentOptimization0     = L"-O0";
//static const wchar_t* DXCArgumentOptimization1     = L"-O1";
//static const wchar_t* DXCArgumentOptimization2     = L"-O2";
static const wchar_t* DXCArgumentOptimization3     = L"-O3";
static const wchar_t* DXCArgumentEnableDebug       = L"-Zi";

//static const wchar_t* DXCArgumentSkipValidation    = L"-Vd";
//static const wchar_t* DXCArgumentSkipOptimizations = L"-Od";

static const wchar_t* DXCArgumentEntryPoint        = L"-E";
static const wchar_t* DXCArgumentShaderProfile     = L"-T";
static const wchar_t* DXCArgumentDefine            = L"-D";
static const wchar_t* DXCArgumentAllResourcesBound = L"-all-resources-bound";

const wchar_t* GetDXCShaderProfile(cr3d::ShaderStage::T shaderStage)
{
	switch (shaderStage)
	{
		case cr3d::ShaderStage::Vertex:   return L"vs_6_0";
		case cr3d::ShaderStage::Geometry: return L"gs_6_0";
		case cr3d::ShaderStage::Hull:     return L"hs_6_0";
		case cr3d::ShaderStage::Domain:   return L"ds_6_0";
		case cr3d::ShaderStage::Pixel:    return L"ps_6_0";
		case cr3d::ShaderStage::Compute:  return L"cs_6_0";
		case cr3d::ShaderStage::RootSignature:  return L"rootsig_1_0";
		default: return L"";
	}
}

// We assign a number of bind points to each shader stage
// We do a tight packing later so the values on them don't matter
// Bindpoints cannot exceed 256 as we store them in 8 bits
static uint32_t GetShaderStageBindpointNamespace(cr3d::ShaderStage::T shaderStage)
{
	// We can have up to 64 simultaneous resources in each stage
	switch (shaderStage)
	{
		case cr3d::ShaderStage::Vertex:   return 0; // 64
		case cr3d::ShaderStage::Pixel:    return 64; // 64
		case cr3d::ShaderStage::Geometry: return 128; // 32
		case cr3d::ShaderStage::Hull:     return 160; // 32
		case cr3d::ShaderStage::Domain:   return 192; // 32
		case cr3d::ShaderStage::Compute:  return 0; // Doesn't clash with any namespace
		default: return 0;
	}
}

class CrDxcIncludeHandler : public IDxcIncludeHandler
{
public:

	CrDxcIncludeHandler(CComPtr<IDxcUtils> dxcUtils) : m_dxcUtils(dxcUtils) {}

	virtual ~CrDxcIncludeHandler() {}

	ULONG STDMETHODCALLTYPE AddRef() override
	{
		return (ULONG)++m_dwRef;
	}

	ULONG STDMETHODCALLTYPE Release() override
	{
		ULONG result = (ULONG)--m_dwRef;
		if (result == 0)
		{
			delete this;
		}

		return result;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID /*iid*/, void** /*ppvObject*/) override
	{
		//return DoBasicQueryInterface<IDxcIncludeHandler>(this, iid, ppvObject);
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override
	{
		auto includeFileIter = m_includeFiles.find(CrWString(pFilename));
		if (includeFileIter == m_includeFiles.end())
		{
			CrString filename;
			filename.append_convert<wchar_t>(pFilename);

			CrFileHandle file = ICrFile::OpenFile(filename.c_str(), FileOpenFlags::Read);

			if (file)
			{
				CrString includeString;
				includeString.resize(file->GetSize());
				file->Read(includeString.data(), (uint32_t)includeString.size());

				CComPtr<IDxcBlobEncoding> dxcIncludeBlob;
				m_dxcUtils->CreateBlob(includeString.data(), (uint32_t)includeString.size(), 0, &dxcIncludeBlob);
				m_includeFiles.insert({ CrWString(pFilename), dxcIncludeBlob });

				*ppIncludeSource = dxcIncludeBlob;
				(*ppIncludeSource)->AddRef();
			}
			else
			{
				return E_FAIL;
			}
		}
		else
		{
			*ppIncludeSource = includeFileIter->second;
			(*ppIncludeSource)->AddRef();
		}

		return S_OK;
	}

private:

	CrHashMap<CrWString, CComPtr<IDxcBlobEncoding>> m_includeFiles;

	CComPtr<IDxcUtils> m_dxcUtils;

	volatile std::atomic<uint32_t> m_dwRef = 0;
};

// This code was adapted from spirv_reflect utils/stripper.cpp
// Removes the PDB instructions
bool SpvStripDebugData(CrVector<uint8_t>& spirvData)
{
	// https://www.khronos.org/registry/SPIR-V/specs/unified1/SPIRV.pdf
	const uint32_t kHeaderLength = 5;
	const uint32_t kMagicNumber = 0x07230203u;
	const uint32_t kExtensionOpcode = 10;
	const uint32_t kSourceContinuedOpcode = 2;
	const uint32_t kSourceOpcode = 3;
	const uint32_t kStringOpcode = 7;
	const uint32_t kLineOpcode = 8;
	const uint32_t kModuleProcessedOpcode = 330;
	const uint32_t kDecorateIdOpcode = 332;
	const uint32_t kDecorateStringOpcode = 5632;
	const uint32_t kMemberDecorateStringOpcode = 5633;
	const uint32_t kCounterBufferDecoration = 5634;
	const uint32_t kUserTypeGOOGLEDecoration = 5636;

	// Treat original data as uint32_t as that is more natural for SPIR-V handling
	uint32_t* data = (uint32_t*)spirvData.data();
	size_t length = spirvData.size() / 4;

	// Make sure we at least have a header and the magic number is correct
	if (!data || length < kHeaderLength || data[0] != kMagicNumber)
	{
		return false;
	}

	CrVector<uint32_t> intermediateSpirv;
	intermediateSpirv.reserve(length);

	for (uint32_t i = 0; i < kHeaderLength; ++i)
	{
		intermediateSpirv.push_back(data[i]);
	}

	for (uint32_t pos = kHeaderLength; pos < length;)
	{
		const uint32_t inst_len = (data[pos] >> 16);
		const uint32_t opcode = data[pos] & 0x0000ffffu;

		bool skip = false;
		if
		(
			opcode == kDecorateStringOpcode ||
			opcode == kMemberDecorateStringOpcode ||
			opcode == kSourceContinuedOpcode ||
			opcode == kSourceOpcode ||
			opcode == kStringOpcode ||
			opcode == kLineOpcode ||
			opcode == kModuleProcessedOpcode
		)
		{
			skip = true;
		}
		else if (opcode == kDecorateIdOpcode)
		{
			if (pos + 2 >= length)
			{
				return false;
			}
		
			if (data[pos + 2] == kCounterBufferDecoration || data[pos + 2] == kUserTypeGOOGLEDecoration)
			{
				skip = true;
			}
		}
		else if (opcode == kExtensionOpcode)
		{
			if (pos + 1 >= length)
			{
				return false;
			}
		
			const char* ext_name = reinterpret_cast<const char*>(&data[pos + 1]);
			if (0 == std::strcmp(ext_name, "SPV_GOOGLE_decorate_string") ||
				0 == std::strcmp(ext_name, "SPV_GOOGLE_hlsl_functionality1") ||
				0 == std::strcmp(ext_name, "SPV_GOOGLE_user_type"))
			{
				skip = true;
			}
		}

		if (!skip)
		{
			for (uint32_t i = 0; i < inst_len; ++i)
			{
				intermediateSpirv.push_back(data[pos + i]);
			}
		}

		pos += inst_len;
	}

	spirvData.resize(intermediateSpirv.size() * 4);

	memcpy(spirvData.data(), intermediateSpirv.data(), spirvData.size());

	return true;
}

// Compile a given shader using the dxcompiler API
HRESULT CrDXCCompileShader
(
	const CompilationDescriptor& compilationDescriptor,
	const CComPtr<IDxcCompiler3>& dxcCompiler,
	const CComPtr<CrDxcIncludeHandler>& dxcIncludeHandler,
	CComPtr<IDxcResult>& dxcCompilationResult
)
{
	// Read in source code
	CrVector<uint8_t> sourceCode;
	CrFileUniqueHandle sourceCodeFile = ICrFile::OpenUnique(compilationDescriptor.inputPath.c_str(), FileOpenFlags::Read);
	sourceCode.resize(sourceCodeFile->GetSize());
	sourceCodeFile->Read(sourceCode.data(), sourceCode.size());
	sourceCodeFile = nullptr;

	DxcBuffer sourceCodeBuffer = { sourceCode.data(), (uint32_t)sourceCode.size(), 0 };

	// Add command line parameters. These are the same as the ones DXC uses
	CrWString wInputPath;
	wInputPath.append_convert(compilationDescriptor.inputPath.c_str());
	CrWString wEntryPoint;
	wEntryPoint.append_convert(compilationDescriptor.entryPoint.c_str());

	CrVector<const wchar_t*> arguments =
	{
		DXCArgumentWarningsAsErrors, // Warnings as errors
		DXCArgumentOptimization3, // Add full optimizations
		DXCArgumentEnableDebug, // Add debug data (for PDBs)
		DXCArgumentAllResourcesBound, // Assume all resources are bound correctly
		wInputPath.c_str(),
		DXCArgumentShaderProfile,
		GetDXCShaderProfile(compilationDescriptor.shaderStage),
		DXCArgumentEntryPoint,
		wEntryPoint.c_str()
	};

	// Set Vulkan-specific options here
	if (compilationDescriptor.graphicsApi == cr3d::GraphicsApi::Vulkan)
	{
		arguments.push_back(L"-spirv");
	}

	// Include defines here
	CrVector<CrWString> wDefines;
	wDefines.resize(compilationDescriptor.defines.size());

	for (uint32_t i = 0; i < compilationDescriptor.defines.size(); ++i)
	{
		wDefines[i].append_convert(compilationDescriptor.defines[i]);
		arguments.push_back(DXCArgumentDefine);
		arguments.push_back(wDefines[i].c_str());
	}

	return dxcCompiler->Compile(&sourceCodeBuffer, arguments.data(), (uint32_t)arguments.size(), dxcIncludeHandler, IID_PPV_ARGS(&dxcCompilationResult));
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
		default: // TODO Handle Data Buffers
			break;
	}
}

bool CrCompilerDXC::HLSLtoSPIRV(const CompilationDescriptor& compilationDescriptor, CrString& compilationStatus)
{
	CComPtr<IDxcUtils> dxcUtils;
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));

	CComPtr<IDxcCompiler3> dxcCompiler;
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));

	CComPtr<CrDxcIncludeHandler> dxcIncludeHandler = new CrDxcIncludeHandler(dxcUtils);
	
	CComPtr<IDxcResult> dxcCompilationResult;
	HRESULT compilationHResult = CrDXCCompileShader(compilationDescriptor, dxcCompiler, dxcIncludeHandler, dxcCompilationResult);
	
	HRESULT hResult;
	dxcCompilationResult->GetStatus(&hResult);
	
	if (compilationHResult != S_OK || hResult != S_OK)
	{
		IDxcBlobUtf8* errorMessage = nullptr;
		dxcCompilationResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errorMessage), nullptr);
	
		if (errorMessage)
		{
			compilationStatus += errorMessage->GetStringPointer();
		}
	
		return false;
	}
	else
	{
		CComPtr<IDxcBlob> compilationResultBlob;
		dxcCompilationResult->GetResult(&compilationResultBlob);
	
		CrVector<uint8_t> bytecode
		(
			(uint8_t*)compilationResultBlob->GetBufferPointer(),
			(uint8_t*)compilationResultBlob->GetBufferPointer() + compilationResultBlob->GetBufferSize()
		);
	
		CrHash shaderHash = CrHash(bytecode.data(), bytecode.size());
	
		// Create platform-independent reflection data from the platform-specific reflection
		SpvReflectShaderModule shaderModule;
		SpvReflectResult reflectResult = spvReflectCreateShaderModule(bytecode.size(), bytecode.data(), &shaderModule);
	
		if (reflectResult != SPV_REFLECT_RESULT_SUCCESS)
		{
			compilationStatus += "Failed shader reflection";
			return false;
		}

		CrShaderReflectionHeader reflectionHeader;
		reflectionHeader.entryPoint = shaderModule.entry_point_name;
		reflectionHeader.shaderStage = compilationDescriptor.shaderStage;
		reflectionHeader.bytecodeHash = shaderHash.GetHash();
	
		// Remap the assigned binding points to the ones for each stage
		uint32_t currentBindPoint = GetShaderStageBindpointNamespace(compilationDescriptor.shaderStage);
	
		for (uint32_t i = 0; i < shaderModule.descriptor_binding_count; ++i)
		{
			const SpvReflectDescriptorBinding& binding = shaderModule.descriptor_bindings[i];
	
			if (binding.accessed)
			{
				uint32_t remappedBinding = currentBindPoint;
	
				CrShaderReflectionResource resource;
				resource.name = binding.name;
				resource.type = GetShaderResourceType(binding);
	
				// Register the binding and make sure the shader knows about it too
				resource.bindPoint = (uint8_t)remappedBinding;
	
				*((uint32_t*)&(bytecode.data()[binding.word_offset.binding * 4])) = remappedBinding;
	
				InsertResourceIntoHeader(reflectionHeader, resource);
	
				currentBindPoint++;
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
	
		// The PDB mechanism for SPIR-V is not the same as DXIL because SPIR-V doesn't have a defined PDB format
		// Renderdoc recommends having two copies, one with debug information, the other without, and creating an
		// association at module load time
		{
			CrFixedPath pdbFilePath = CrShaderCompiler::GetPDBDirectory(compilationDescriptor.platform, compilationDescriptor.graphicsApi);
			pdbFilePath /= CrString(shaderHash.GetHash()).c_str();
			pdbFilePath.replace_extension(".pdb");
			CrFileHandle pdbFile = ICrFile::OpenFile(pdbFilePath.c_str(), FileOpenFlags::ForceCreate | FileOpenFlags::Write);
			if (pdbFile)
			{
				pdbFile->Write(bytecode.data(), bytecode.size());
			}
		}
	
		// This doesn't strip the reflection data we need to create the reflection header
		SpvStripDebugData(bytecode);

		// Write reflection header and shader bytecode
		CrWriteFileStream writeFileStream(compilationDescriptor.outputPath.c_str());
		writeFileStream << reflectionHeader;
		writeFileStream << bytecode;
	
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
	CComPtr<IDxcUtils> dxcUtils;
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));

	CComPtr<IDxcCompiler3> dxcCompiler;
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	
	CComPtr<CrDxcIncludeHandler> dxcIncludeHandler = new CrDxcIncludeHandler(dxcUtils);
	
	CComPtr<IDxcResult> dxcCompilationResult;
	HRESULT compilationHResult = CrDXCCompileShader(compilationDescriptor, dxcCompiler, dxcIncludeHandler, dxcCompilationResult);

	HRESULT hResult;
	dxcCompilationResult->GetStatus(&hResult);

	if (compilationHResult != S_OK || hResult != S_OK)
	{
		IDxcBlobUtf8* errorMessage = nullptr;
		dxcCompilationResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errorMessage), nullptr);

		if (errorMessage)
		{
			compilationStatus += errorMessage->GetStringPointer();
		}

		return false;
	}
	else
	{
		CComPtr<IDxcBlob> shaderBlob = nullptr;
		CComPtr<IDxcBlobUtf16> shaderName = nullptr;
		dxcCompilationResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), &shaderName);

		CrVector<uint8_t> bytecode
		(
			(uint8_t*)shaderBlob->GetBufferPointer(),
			(uint8_t*)shaderBlob->GetBufferPointer() + shaderBlob->GetBufferSize()
		);

		CrShaderReflectionHeader reflectionHeader;
		reflectionHeader.entryPoint = compilationDescriptor.entryPoint;
		reflectionHeader.shaderStage = compilationDescriptor.shaderStage;
		//reflectionHeader.bytecodeHash = shaderHash.GetHash();

		if (compilationDescriptor.shaderStage != cr3d::ShaderStage::RootSignature)
		{
			// Store PDB in PDB directory assigned by DXC. It's a hash PIX and renderdoc know how to find
			CComPtr<IDxcBlob> pdbBlob = nullptr;
			CComPtr<IDxcBlobUtf16> pdbName = nullptr;
			dxcCompilationResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pdbBlob), &pdbName);

			if (pdbBlob)
			{
				IDxcBlobUtf8* pdbNameAsUtf8 = nullptr;
				dxcUtils->GetBlobAsUtf8(pdbName, &pdbNameAsUtf8);

				const CrFixedPath& pdbDirectory = CrShaderCompiler::GetPDBDirectory(compilationDescriptor.platform, compilationDescriptor.graphicsApi);
				CrFixedPath pdbFilePath = pdbDirectory / pdbNameAsUtf8->GetStringPointer();

				CrFileHandle pdbFile = ICrFile::OpenFile(pdbFilePath.c_str(), FileOpenFlags::ForceCreate | FileOpenFlags::Write);
				if (pdbFile)
				{
					pdbFile->Write(pdbBlob->GetBufferPointer(), pdbBlob->GetBufferSize());
				}
			}

			DxcBuffer buffer = { bytecode.data(), bytecode.size(), 0 };

			CComPtr<ID3D12ShaderReflection> pReflection;
			hResult = dxcUtils->CreateReflection(&buffer, IID_PPV_ARGS(&pReflection));

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

		return true;
	}
}