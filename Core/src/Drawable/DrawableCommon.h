#pragma once
#include "Drawable.h"
#include "Bindable/IndexBuffer.h"

/**
 * \brief 
 * \tparam T 
 */
template <class T>
class DrawableBase : public Drawable
{
protected:
	static bool IsStaticInitialized() noexcept
	{
		// assume every valid Drawable has at least one static bindable
		return !staticBinds_.empty();
	}

	static void AddStaticBind(std::unique_ptr<Bindable> bind) noxnd
	{
		assert("*Must* use AddStaticIndexBuffer to bind index buffer" && typeid(*bind) != typeid(IndexBuffer));
		staticBinds_.push_back(std::move(bind));
	}

	// this function is not static b/c we need to access per-instance index buffer pointer
	void AddStaticIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noxnd
	{
		assert("Attempting to add index buffer a second time" && pIndexBuffer_ == nullptr);
		pIndexBuffer_ = ibuf.get();
		staticBinds_.push_back(std::move(ibuf));
	}

	// this function is used to set the index buffer pointer of the instances created after the first instance
	void SetIndexFromStatic() noxnd
	{
		assert("Attempting to add index buffer a second time" && pIndexBuffer_ == nullptr);
		for (const auto& b : staticBinds_)
		{
			if (const auto p = dynamic_cast<IndexBuffer*>(b.get()))
			{
				pIndexBuffer_ = p;
				return;
			}
		}
		assert("Failed to find index buffer in static binds" && pIndexBuffer_ != nullptr);
	}

private:
	const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds() const noexcept override
	{
		return staticBinds_;
	}

	// static bindable are those that are shared among instances of a drawable
	static std::vector<std::unique_ptr<Bindable>> staticBinds_;
};

template <class T>
std::vector<std::unique_ptr<Bindable>> DrawableBase<T>::staticBinds_;
