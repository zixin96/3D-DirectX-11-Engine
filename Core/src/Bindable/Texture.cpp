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

		// Describes a 2D texture
		D3D11_TEXTURE2D_DESC textureDesc = {
			.Width = s.GetWidth(),
			.Height = s.GetHeight(),
			.MipLevels = 1,
			.ArraySize = 1,
			.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
			.SampleDesc = {.Count = 1, .Quality = 0}, // no AA
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_SHADER_RESOURCE, // bind it as resource to the pipeline used by shaders
			.CPUAccessFlags = 0,                     // CPU won't touch it
			.MiscFlags = 0,
		};

		// initialize with data
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem                = s.GetBufferPtr();
		sd.SysMemPitch            = s.GetWidth() * sizeof(Surface::Color); // distance in bytes between the first pixel in row 0 and the first pixel in row 1 

		wrl::ComPtr<ID3D11Texture2D> pTexture;
		GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(
			               &textureDesc,
			               &sd,
			               &pTexture
		               ));

		// create the resource view on the texture
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format                          = textureDesc.Format;
		srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip       = 0;
		srvDesc.Texture2D.MipLevels             = 1;

		GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
			               pTexture.Get(), &srvDesc, &pTextureView_
		               ));
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
