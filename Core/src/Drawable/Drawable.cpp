#include "Drawable.h"
#include "Debug/GraphicsThrowMacros.h"
#include "Bindable/IndexBuffer.h"

namespace D3DEngine
{
	void Drawable::Draw(Graphics& gfx) const noxnd
	{
		// bind the bindables (bindables that are unique per instance)
		for (auto& b : binds_)
		{
			b->Bind(gfx);
		}
		gfx.DrawIndexed(pIndexBuffer_->GetCount());
	}

	void Drawable::AddBind(std::shared_ptr<Bindable> bind) noxnd
	{
		// special case for index buffer
		if (typeid(*bind) == typeid(IndexBuffer))
		{
			assert("Binding multiple index buffers not allowed" && pIndexBuffer_ == nullptr);
			pIndexBuffer_ = &static_cast<IndexBuffer&>(*bind);
		}
		binds_.push_back(std::move(bind));
	}
}
