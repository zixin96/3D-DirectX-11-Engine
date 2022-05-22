#include "VertexShader.h"
#include "Debug/GraphicsThrowMacros.h"


VertexShader::VertexShader(Graphics& gfx, const std::wstring& path)
{
	INFOMAN(gfx);

	// Reads the shader file that is on disk into memory
	GFX_THROW_INFO(D3DReadFileToBlob(
		// A pointer to a constant null-terminated (wide) string that contains the name of the file to read into memory
		path.c_str(),
		// output handle for the blob
		&pBytecodeBlob_ ));

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
