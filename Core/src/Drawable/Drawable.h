#pragma once
#include "Graphics.h"

class Bindable;
class IndexBuffer;

class Drawable
{
	template <class T>
	friend class DrawableCommon;
public:
	Drawable() = default;
	virtual ~Drawable() = default;
	Drawable(const Drawable&) = delete;
	virtual DirectX::XMMATRIX GetTransformXM() const noexcept = 0;
	virtual void Update(float dt) noexcept {};
	void Draw(Graphics& gfx) const noxnd;
protected:
	template <class T>
	T* QueryBindable() noexcept
	{
		for (auto& pb : binds_)
		{
			if (auto pt = dynamic_cast<T*>(pb.get()))
			{
				return pt;
			}
		}
		return nullptr;
	}

	// unique pointer pass-by-value means we need to std::move it
	void AddBind(std::unique_ptr<Bindable> bind) noxnd;
	void AddIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noxnd;
private:
	// to be implemented in DrawableCommon. This function helps us retrieve static binds needed in Drawable::Draw
	virtual const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds() const noexcept = 0;

	// we need a handle to our index buffer to get the index count
	const IndexBuffer* pIndexBuffer_ = nullptr;

	// Drawable contains a list of bindable objects
	std::vector<std::unique_ptr<Bindable>> binds_;
};
