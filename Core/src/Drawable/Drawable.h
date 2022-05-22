#pragma once
#include "Graphics.h"
#include <DirectXMath.h>

class Bindable;

class Drawable
{
	template <class T>
	friend class DrawableBase;
public:
	Drawable() = default;
	virtual ~Drawable() = default;

	Drawable(const Drawable&) = delete;

	virtual DirectX::XMMATRIX GetTransformXM() const noexcept = 0;
	virtual void Update(float dt) noexcept = 0;

	void Draw(Graphics& gfx) const noexcept(!IS_DEBUG);
protected:
	// unique pointer pass-by-value means we need to std::move it
	void AddBind(std::unique_ptr<Bindable> bind) noexcept(!IS_DEBUG);
	void AddIndexBuffer(std::unique_ptr<class IndexBuffer> ibuf) noexcept(!IS_DEBUG);
private:
	virtual const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds() const noexcept = 0;

	// we need a handle to our index buffer to get the index count
	const IndexBuffer* pIndexBuffer_ = nullptr;

	// Drawable contains a list of bindable objects
	std::vector<std::unique_ptr<Bindable>> binds_;
};