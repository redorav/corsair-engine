#include "CrModelDecoderGLTF.h"

#include "Core/FileSystem/CrFileSystem.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Containers/CrPair.h"

#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/CrGPUBuffer.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrMesh.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/CrImage.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include <string.h>

CrRenderModelSharedHandle CrModelDecoderGLTF::Decode(const CrFileSharedHandle& file)
{
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	
	/*
	typedef bool (*LoadImageDataFunction)(Image *, const int, std::string *,
                                      std::string *, int, int,
                                      const unsigned char *, int,
                                      void *user_pointer);

	return (*LoadImageData)(image, image_idx, err, warn, 0, 0, &img.at(0),
			static_cast<int>(img.size()), load_image_user_data);
	*/
	/*
	loader.SetImageLoader(
		[]( tinygltf::Image* image, const int imageIndex, std::string* error, std::string* warning,
			int requestedWidth, int requestedHeight, const unsigned char* data, int dataSize, void* userData)
		{
			return true;
		}, nullptr);
		*/

	bool binaryGLTF = strstr(file->GetFilePath(), ".glb") != nullptr;
	CrPath filePath(file->GetFilePath());
	std::string parentPath = filePath.parent_path().string();
	
	// Load it:
	{
		// Read contents of the file:
		uint64_t fileSize = file->GetSize();
		void* fileData = malloc(fileSize);
		file->Read(fileData, fileSize);

		std::string error, warning;
		bool success = false;
		if (binaryGLTF)
		{
			//loader.LoadBinaryFromMemory()
		}
		else
		{
			success = loader.LoadASCIIFromString(&model, &error, &warning, (const char*)fileData, (unsigned int)fileSize, parentPath);
		}
		free(fileData); // Free this now.

		// Check for errors:
		if (!success)
		{
			CrLog("Failed to load:%s (%s)", file->GetFilePath(), error.c_str());
			return nullptr;
		}
		if (!warning.empty())
		{
			CrLog("Warning when loading:%s (%s)", file->GetFilePath(), warning.c_str());
		}
	}
	


	if (file)
		return nullptr;
	return nullptr;



}