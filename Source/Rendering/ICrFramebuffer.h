#pragma once

#include "Core/Containers/CrArray.h"

class ICrTexture;

struct CrFramebufferDescriptor
{
	struct CrAttachmentProperties
	{
		const ICrTexture* texture = nullptr;
		uint32_t mipMap = 0;
		uint32_t slice = 0;
	};

	CrFramebufferDescriptor(const ICrTexture* colorTexture, const ICrTexture* depthTexture = nullptr);

	CrFramebufferDescriptor(const CrArray<CrAttachmentProperties, cr3d::MaxRenderTargets>& colorTextures);

	CrFramebufferDescriptor(const CrAttachmentProperties& depthTexture);

	CrFramebufferDescriptor(const CrArray<CrAttachmentProperties, cr3d::MaxRenderTargets>& colorTextures, const CrAttachmentProperties& depthTexture);

	CrArray<CrAttachmentProperties, cr3d::MaxRenderTargets> m_colorTargets;

	CrAttachmentProperties m_depthTarget;
};

class ICrFramebuffer
{
public:

	ICrFramebuffer();

	~ICrFramebuffer();

	ICrFramebuffer(const CrFramebufferDescriptor& descriptor);

private:

	uint32_t m_width;
	uint32_t m_height;
};
