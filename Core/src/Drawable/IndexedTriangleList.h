#pragma once
#include <vector>
#include <DirectXMath.h>

template <class T>
class IndexedTriangleList
{
public:
	IndexedTriangleList() = default;

	IndexedTriangleList(std::vector<T> verts_in, std::vector<unsigned short> indices_in)
		:
		vertices_(std::move(verts_in)),
		indices_(std::move(indices_in))
	{
		assert(vertices_.size() > 2);
		assert(indices_.size() % 3 == 0);
	}

	void Transform(DirectX::FXMMATRIX matrix)
	{
		for (auto& v : vertices_)
		{
			const DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&v.pos);
			DirectX::XMStoreFloat3(
				&v.pos,
				DirectX::XMVector3Transform(pos, matrix)
			);
		}
	}

	void SetNormalsIndependentFlat() noexcept(!IS_DEBUG)
	{
		using namespace DirectX;
		assert(indices_.size() % 3 == 0 && indices_.size() > 0);
		for (size_t i = 0; i < indices_.size(); i += 3)
		{
			auto& v0 = vertices_[indices_[i]];
			auto& v1 = vertices_[indices_[i + 1]];
			auto& v2 = vertices_[indices_[i + 2]];
			const auto p0 = XMLoadFloat3(&v0.pos);
			const auto p1 = XMLoadFloat3(&v1.pos);
			const auto p2 = XMLoadFloat3(&v2.pos);

			const auto n = XMVector3Normalize(XMVector3Cross((p1 - p0), (p2 - p0)));

			XMStoreFloat3(&v0.n, n);
			XMStoreFloat3(&v1.n, n);
			XMStoreFloat3(&v2.n, n);
		}
	}

	std::vector<T> vertices_;
	std::vector<unsigned short> indices_;
};
