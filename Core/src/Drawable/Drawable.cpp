#include "Drawable.h"
#include "Debug/GraphicsThrowMacros.h"
#include "Bindable/IndexBuffer.h"

void Drawable::Draw(Graphics& gfx) const noxnd
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
void Drawable::AddBind(std::unique_ptr<Bindable> bind) noxnd
{
	assert("*Must* use AddIndexBuffer to bind index buffer" && typeid(*bind) != typeid(IndexBuffer));
	binds_.push_back(std::move(bind));
}

// use this when adding index buffer
void Drawable::AddIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noxnd
{
	assert("Attempting to add index buffer a second time" && pIndexBuffer_ == nullptr);
	pIndexBuffer_ = ibuf.get();
	binds_.push_back(std::move(ibuf));
}
