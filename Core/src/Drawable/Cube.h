#pragma once
#include "IndexedTriangleList.h"
#include <DirectXMath.h>

class Cube
{
public:
	// plain old cube
	template <class V>
	static IndexedTriangleList<V> Make()
	{
		namespace dx = DirectX;

		constexpr float side = 1.0f / 2.0f;

		std::vector<V> vertices_(8);
		vertices_[0].pos = {-side, -side, -side};
		vertices_[1].pos = {side, -side, -side};
		vertices_[2].pos = {-side, side, -side};
		vertices_[3].pos = {side, side, -side};
		vertices_[4].pos = {-side, -side, side};
		vertices_[5].pos = {side, -side, side};
		vertices_[6].pos = {-side, side, side};
		vertices_[7].pos = {side, side, side};

		return {
			std::move(vertices_), {
				0, 2, 1, 2, 3, 1,
				1, 3, 5, 3, 7, 5,
				2, 6, 3, 3, 6, 7,
				4, 5, 7, 4, 7, 6,
				0, 4, 2, 2, 4, 6,
				0, 1, 4, 1, 5, 4
			}
		};
	}

	// cube with texture coordinates
	template <class V>
	static IndexedTriangleList<V> MakeSkinned()
	{
		namespace dx = DirectX;

		constexpr float side = 1.0f / 2.0f;

		std::vector<V> vertices_(14);

		vertices_[0].pos = {-side, -side, -side};
		vertices_[0].tex = {2.0f / 3.0f, 0.0f / 4.0f};
		vertices_[1].pos = {side, -side, -side};
		vertices_[1].tex = {1.0f / 3.0f, 0.0f / 4.0f};
		vertices_[2].pos = {-side, side, -side};
		vertices_[2].tex = {2.0f / 3.0f, 1.0f / 4.0f};
		vertices_[3].pos = {side, side, -side};
		vertices_[3].tex = {1.0f / 3.0f, 1.0f / 4.0f};
		vertices_[4].pos = {-side, -side, side};
		vertices_[4].tex = {2.0f / 3.0f, 3.0f / 4.0f};
		vertices_[5].pos = {side, -side, side};
		vertices_[5].tex = {1.0f / 3.0f, 3.0f / 4.0f};
		vertices_[6].pos = {-side, side, side};
		vertices_[6].tex = {2.0f / 3.0f, 2.0f / 4.0f};
		vertices_[7].pos = {side, side, side};
		vertices_[7].tex = {1.0f / 3.0f, 2.0f / 4.0f};
		vertices_[8].pos = {-side, -side, -side};
		vertices_[8].tex = {2.0f / 3.0f, 4.0f / 4.0f};
		vertices_[9].pos = {side, -side, -side};
		vertices_[9].tex = {1.0f / 3.0f, 4.0f / 4.0f};
		vertices_[10].pos = {-side, -side, -side};
		vertices_[10].tex = {3.0f / 3.0f, 1.0f / 4.0f};
		vertices_[11].pos = {-side, -side, side};
		vertices_[11].tex = {3.0f / 3.0f, 2.0f / 4.0f};
		vertices_[12].pos = {side, -side, -side};
		vertices_[12].tex = {0.0f / 3.0f, 1.0f / 4.0f};
		vertices_[13].pos = {side, -side, side};
		vertices_[13].tex = {0.0f / 3.0f, 2.0f / 4.0f};

		return {
			std::move(vertices_), {
				0, 2, 1, 2, 3, 1,
				4, 8, 5, 5, 8, 9,
				2, 6, 3, 3, 6, 7,
				4, 5, 7, 4, 7, 6,
				2, 10, 11, 2, 11, 6,
				12, 3, 7, 12, 7, 13
			}
		};
	}

	template <class V>
	static IndexedTriangleList<V> MakeIndependent()
	{
		constexpr float side = 1.0f / 2.0f;

		std::vector<V> vertices_(24);
		vertices_[0].pos = {-side, -side, -side}; // 0 near side
		vertices_[1].pos = {side, -side, -side}; // 1
		vertices_[2].pos = {-side, side, -side}; // 2
		vertices_[3].pos = {side, side, -side}; // 3
		vertices_[4].pos = {-side, -side, side}; // 4 far side
		vertices_[5].pos = {side, -side, side}; // 5
		vertices_[6].pos = {-side, side, side}; // 6
		vertices_[7].pos = {side, side, side}; // 7
		vertices_[8].pos = {-side, -side, -side}; // 8 left side
		vertices_[9].pos = {-side, side, -side}; // 9
		vertices_[10].pos = {-side, -side, side}; // 10
		vertices_[11].pos = {-side, side, side}; // 11
		vertices_[12].pos = {side, -side, -side}; // 12 right side
		vertices_[13].pos = {side, side, -side}; // 13
		vertices_[14].pos = {side, -side, side}; // 14
		vertices_[15].pos = {side, side, side}; // 15
		vertices_[16].pos = {-side, -side, -side}; // 16 bottom side
		vertices_[17].pos = {side, -side, -side}; // 17
		vertices_[18].pos = {-side, -side, side}; // 18
		vertices_[19].pos = {side, -side, side}; // 19
		vertices_[20].pos = {-side, side, -side}; // 20 top side
		vertices_[21].pos = {side, side, -side}; // 21
		vertices_[22].pos = {-side, side, side}; // 22
		vertices_[23].pos = {side, side, side}; // 23

		return {
			std::move(vertices_), {
				0, 2, 1, 2, 3, 1,
				4, 5, 7, 4, 7, 6,
				8, 10, 9, 10, 11, 9,
				12, 13, 15, 12, 15, 14,
				16, 17, 18, 18, 17, 19,
				20, 23, 21, 20, 22, 23
			}
		};
	}

	template <class V>
	static IndexedTriangleList<V> MakeIndependentTextured()
	{
		auto itl = MakeIndependent<V>();

		itl.vertices_[0].tc = {0.0f, 0.0f};
		itl.vertices_[1].tc = {1.0f, 0.0f};
		itl.vertices_[2].tc = {0.0f, 1.0f};
		itl.vertices_[3].tc = {1.0f, 1.0f};
		itl.vertices_[4].tc = {0.0f, 0.0f};
		itl.vertices_[5].tc = {1.0f, 0.0f};
		itl.vertices_[6].tc = {0.0f, 1.0f};
		itl.vertices_[7].tc = {1.0f, 1.0f};
		itl.vertices_[8].tc = {0.0f, 0.0f};
		itl.vertices_[9].tc = {1.0f, 0.0f};
		itl.vertices_[10].tc = {0.0f, 1.0f};
		itl.vertices_[11].tc = {1.0f, 1.0f};
		itl.vertices_[12].tc = {0.0f, 0.0f};
		itl.vertices_[13].tc = {1.0f, 0.0f};
		itl.vertices_[14].tc = {0.0f, 1.0f};
		itl.vertices_[15].tc = {1.0f, 1.0f};
		itl.vertices_[16].tc = {0.0f, 0.0f};
		itl.vertices_[17].tc = {1.0f, 0.0f};
		itl.vertices_[18].tc = {0.0f, 1.0f};
		itl.vertices_[19].tc = {1.0f, 1.0f};
		itl.vertices_[20].tc = {0.0f, 0.0f};
		itl.vertices_[21].tc = {1.0f, 0.0f};
		itl.vertices_[22].tc = {0.0f, 1.0f};
		itl.vertices_[23].tc = {1.0f, 1.0f};

		return itl;
	}
};
