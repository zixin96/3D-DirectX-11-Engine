#pragma once
#include <vector>
#include <type_traits>
#include "Graphics.h"
#include "Color.h"

namespace D3DEngine
{
	class DynamicVertexLayout
	{
		public:
			/**
			 * \brief Describes typical elements in vertices
			 */
			enum ElementType
			{
				Position2D,
				Position3D,
				// TODO: what if there are two sets of Texture coordinates? Current system doesn't support it.
				Texture2D,
				Normal,
				Float3Color,
				Float4Color,
				BGRAColor,
				Count,
			};

			// template specialization
			// compile-time lookup table
			template <ElementType>
			struct Map;

			template <>
			struct Map<Position2D>
			{
				using SysType = DirectX::XMFLOAT2;
				static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
				static constexpr const char* semantic   = "Position";
				static constexpr const char* code       = "P2"; // this code is used to identify a particular element type
			};

			template <>
			struct Map<Position3D>
			{
				using SysType = DirectX::XMFLOAT3;
				static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
				static constexpr const char* semantic   = "Position";
				static constexpr const char* code       = "P3";
			};

			template <>
			struct Map<Texture2D>
			{
				using SysType = DirectX::XMFLOAT2;
				static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
				static constexpr const char* semantic   = "TexCoord";
				static constexpr const char* code       = "T2";
			};

			template <>
			struct Map<Normal>
			{
				using SysType = DirectX::XMFLOAT3;
				static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
				static constexpr const char* semantic   = "Normal";
				static constexpr const char* code       = "N";
			};

			template <>
			struct Map<Float3Color>
			{
				using SysType = DirectX::XMFLOAT3;
				static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
				static constexpr const char* semantic   = "Color";
				static constexpr const char* code       = "C3";
			};

			template <>
			struct Map<Float4Color>
			{
				using SysType = DirectX::XMFLOAT4;
				static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
				static constexpr const char* semantic   = "Color";
				static constexpr const char* code       = "C4";
			};

			template <>
			struct Map<BGRAColor>
			{
				using SysType = ::BGRAColor;
				static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
				static constexpr const char* semantic   = "Color";
				static constexpr const char* code       = "C8";
			};

			class Element
			{
				public:
					Element(ElementType type, size_t offset);
					size_t                   GetOffsetAfter() const noxnd;
					size_t                   GetOffset() const;
					size_t                   Size() const noxnd;
					static constexpr size_t  SizeOf(ElementType type) noxnd;
					ElementType              GetType() const noexcept;
					D3D11_INPUT_ELEMENT_DESC GetDesc() const noxnd;
					const char*              GetCode() const noexcept;
				private:
					template <ElementType type>
					static constexpr D3D11_INPUT_ELEMENT_DESC GenerateDesc(size_t offset) noexcept
					{
						return {Map<type>::semantic, 0, Map<type>::dxgiFormat, 0, (UINT)offset, D3D11_INPUT_PER_VERTEX_DATA, 0};
					}

				private:
					// each element has a type
					ElementType type;
					// number of bytes from the beginning of the vertex
					size_t offset;
			};

		public:
			// resolve an element given its type
			template <ElementType Type>
			const Element& Resolve() const noxnd
			{
				// find the matched type
				for (auto& e : elements)
				{
					if (e.GetType() == Type)
					{
						return e;
					}
				}
				assert("Could not resolve element type" && false);
				return elements.front();
			}

			const Element&                        ResolveByIndex(size_t i) const noxnd;
			DynamicVertexLayout&                  Append(ElementType type) noxnd;
			size_t                                Size() const noxnd;
			size_t                                GetElementCount() const noexcept;
			std::vector<D3D11_INPUT_ELEMENT_DESC> GetD3DLayout() const noxnd;
			std::string                           GetCode() const noxnd;
		private:
			std::vector<Element> elements;
	};

	/**
	 * \brief Vertex is a proxy (view) into a vertex in the vertex buffer
	 */
	class VertexView
	{
		friend class RawVertexBufferWithLayout;
		public:
			// access attribute based on its element type
			template <DynamicVertexLayout::ElementType Type>
			auto& Attr() noxnd
			{
				auto pAttribute = pData + layout.Resolve<Type>().GetOffset();
				return *reinterpret_cast<typename DynamicVertexLayout::Map<Type>::SysType*>(pAttribute);
			}

			template <typename T>
			void SetAttributeByIndex(size_t i, T&& val) noxnd
			{
				const auto& element    = layout.ResolveByIndex(i);
				auto        pAttribute = pData + element.GetOffset();
				switch (element.GetType())
				{
					case DynamicVertexLayout::Position2D:
						// "perfect forwarding"
						SetAttribute<DynamicVertexLayout::Position2D>(pAttribute, std::forward<T>(val));
						break;
					case DynamicVertexLayout::Position3D:
						SetAttribute<DynamicVertexLayout::Position3D>(pAttribute, std::forward<T>(val));
						break;
					case DynamicVertexLayout::Texture2D:
						SetAttribute<DynamicVertexLayout::Texture2D>(pAttribute, std::forward<T>(val));
						break;
					case DynamicVertexLayout::Normal:
						SetAttribute<DynamicVertexLayout::Normal>(pAttribute, std::forward<T>(val));
						break;
					case DynamicVertexLayout::Float3Color:
						SetAttribute<DynamicVertexLayout::Float3Color>(pAttribute, std::forward<T>(val));
						break;
					case DynamicVertexLayout::Float4Color:
						SetAttribute<DynamicVertexLayout::Float4Color>(pAttribute, std::forward<T>(val));
						break;
					case DynamicVertexLayout::BGRAColor:
						SetAttribute<DynamicVertexLayout::BGRAColor>(pAttribute, std::forward<T>(val));
						break;
					default:
						assert("Bad element type" && false);
				}
			}

		protected:
			VertexView(char* pData, const DynamicVertexLayout& layout) noxnd;
		private:
			// enables parameter pack setting of multiple parameters by element index
			template <typename First, typename ...Rest>
			void SetAttributeByIndex(size_t i, First&& first, Rest&&...rest) noxnd
			{
				SetAttributeByIndex(i, std::forward<First>(first));
				SetAttributeByIndex(i + 1, std::forward<Rest>(rest)...);
			}

			// helper to reduce code duplication in SetAttributeByIndex
			template <DynamicVertexLayout::ElementType DestLayoutType, typename SrcType>
			void SetAttribute(char* pAttribute, SrcType&& val) noxnd
			{
				using Dest = typename DynamicVertexLayout::Map<DestLayoutType>::SysType;
				if constexpr (std::is_assignable<Dest, SrcType>::value)
				{
					*reinterpret_cast<Dest*>(pAttribute) = val;
				}
				else
				{
					assert("Parameter attribute type mismatch" && false);
				}
			}

		private:
			char*                      pData = nullptr;
			const DynamicVertexLayout& layout;
	};

	/**
	 * \brief Proxy view that is read-only
	 */
	class ConstVertexView
	{
		public:
			ConstVertexView(const VertexView& v) noxnd;

			template <DynamicVertexLayout::ElementType Type>
			const auto& Attr() const noxnd
			{
				return const_cast<VertexView&>(vertex).Attr<Type>();
			}

		private:
			VertexView vertex;
	};

	class RawVertexBufferWithLayout
	{
		public:
			RawVertexBufferWithLayout(DynamicVertexLayout layout) noxnd;
			const char*                GetData() const noxnd;
			const DynamicVertexLayout& GetLayout() const noexcept;
			size_t                     Size() const noxnd;
			size_t                     SizeBytes() const noxnd;

			// construct a new Vertex in-place at the end of the buffer
			template <typename ...Params>
			void EmplaceBack(Params&&...params) noxnd
			{
				assert(sizeof...(params) == layout.GetElementCount() && "Param count doesn't match number of vertex elements");
				buffer.resize(buffer.size() + layout.Size());
				Back().SetAttributeByIndex(0u, std::forward<Params>(params)...);
			}

			// The following Front/Back/operator[] returns a vertex view into the buffer

			VertexView      Back() noxnd;
			VertexView      Front() noxnd;
			VertexView      operator[](size_t i) noxnd;
			ConstVertexView Back() const noxnd;
			ConstVertexView Front() const noxnd;
			ConstVertexView operator[](size_t i) const noxnd;
		private:
			// raw buffer data as vector of bytes
			std::vector<char> buffer;
			// the layout that describes the structure of the vertices
			DynamicVertexLayout layout;
	};
}
