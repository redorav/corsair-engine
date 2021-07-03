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
#include "Core/FileSystem/CrPathString.h"

#include "Image/CrImageDecoderDDS.h"
#include "Image/CrImageDecoderSTB.h"

#include "Model/ICrModelDecoder.h"
#include "Model/CrModelDecoderASSIMP.h"
#include "Model/CrModelDecoderGLTF.h"

CrRenderModelSharedHandle CrResourceManager::LoadModel(const CrPathString& fullPath)
{
	CrFileSharedHandle file = ICrFile::OpenFile(fullPath.c_str(), FileOpenFlags::Read);

	CrSharedPtr<ICrModelDecoder> modelDecoder;
	const std::string extension = fullPath.extension().c_str();
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

CrPathString CrResourceManager::GetFullResourcePath(const CrPathString& relativePath)
{
	CrString dataPath = crcore::CommandLine("-root").c_str();
	return CrPathString(dataPath.c_str()) / relativePath;
}

CrImageHandle CrResourceManager::LoadImageFromDisk(const CrPathString& fullPath)
{
	CrPathString extension = fullPath.extension();

	CrFileSharedHandle file = ICrFile::OpenFile(fullPath.c_str(), FileOpenFlags::Read);

	CrSharedPtr<ICrImageDecoder> imageDecoder;

	if(CrString(".dds").comparei(extension.c_str()) == 0)
	{
		imageDecoder = CrSharedPtr<ICrImageDecoder>(new CrImageDecoderDDS());
	}
	else
	{
		imageDecoder = CrSharedPtr<ICrImageDecoder>(new CrImageDecoderSTB());
	}

	return imageDecoder->Decode(file);
}
