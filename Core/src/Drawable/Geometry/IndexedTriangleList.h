#pragma once
#include "Drawable/Complex/VertexView.h"
#include <DirectXMath.h>

namespace D3DEngine
{
	class IndexedTriangleList
	{
		public:
			IndexedTriangleList() = default;

			IndexedTriangleList(RawVertexBufferWithLayout verts_in, std::vector<unsigned short> indices_in)
				:
				vertices(std::move(verts_in)),
				indices(std::move(indices_in))
			{
				assert(vertices.Size() > 2);
				assert(indices.size() % 3 == 0);
			}

			void Transform(DirectX::FXMMATRIX matrix)
			{
				using Elements = DynamicVertexLayout::ElementType;
				for (int i = 0; i < vertices.Size(); i++)
				{
					auto& pos = vertices[i].Attr<Elements::Position3D>();
					DirectX::XMStoreFloat3(
					                       &pos,
					                       DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&pos), matrix)
					                      );
				}
			}

		public:
			RawVertexBufferWithLayout   vertices;
			std::vector<unsigned short> indices;
	};
}
