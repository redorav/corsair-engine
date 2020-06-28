#include "CrRendering_pch.h"

#include "ICrTexture.h"
#include "ICrFramebuffer.h"

#include "Core/CrMacros.h"

ICrFramebuffer::ICrFramebuffer(const CrFramebufferCreateParams&/* params*/)
{
}

ICrFramebuffer::ICrFramebuffer()
{

}

ICrFramebuffer::~ICrFramebuffer()
{

}

CrFramebufferCreateParams::CrFramebufferCreateParams(const ICrTexture* colorTexture, const ICrTexture* depthTexture /*= nullptr*/)
{
	m_colorTargets[0].texture = colorTexture;
	m_depthTarget.texture = depthTexture;
}

CrFramebufferCreateParams::CrFramebufferCreateParams(const CrArray<CrAttachmentProperties, cr3d::MaxRenderTargets>& colorTextures)
{
	m_colorTargets = colorTextures;
}

CrFramebufferCreateParams::CrFramebufferCreateParams(const CrAttachmentProperties& depthTexture)
{
	m_depthTarget = depthTexture;
}

CrFramebufferCreateParams::CrFramebufferCreateParams(const CrArray<CrAttachmentProperties, cr3d::MaxRenderTargets>& colorTextures, const CrAttachmentProperties& depthTexture)
{
	m_colorTargets = colorTextures;
	m_depthTarget = depthTexture;
}
