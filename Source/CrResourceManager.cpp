#include "CrResourceManager.h"

#include "Rendering/CrImage.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/CrGPUBuffer.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrRenderSystem.h"
#include "Rendering/CrRenderModel.h"

#include "Core/CrCommandLine.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Logging/ICrDebug.h"
#include "Core/FileSystem/CrPath.h"

#include "Image/CrImageCodecDDS.h"
#include "Image/CrImageCodecSTB.h"

#include "Model/ICrModelDecoder.h"
#include "Model/CrModelDecoderASSIMP.h"
#include "Model/CrModelDecoderGLTF.h"

CrRenderModelSharedHandle CrResourceManager::LoadModel(const CrPath& fullPath)
{
	CrFileSharedHandle file = ICrFile::OpenFile(fullPath.c_str(), FileOpenFlags::Read);

	if (file)
	{
		CrSharedPtr<ICrModelDecoder> modelDecoder;
		CrPath extension = fullPath.extension();
		if (CrFixedString8(".gltf").comparei(extension.c_str()) == 0 || CrFixedString8(".glb").comparei(extension.c_str()) == 0)
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
	else
	{
		return nullptr;
	}
}

CrPath CrResourceManager::GetFullResourcePath(const CrPath& relativePath)
{
	CrString dataPath = crcore::CommandLine("-root").c_str();
	return CrPath(dataPath.c_str()) / relativePath;
}

CrImageHandle CrResourceManager::LoadImageFromDisk(const CrPath& fullPath)
{
	CrPath extension = fullPath.extension();

	CrFileSharedHandle file = ICrFile::OpenFile(fullPath.c_str(), FileOpenFlags::Read);

	CrSharedPtr<ICrImageDecoder> imageDecoder;

	if (file)
	{
		if (extension.comparei(".dds") == 0)
		{
			imageDecoder = CrSharedPtr<ICrImageDecoder>(new CrImageDecoderDDS());
		}
		else
		{
			imageDecoder = CrSharedPtr<ICrImageDecoder>(new CrImageDecoderSTB());
		}
	}

	if (imageDecoder)
	{
		return imageDecoder->Decode(file);
	}
	else
	{
		return nullptr;
	}
}

void CrResourceManager::SaveImageToDisk(const CrImageHandle& image, const CrPath& fullPath)
{
	CrPath extension = fullPath.extension();

	CrFileSharedHandle file = ICrFile::OpenFile(fullPath.c_str(), FileOpenFlags::ForceCreate | FileOpenFlags::Write);

	if (file)
	{
		CrSharedPtr<ICrImageEncoder> imageEncoder;

		if (extension.comparei(".dds") == 0)
		{
			imageEncoder = CrSharedPtr<ICrImageEncoder>(new CrImageEncoderDDS());
		}
		else if(extension.comparei(".png") == 0)
		{
			imageEncoder = CrSharedPtr<ICrImageEncoder>(new CrImageEncoderSTB(CrImageContainerFormat::PNG));
		}
		else if (extension.comparei(".jpg") == 0)
		{
			imageEncoder = CrSharedPtr<ICrImageEncoder>(new CrImageEncoderSTB(CrImageContainerFormat::JPG));
		}
		else if (extension.comparei(".hdr") == 0)
		{
			imageEncoder = CrSharedPtr<ICrImageEncoder>(new CrImageEncoderSTB(CrImageContainerFormat::HDR));
		}
		else if (extension.comparei(".tga") == 0)
		{
			imageEncoder = CrSharedPtr<ICrImageEncoder>(new CrImageEncoderSTB(CrImageContainerFormat::TGA));
		}
		else if (extension.comparei(".bmp") == 0)
		{
			imageEncoder = CrSharedPtr<ICrImageEncoder>(new CrImageEncoderSTB(CrImageContainerFormat::BMP));
		}

		if (imageEncoder && imageEncoder->IsImageFormatSupported(image->GetFormat()))
		{
			imageEncoder->Encode(image, file);
		}
	}
}