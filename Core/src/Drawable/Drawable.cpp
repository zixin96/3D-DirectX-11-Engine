#include "Drawable.h"
#include "Debug/GraphicsThrowMacros.h"
#include "Bindable/IndexBuffer.h"
#include <cassert>

void Drawable::Draw(Graphics& gfx) const noexcept(!IS_DEBUG)
{
	// bind the bindables (bindables that are unique per instance)
	for (auto& b : binds_)
	{
		b->Bind(gfx);
	}
	// bind static bindables (bindables that are shared among instances)
	for (auto& b : GetStaticBinds())
	{
		b->Bind(gfx);
	}
	gfx.DrawIndexed(pIndexBuffer_->GetCount());
}

// use this when adding anything else
void Drawable::AddBind(std::unique_ptr<Bindable> bind) noexcept(!IS_DEBUG)
{
	assert("*Must* use AddIndexBuffer to bind index buffer" && typeid(*bind) != typeid(IndexBuffer));
	binds_.push_back(std::move(bind));
}

// use this when adding index buffer
void Drawable::AddIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noexcept(!IS_DEBUG)
{
	assert("Attempting to add index buffer a second time" && pIndexBuffer_ == nullptr);
	pIndexBuffer_ = ibuf.get();
	binds_.push_back(std::move(ibuf));
}
