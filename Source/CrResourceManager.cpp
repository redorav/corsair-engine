#include "CrResourceManager.h"

#include "Rendering/CrImage.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/CrGPUBuffer.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrRenderSystem.h"
#include "Rendering/CrRenderModel.h"

#include "Core/CrCommandLine.h"
#include "Core/FileSystem/CrFileSystem.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Logging/ICrDebug.h"

#include "Image/CrImageDecoderDDS.h"
#include "Image/CrImageDecoderSTB.h"

#include "Model/ICrModelDecoder.h"
#include "Model/CrModelDecoderASSIMP.h"
#include "Model/CrModelDecoderGLTF.h"

CrRenderModelSharedHandle CrResourceManager::LoadModel(const CrPath& relativePath)
{
	CrPath fullPath = CrResourceManager::GetFullResourcePath(relativePath);
	CrFileSharedHandle file = ICrFile::Create(fullPath.string().c_str(), FileOpenFlags::Read);

	CrSharedPtr<ICrModelDecoder> modelDecoder;
	const std::string extension = fullPath.extension().string();
	if (extension == ".gltf" || extension == ".glb")
	{
		modelDecoder = CrMakeShared<CrModelDecoderGLTF>();
	}
	else
	{
		modelDecoder = CrMakeShared<CrModelDecoderASSIMP>();
	}

	CrRenderModelSharedHandle model = modelDecoder->Decode(file);

	model->ComputeBoundingBoxFromMeshes();

	return model;
}

CrPath CrResourceManager::GetFullResourcePath(const CrPath& relativePath)
{
	CrString dataPath = crcore::CommandLine("-root").str().c_str();
	return CrPath(dataPath.c_str()) / relativePath;
}

CrImageHandle CrResourceManager::LoadImageFromDisk(const CrPath& relativePath)
{
	CrPath fullPath = CrResourceManager::GetFullResourcePath(relativePath);
	CrPath extension = fullPath.extension();

	CrFileSharedHandle file = ICrFile::Create(fullPath.string().c_str(), FileOpenFlags::Read);

	CrSharedPtr<ICrImageDecoder> imageDecoder;

	if(CrString(".dds").comparei(extension.string().c_str()) == 0)
	{
		imageDecoder = CrSharedPtr<ICrImageDecoder>(new CrImageDecoderDDS());
	}
	else
	{
		imageDecoder = CrSharedPtr<ICrImageDecoder>(new CrImageDecoderSTB());
	}

	return imageDecoder->Decode(file);
}
