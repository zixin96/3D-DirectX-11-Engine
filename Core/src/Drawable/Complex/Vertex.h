#pragma once
#include "Graphics.h"

// TODO: Come back to this. Architecture is described: https://youtu.be/VAa73H5fwwk

namespace D3DEngine
{
	struct BGRAColorType
	{
		unsigned char a;
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};

	/**
	 * \brief Describes structure of a single vertex
	 */
	class VertexLayout
	{
	public:
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
			static constexpr const char* semantic = "Position";
		};

		template <>
		struct Map<Position3D>
		{
			using SysType = DirectX::XMFLOAT3;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* semantic = "Position";
		};

		template <>
		struct Map<Texture2D>
		{
			using SysType = DirectX::XMFLOAT2;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
			static constexpr const char* semantic = "Texcoord";
		};

		template <>
		struct Map<Normal>
		{
			using SysType = DirectX::XMFLOAT3;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* semantic = "Normal";
		};

		template <>
		struct Map<Float3Color>
		{
			using SysType = DirectX::XMFLOAT3;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* semantic = "Color";
		};

		template <>
		struct Map<Float4Color>
		{
			using SysType = DirectX::XMFLOAT4;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
			static constexpr const char* semantic = "Color";
		};

		template <>
		struct Map<BGRAColor>
		{
			using SysType = BGRAColorType;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			static constexpr const char* semantic = "Color";
		};

		class Element
		{
		public:
			Element(ElementType type, size_t offset)
				:
				type(type),
				offset(offset)
			{
			}

			size_t GetOffsetAfter() const noxnd
			{
				return offset + Size();
			}

			size_t GetOffset() const
			{
				return offset;
			}

			size_t Size() const noxnd
			{
				return SizeOf(type);
			}

			static constexpr size_t SizeOf(ElementType type) noxnd
			{
				switch (type)
				{
				case Position2D:
					// look into the map and get the type
					return sizeof(Map<Position2D>::SysType);
				case Position3D:
					return sizeof(Map<Position3D>::SysType);
				case Texture2D:
					return sizeof(Map<Texture2D>::SysType);
				case Normal:
					return sizeof(Map<Normal>::SysType);
				case Float3Color:
					return sizeof(Map<Float3Color>::SysType);
				case Float4Color:
					return sizeof(Map<Float4Color>::SysType);
				case BGRAColor:
					return sizeof(Map<BGRAColor>::SysType);
				}
				assert("Invalid element type" && false);
				return 0u;
			}

			ElementType GetType() const noexcept
			{
				return type;
			}

			D3D11_INPUT_ELEMENT_DESC GetDesc() const noxnd
			{
				switch (type)
				{
				case Position2D:
					return GenerateDesc<Position2D>(GetOffset());
				case Position3D:
					return GenerateDesc<Position3D>(GetOffset());
				case Texture2D:
					return GenerateDesc<Texture2D>(GetOffset());
				case Normal:
					return GenerateDesc<Normal>(GetOffset());
				case Float3Color:
					return GenerateDesc<Float3Color>(GetOffset());
				case Float4Color:
					return GenerateDesc<Float4Color>(GetOffset());
				case BGRAColor:
					return GenerateDesc<BGRAColor>(GetOffset());
				}
				assert("Invalid element type" && false);
				return {"INVALID", 0, DXGI_FORMAT_UNKNOWN, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0};
			}

		private:
			template <ElementType type>
			static constexpr D3D11_INPUT_ELEMENT_DESC GenerateDesc(size_t offset) noxnd
			{
				return {Map<type>::semantic, 0, Map<type>::dxgiFormat, 0, (UINT)offset, D3D11_INPUT_PER_VERTEX_DATA, 0};
			}

		private:
			ElementType type;
			size_t offset;
		};

	public:
		template <ElementType Type>
		const Element& Resolve() const noxnd
		{
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

		const Element& ResolveByIndex(size_t i) const noxnd
		{
			return elements[i];
		}

		VertexLayout& Append(ElementType type) noxnd
		{
			elements.emplace_back(type, Size());
			return *this;
		}

		size_t Size() const noxnd
		{
			return elements.empty() ? 0u : elements.back().GetOffsetAfter();
		}

		size_t GetElementCount() const noexcept
		{
			return elements.size();
		}

		std::vector<D3D11_INPUT_ELEMENT_DESC> GetD3DLayout() const noxnd
		{
			std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
			desc.reserve(GetElementCount());
			for (const auto& e : elements)
			{
				desc.push_back(e.GetDesc());
			}
			return desc;
		}

	private:
		std::vector<Element> elements;
	};

	/**
	 * \brief Proxy view into the data of a single vertex in the buffer
	 */
	class Vertex
	{
		friend class VertexBuffer;
	public:
		template <VertexLayout::ElementType Type>
		auto& Attr() noxnd
		{
			auto pAttribute = pData + layout.Resolve<Type>().GetOffset();
			return *reinterpret_cast<typename VertexLayout::Map<Type>::SysType*>(pAttribute);
		}

		template <typename T>
		void SetAttributeByIndex(size_t i, T&& val) noxnd
		{
			const auto& element = layout.ResolveByIndex(i);
			auto pAttribute = pData + element.GetOffset();
			switch (element.GetType())
			{
			case VertexLayout::Position2D:
				SetAttribute<VertexLayout::Position2D>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Position3D:
				SetAttribute<VertexLayout::Position3D>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Texture2D:
				SetAttribute<VertexLayout::Texture2D>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Normal:
				SetAttribute<VertexLayout::Normal>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Float3Color:
				SetAttribute<VertexLayout::Float3Color>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Float4Color:
				SetAttribute<VertexLayout::Float4Color>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::BGRAColor:
				SetAttribute<VertexLayout::BGRAColor>(pAttribute, std::forward<T>(val));
				break;
			default:
				assert("Bad element type" && false);
			}
		}

	protected:
		Vertex(char* pData, const VertexLayout& layout) noxnd
			:
			pData(pData),
			layout(layout)
		{
			assert(pData != nullptr);
		}

	private:
		template <typename First, typename ...Rest>
		// enables parameter pack setting of multiple parameters by element index
		void SetAttributeByIndex(size_t i, First&& first, Rest&&... rest) noxnd
		{
			SetAttributeByIndex(i, std::forward<First>(first));
			SetAttributeByIndex(i + 1, std::forward<Rest>(rest)...);
		}

		// helper to reduce code duplication in SetAttributeByIndex
		template <VertexLayout::ElementType DestLayoutType, typename SrcType>
		void SetAttribute(char* pAttribute, SrcType&& val) noxnd
		{
			using Dest = typename VertexLayout::Map<DestLayoutType>::SysType;
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
		char* pData = nullptr;
		const VertexLayout& layout;
	};

	/**
	 * \brief Proxy view that is read-only
	 */
	class ConstVertex
	{
	public:
		ConstVertex(const Vertex& v) noxnd
			:
			vertex(v)
		{
		}

		template <VertexLayout::ElementType Type>
		const auto& Attr() const noxnd
		{
			return const_cast<Vertex&>(vertex).Attr<Type>();
		}

	private:
		Vertex vertex;
	};

	/**
	 * \brief Actual data of vertices
	 */
	class VertexBuffer
	{
	public:
		VertexBuffer(VertexLayout layout) noxnd
			:
			layout(std::move(layout))
		{
		}

		const char* GetData() const noxnd
		{
			return buffer.data();
		}

		const VertexLayout& GetLayout() const noexcept
		{
			return layout;
		}

		// size: in number of vertices
		size_t Size() const noxnd
		{
			return buffer.size() / layout.Size();
		}

		size_t SizeBytes() const noxnd
		{
			return buffer.size();
		}

		// construct a new Vertex in-place at the end of the buffer
		template <typename ...Params>
		void EmplaceBack(Params&&... params) noxnd
		{
			assert(
				sizeof...(params) == layout.GetElementCount() && "Param count doesn't match number of vertex elements");
			buffer.resize(buffer.size() + layout.Size());
			Back().SetAttributeByIndex(0u, std::forward<Params>(params)...);
		}

		Vertex Back() noxnd
		{
			assert(buffer.size() != 0u);
			return Vertex{buffer.data() + buffer.size() - layout.Size(), layout};
		}

		Vertex Front() noxnd
		{
			assert(buffer.size() != 0u);
			return Vertex{buffer.data(), layout};
		}

		Vertex operator[](size_t i) noxnd
		{
			assert(i < Size());
			return Vertex{buffer.data() + layout.Size() * i, layout};
		}

		ConstVertex Back() const noxnd
		{
			return const_cast<VertexBuffer*>(this)->Back();
		}

		ConstVertex Front() const noxnd
		{
			return const_cast<VertexBuffer*>(this)->Front();
		}

		ConstVertex operator[](size_t i) const noxnd
		{
			return const_cast<VertexBuffer&>(*this)[i];
		}

	private:
		// raw buffer data
		std::vector<char> buffer;
		// the layout of the raw buffer data
		VertexLayout layout;
	};
}
