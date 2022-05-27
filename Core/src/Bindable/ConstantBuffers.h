#pragma once
#include "Bindable.h"
#include "Debug/GraphicsThrowMacros.h"
#include "BindableCodex.h"

namespace D3DEngine
{
	// since constant buffer type is defined by us,
	// we template the entire class to make it flexible
	template <typename C>
	class ConstantBuffer : public Bindable
	{
		public:
			// update constant buffer every frame
			void Update(Graphics& gfx, const C& consts)
			{
				INFOMAN(gfx);

				// Provides access to subresource data
				D3D11_MAPPED_SUBRESOURCE msr;
				// Gets a pointer to the data contained in a subresource, and denies the GPU access to that subresource
				GFX_THROW_INFO(GetContext(gfx)->Map(
					               // A pointer to a ID3D11Resource interface
					               pConstantBuffer_.Get(),
					               // Index number of the subresource
					               0u,
					               // A D3D11_MAP-typed value that specifies the CPU's read and write permissions for a resource
					               D3D11_MAP_WRITE_DISCARD,
					               // Flag that specifies what the CPU does when the GPU is busy
					               0u,
					               // output pointer
					               &msr
				               ));

				memcpy(msr.pData, &consts, sizeof(consts));

				// Invalidate the pointer to a resource and reenable the GPU's access to that resource
				GetContext(gfx)->Unmap(pConstantBuffer_.Get(), 0u);
			}

			// create a constant buffer (that binds to slot 0 by default) with initializing data 
			ConstantBuffer(Graphics& gfx, const C& consts, UINT slot = 0u)
				:
				slot_(slot)
			{
				INFOMAN(gfx);

				D3D11_BUFFER_DESC cbd = {
					.ByteWidth = sizeof(consts),
					// dynamic b/c we update constant buffer every frame
					.Usage = D3D11_USAGE_DYNAMIC,
					.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
					// CPU will write to the buffer every frame
					.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
					.MiscFlags = 0u,
					// no need to provide a stride for this constant buffer
					.StructureByteStride = 0u,
				};

				D3D11_SUBRESOURCE_DATA csd = {
					.pSysMem = &consts,
				};

				GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&cbd, &csd, &pConstantBuffer_));
			}


			// create a constant buffer (that binds to slot 0 by default) without initializing it 
			ConstantBuffer(Graphics& gfx, UINT slot = 0u)
				:
				slot_(slot)
			{
				INFOMAN(gfx);

				D3D11_BUFFER_DESC cbd = {
					.ByteWidth = sizeof(C),
					// dynamic b/c we update constant buffer every frame
					.Usage = D3D11_USAGE_DYNAMIC,
					.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
					// CPU will write to the buffer every frame
					.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
					.MiscFlags = 0u,
					// no need to provide a stride for this constant buffer
					.StructureByteStride = 0u,
				};

				GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&cbd, nullptr, &pConstantBuffer_));
			}

		protected:
			Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer_;
			UINT                                 slot_;
	};

	// There are two types of constant buffers: vertex constant buffer, and pixel constant buffer

	template <typename C>
	class VertexConstantBuffer : public ConstantBuffer<C>
	{
		public:
			// import parent namespace to enable us to use parent's constructor to construct the object
			using ConstantBuffer<C>::ConstantBuffer;

			// this function is called inside TransformCbuf::Bind
			void Bind(Graphics& gfx) noexcept override
			{
				// "this->" is necessary in order to access parent's protected variables
				this->GetContext(gfx)->VSSetConstantBuffers(this->slot_, 1u, this->pConstantBuffer_.GetAddressOf());
			}

			static std::shared_ptr<VertexConstantBuffer> Resolve(Graphics& gfx, const C& consts, UINT slot = 0)
			{
				return Codex::Resolve<VertexConstantBuffer>(gfx, consts, slot);
			}

			static std::shared_ptr<VertexConstantBuffer> Resolve(Graphics& gfx, UINT slot = 0)
			{
				return Codex::Resolve<VertexConstantBuffer>(gfx, slot);
			}

			static std::string GenerateUID(const C&, UINT slot)
			{
				return GenerateUID(slot);
			}

			static std::string GenerateUID(UINT slot = 0)
			{
				using namespace std::string_literals;
				// use the name of the constant buffer directly as the UID, b/c every constant buffer templated on different structures will have different names
				// Assume that if we request the constant buffer with the same structure, then you want to share it
				// TODO: Sometimes you don't want to share the constant buffer even though they templated from the same structure. We may need to add a tag to this as well. 
				return typeid(VertexConstantBuffer).name() + "#"s + std::to_string(slot);
			}

			std::string GetUID() const noexcept override
			{
				return GenerateUID(this->slot_);
			}
	};

	template <typename C>
	class PixelConstantBuffer : public ConstantBuffer<C>
	{
		public:
			using ConstantBuffer<C>::ConstantBuffer;

			void Bind(Graphics& gfx) noexcept override
			{
				this->GetContext(gfx)->PSSetConstantBuffers(this->slot_, 1u, this->pConstantBuffer_.GetAddressOf());
			}

			static std::shared_ptr<PixelConstantBuffer> Resolve(Graphics& gfx, const C& consts, UINT slot = 0)
			{
				return Codex::Resolve<PixelConstantBuffer>(gfx, consts, slot);
			}

			static std::shared_ptr<PixelConstantBuffer> Resolve(Graphics& gfx, UINT slot = 0)
			{
				return Codex::Resolve<PixelConstantBuffer>(gfx, slot);
			}

			static std::string GenerateUID(const C&, UINT slot)
			{
				return GenerateUID(slot);
			}

			static std::string GenerateUID(UINT slot = 0)
			{
				using namespace std::string_literals;
				return typeid(PixelConstantBuffer).name() + "#"s + std::to_string(slot);
			}

			std::string GetUID() const noexcept override
			{
				return GenerateUID(this->slot_);
			}
	};
}
