#include "VertexView.h"

namespace D3DEngine
{
	// VertexLayout
	// resolve an element based on its index
	const DynamicVertexLayout::Element& DynamicVertexLayout::ResolveByIndex(size_t i) const noxnd
	{
		return elements[i];
	}

	// append an element to the layout
	DynamicVertexLayout& DynamicVertexLayout::Append(ElementType type) noxnd
	{
		elements.emplace_back(type, Size());
		return *this;
	}

	size_t DynamicVertexLayout::Size() const noxnd
	{
		return elements.empty() ? 0u : elements.back().GetOffsetAfter();
	}

	size_t DynamicVertexLayout::GetElementCount() const noexcept
	{
		return elements.size();
	}

	std::vector<D3D11_INPUT_ELEMENT_DESC> DynamicVertexLayout::GetD3DLayout() const noxnd
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
		desc.reserve(GetElementCount());
		for (const auto& e : elements)
		{
			desc.push_back(e.GetDesc());
		}
		return desc;
	}

	std::string DynamicVertexLayout::GetCode() const noxnd
	{
		std::string code;
		for (const auto& e : elements)
		{
			code += e.GetCode();
		}
		return code;
	}


	// VertexLayout::Element
	DynamicVertexLayout::Element::Element(ElementType type, size_t offset)
		:
		type(type),
		offset(offset)
	{
	}

	// return offset into the vertex after the current element
	size_t DynamicVertexLayout::Element::GetOffsetAfter() const noxnd
	{
		return offset + Size();
	}

	size_t DynamicVertexLayout::Element::GetOffset() const
	{
		return offset;
	}

	size_t DynamicVertexLayout::Element::Size() const noxnd
	{
		return SizeOf(type);
	}

	constexpr size_t DynamicVertexLayout::Element::SizeOf(ElementType type) noxnd
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

	DynamicVertexLayout::ElementType DynamicVertexLayout::Element::GetType() const noexcept
	{
		return type;
	}

	const char* DynamicVertexLayout::Element::GetCode() const noexcept
	{
		switch (type)
		{
			case Position2D:
				return Map<Position2D>::code;
			case Position3D:
				return Map<Position3D>::code;
			case Texture2D:
				return Map<Texture2D>::code;
			case Normal:
				return Map<Normal>::code;
			case Float3Color:
				return Map<Float3Color>::code;
			case Float4Color:
				return Map<Float4Color>::code;
			case BGRAColor:
				return Map<BGRAColor>::code;
		}
		assert("Invalid element type" && false);
		return "Invalid";
	}

	D3D11_INPUT_ELEMENT_DESC DynamicVertexLayout::Element::GetDesc() const noxnd
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


	// Vertex
	VertexView::VertexView(char* pData, const DynamicVertexLayout& layout) noxnd
		:
		pData(pData),
		layout(layout)
	{
		assert(pData != nullptr);
	}

	ConstVertexView::ConstVertexView(const VertexView& v) noxnd
		:
		vertex(v)
	{
	}


	// VertexBuffer
	RawVertexBufferWithLayout::RawVertexBufferWithLayout(DynamicVertexLayout layout) noxnd
		:
		layout(std::move(layout))
	{
	}

	const char* RawVertexBufferWithLayout::GetData() const noxnd
	{
		return buffer.data();
	}

	const DynamicVertexLayout& RawVertexBufferWithLayout::GetLayout() const noexcept
	{
		return layout;
	}

	// size: in number of vertices
	size_t RawVertexBufferWithLayout::Size() const noxnd
	{
		return buffer.size() / layout.Size();
	}

	size_t RawVertexBufferWithLayout::SizeBytes() const noxnd
	{
		return buffer.size();
	}

	VertexView RawVertexBufferWithLayout::Back() noxnd
	{
		assert(buffer.size() != 0u);
		return VertexView{buffer.data() + buffer.size() - layout.Size(), layout};
	}

	VertexView RawVertexBufferWithLayout::Front() noxnd
	{
		assert(buffer.size() != 0u);
		return VertexView{buffer.data(), layout};
	}

	VertexView RawVertexBufferWithLayout::operator[](size_t i) noxnd
	{
		assert(i < Size());
		return VertexView{buffer.data() + layout.Size() * i, layout};
	}

	ConstVertexView RawVertexBufferWithLayout::Back() const noxnd
	{
		return const_cast<RawVertexBufferWithLayout*>(this)->Back();
	}

	ConstVertexView RawVertexBufferWithLayout::Front() const noxnd
	{
		return const_cast<RawVertexBufferWithLayout*>(this)->Front();
	}

	ConstVertexView RawVertexBufferWithLayout::operator[](size_t i) const noxnd
	{
		return const_cast<RawVertexBufferWithLayout&>(*this)[i];
	}
}
