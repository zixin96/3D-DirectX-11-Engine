#pragma once
#include "Drawable/TestObject.h"

class Melon : public TestObject<Melon>
{
public:
	// used in AppAssortment
	Melon(Graphics& gfx, std::mt19937& rng,
	      std::uniform_real_distribution<float>& adist,
	      std::uniform_real_distribution<float>& ddist,
	      std::uniform_real_distribution<float>& odist,
	      std::uniform_real_distribution<float>& rdist,
	      std::uniform_int_distribution<int>& longdist,
	      std::uniform_int_distribution<int>& latdist);
};
