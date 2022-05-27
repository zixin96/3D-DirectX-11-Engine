#include "VertexShader.h"
#include "BindableCodex.h"
#include "Debug/GraphicsThrowMacros.h"

namespace D3DEngine
{
	using namespace std::string_literals;

	VertexShader::VertexShader(Graphics& gfx, const std::string& path)
		:
		path(path)
	{
		INFOMAN(gfx);

		// Reads the shader file that is on disk into memory
		GFX_THROW_INFO(D3DReadFileToBlob(
			               // A pointer to a constant null-terminated (wide) string that contains the name of the file to read into memory
			               // This will work only for ASCII
			               std::wstring{ path.begin(),path.end() }.c_str(),
			               // output handle for the blob
			               &pBytecodeBlob_));


		GFX_THROW_INFO(GetDevice(gfx)->CreateVertexShader(
			               // A pointer to the compiled shader
			               pBytecodeBlob_->GetBufferPointer(),
			               // Size of the compiled vertex shader
			               pBytecodeBlob_->GetBufferSize(),
			               // A pointer to a class linkage interface
			               nullptr,
			               // Address of a pointer to a ID3D11VertexShader interface
			               &pVertexShader_));
	}

	void VertexShader::Bind(Graphics& gfx) noexcept
	{
		// Set a vertex shader to the device
		GetContext(gfx)->VSSetShader(
		                             // Pointer to a vertex shader
		                             pVertexShader_.Get(),
		                             // A pointer to an array of class-instance interfaces
		                             nullptr,
		                             // The number of class-instance interfaces in the array
		                             0u);
	}

	ID3DBlob* VertexShader::GetBytecode() const noexcept
	{
		return pBytecodeBlob_.Get();
	}

	// call this function if you want to create a VertexShader
	// we will have a resolve function per bindable
	std::shared_ptr<VertexShader> VertexShader::Resolve(Graphics& gfx, const std::string& path)
	{
		return Codex::Resolve<VertexShader>(gfx, path);
	}

	/**
	 * \brief Generate a unique ID for this vertex shader
	 * \param path Shader path
	 * \return A UID for this vertex shader
	 */
	std::string VertexShader::GenerateUID(const std::string& path)
	{
		using namespace std::string_literals;
		return typeid(VertexShader).name() + "#"s + path;
	}

	// provide a way to get the UID from an existing bindable (useful if we want to query for specific bindables)
	std::string VertexShader::GetUID() const noexcept
	{
		return GenerateUID(path);
	}
}
