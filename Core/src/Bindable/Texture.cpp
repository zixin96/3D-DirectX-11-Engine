#include "Texture.h"

#include "BindableCodex.h"
#include "Utils/Surface.h"
#include "Debug/GraphicsThrowMacros.h"

namespace D3DEngine
{
	namespace wrl = Microsoft::WRL;

	Texture::Texture(Graphics& gfx, const std::string& path, UINT slot)
		: path_(path),
		  slot_(slot)
	{
		INFOMAN(gfx);

		// load surface
		const auto s = Surface::FromFile(path);
		hasAlpha     = s.AlphaLoaded();

		// TODO: consider using a staging texture

		// Describes a 2D texture
		D3D11_TEXTURE2D_DESC textureDesc = {
			.Width = s.GetWidth(),
			.Height = s.GetHeight(),
			.MipLevels = 0, // set to 0: want all mipmap levels, all the way down to 1x1
			.ArraySize = 1,
			.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
			.SampleDesc = {.Count = 1, .Quality = 0}, // no AA
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, // bind it as resource to the pipeline used by shaders + allow it to be bound as render target (generating mipmaps means GPU needs to be able to write to this texture)
			.CPUAccessFlags = 0,                                                // CPU won't touch it
			.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS                      // flags for generating mipmaps
		};

		// initialize with data
		// D3D11_SUBRESOURCE_DATA sd = {};
		// sd.pSysMem                = s.GetBufferPtr();
		// sd.SysMemPitch            = s.GetWidth() * sizeof(Surface::Color); // distance in bytes between the first pixel in row 0 and the first pixel in row 1 

		wrl::ComPtr<ID3D11Texture2D> pTexture;
		GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(
			               &textureDesc,
			               nullptr, // we do not need to initialize texture with subresource (if using mipmaping)
			               &pTexture
		               ));

		// since we are no longer providing a subresource when creating the texture, we need to update it ourselves
		// manually write original image data into top/first mip level
		GetContext(gfx)->UpdateSubresource(pTexture.Get(),
		                                   0u, // write data to top level 
		                                   nullptr,
		                                   s.GetBufferPtrConst(),
		                                   s.GetWidth() * sizeof(Surface::Color),
		                                   0u); // for 3d data

		// create the resource view on the texture
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format                          = textureDesc.Format;
		srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip       = 0;
		srvDesc.Texture2D.MipLevels             = -1; // use all the mip levels

		GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
			               pTexture.Get(), &srvDesc, &pTextureView_
		               ));

		// generate the mipmap chain using the gpu rendering pipeline
		GetContext(gfx)->GenerateMips(pTextureView_.Get());
	}

	void Texture::Bind(Graphics& gfx) noexcept
	{
		// textures are used by pixel shaders
		GetContext(gfx)->PSSetShaderResources(slot_, 1u, pTextureView_.GetAddressOf());
	}

	std::shared_ptr<Texture> Texture::Resolve(Graphics& gfx, const std::string& path, UINT slot)
	{
		return Codex::Resolve<Texture>(gfx, path, slot);
	}

	std::string Texture::GenerateUID(const std::string& path, UINT slot)
	{
		using namespace std::string_literals;
		// a texture is uniquely defined by its the name, path, and slot
		return typeid(Texture).name() + "#"s + path + "#" + std::to_string(slot);
	}

	std::string Texture::GetUID() const noexcept
	{
		return GenerateUID(path_, slot_);
	}

	bool Texture::HasAlpha() const noexcept
	{
		return hasAlpha;
	}
}
