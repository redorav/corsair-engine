#include "CrRendering_pch.h"

#include "ICrTexture.h"
#include "ICrFramebuffer.h"

#include "Core/CrMacros.h"

ICrFramebuffer::ICrFramebuffer(const CrFramebufferDescriptor&/* params*/)
{
}

ICrFramebuffer::ICrFramebuffer()
{

}

ICrFramebuffer::~ICrFramebuffer()
{

}

CrFramebufferDescriptor::CrFramebufferDescriptor(const ICrTexture* colorTexture, const ICrTexture* depthTexture /*= nullptr*/)
{
	m_colorTargets[0].texture = colorTexture;
	m_depthTarget.texture = depthTexture;
}

CrFramebufferDescriptor::CrFramebufferDescriptor(const CrArray<CrAttachmentProperties, cr3d::MaxRenderTargets>& colorTextures)
{
	m_colorTargets = colorTextures;
}

CrFramebufferDescriptor::CrFramebufferDescriptor(const CrAttachmentProperties& depthTexture)
{
	m_depthTarget = depthTexture;
}

CrFramebufferDescriptor::CrFramebufferDescriptor(const CrArray<CrAttachmentProperties, cr3d::MaxRenderTargets>& colorTextures, const CrAttachmentProperties& depthTexture)
{
	m_colorTargets = colorTextures;
	m_depthTarget = depthTexture;
}
