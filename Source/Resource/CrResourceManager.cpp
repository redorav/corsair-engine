#include "Resource/CrResource_pch.h"

#include "CrResourceManager.h"

#include "Rendering/CrImage.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/CrGPUBuffer.h"
#include "Rendering/CrRenderModel.h"

#include "Core/CrCommandLine.h"
#include "Core/Logging/ICrDebug.h"
#include "Core/FileSystem/CrFixedPath.h"

#include "Resource/Image/CrImageCodecDDS.h"
#include "Resource/Image/CrImageCodecSTB.h"
#include "Resource/Image/CrImageCodecWuffs.h"

#include "Resource/Model/ICrModelDecoder.h"
#include "Resource/Model/CrModelDecoderASSIMP.h"
#include "Resource/Model/CrModelDecoderCGLTF.h"
#include "Resource/Model/CrModelDecoderUFBX.h"

CrRenderModelHandle CrResourceManager::LoadModel(const CrFixedPath& fullPath)
{
	crstl::file file(fullPath.c_str(), crstl::file_flags::read);

	if (file)
	{
		crstl::unique_ptr<ICrModelDecoder> modelDecoder;
		CrFixedPath extension = fullPath.extension();
		if (extension.comparei(".gltf") == 0 || extension.comparei(".glb") == 0)
		{
			modelDecoder = crstl::unique_ptr<ICrModelDecoder>(new CrModelDecoderCGLTF());
		}
		else if (extension.comparei(".fbx") == 0 || extension.comparei(".obj") == 0)
		{
			modelDecoder = crstl::unique_ptr<ICrModelDecoder>(new CrModelDecoderUFBX());
		}
		else
		{
			modelDecoder = crstl::unique_ptr<ICrModelDecoder>(new CrModelDecoderASSIMP());
		}

		CrRenderModelHandle model = modelDecoder->Decode(file);

		return model;
	}
	else
	{
		return nullptr;
	}
}

CrFixedPath CrResourceManager::GetFullResourcePath(const CrFixedPath& relativePath)
{
	crstl::string dataPath = crcore::CommandLine("-root").c_str();
	return CrFixedPath(dataPath.c_str()) / relativePath;
}

CrImageHandle CrResourceManager::LoadImageFromDisk(const CrFixedPath& fullPath)
{
	CrFixedPath extension = fullPath.extension();

	crstl::file file(fullPath.c_str(), crstl::file_flags::read);

	crstl::unique_ptr<ICrImageDecoder> imageDecoder;

	if (file)
	{
		if (extension.comparei(".dds") == 0)
		{
			imageDecoder = crstl::unique_ptr<ICrImageDecoder>(new CrImageDecoderDDS());
		}
		else if
		(
			extension.comparei(".png") == 0 ||
			extension.comparei(".tga") == 0 ||
			extension.comparei(".jpg") == 0 ||
			extension.comparei(".jpeg") == 0 ||
			extension.comparei(".gif") == 0 ||
			extension.comparei(".bmp") == 0 ||
			extension.comparei(".wbmp") == 0 ||
			extension.comparei(".nie") == 0
		)
		{
			imageDecoder = crstl::unique_ptr<ICrImageDecoder>(new CrImageDecoderWuffs());
		}
		else
		{
			imageDecoder = crstl::unique_ptr<ICrImageDecoder>(new CrImageDecoderSTB());
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

void CrResourceManager::SaveImageToDisk(const CrImageHandle& image, const CrFixedPath& fullPath)
{
	CrFixedPath extension = fullPath.extension();

	CrWriteFileStream fileStream(fullPath.c_str());

	if (fileStream.GetFile())
	{
		crstl::unique_ptr<ICrImageEncoder> imageEncoder;

		if (extension.comparei(".dds") == 0)
		{
			imageEncoder = crstl::unique_ptr<ICrImageEncoder>(new CrImageEncoderDDS());
		}
		else if(extension.comparei(".png") == 0)
		{
			imageEncoder = crstl::unique_ptr<ICrImageEncoder>(new CrImageEncoderSTB(CrImageContainerFormat::PNG));
		}
		else if (extension.comparei(".jpg") == 0)
		{
			imageEncoder = crstl::unique_ptr<ICrImageEncoder>(new CrImageEncoderSTB(CrImageContainerFormat::JPG));
		}
		else if (extension.comparei(".hdr") == 0)
		{
			imageEncoder = crstl::unique_ptr<ICrImageEncoder>(new CrImageEncoderSTB(CrImageContainerFormat::HDR));
		}
		else if (extension.comparei(".tga") == 0)
		{
			imageEncoder = crstl::unique_ptr<ICrImageEncoder>(new CrImageEncoderSTB(CrImageContainerFormat::TGA));
		}
		else if (extension.comparei(".bmp") == 0)
		{
			imageEncoder = crstl::unique_ptr<ICrImageEncoder>(new CrImageEncoderSTB(CrImageContainerFormat::BMP));
		}

		if (imageEncoder && imageEncoder->IsImageFormatSupported(image->GetFormat()))
		{
			imageEncoder->Encode(image, fileStream);
		}
	}
}