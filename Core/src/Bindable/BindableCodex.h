#pragma once
#include "Bindable.h"

namespace D3DEngine
{
	/**
	 * \brief A singleton central repository for all our binadable
	 */
	class Codex
	{
		public:
			/**
			 * \brief Query for a bindable of type T and see if it exists in the central repository.
			 * It the bindable exists, return it. O.W. it will create a new one in the repo and return it.
			 * \tparam T Type of the bindable
			 * \tparam Params zero or more arguments passing into the constructors of the bindables
			 * \param gfx Graphics object
			 * \param p Parameter packs for arguments passing into the constructors of the bindables
			 * \return A shared pointer to the bindable
			 */
			template <class T, typename...Params>
			static std::shared_ptr<T> Resolve(Graphics& gfx, Params&&...p) noxnd
			{
				static_assert(std::is_base_of<Bindable, T>::value, "Can only resolve classes derived from Bindable");
				return Get().Resolve_<T>(gfx, std::forward<Params>(p)...);
			}

		private:
			template <class T, typename...Params>
			std::shared_ptr<T> Resolve_(Graphics& gfx, Params&&...p) noxnd
			{
				// generate a unique ID based on the type T
				const auto key = T::GenerateUID(std::forward<Params>(p)...);

				// does the bindable exist in the repo? 
				const auto i = binds.find(key);

				if (i == binds.end())
				{
					// create a new bindable
					auto bind  = std::make_shared<T>(gfx, std::forward<Params>(p)...);
					binds[key] = bind;
					// add one reference to this bindable in our repo
					return bind;
				}

				// The bindable does exist the repo. Return a copy of the shared pointer.
				// We now have one additional shared pointer referring to the same bindable.
				// PS: we cast the generic bindable pointer to our specific type T before returning it to the user
				return std::static_pointer_cast<T>(i->second);
			}

			static Codex& Get()
			{
				static Codex codex;
				return codex;
			}

		private:
			// Every bindable has a unique string ID, which we use to lookup bindable to see if we have identical bindables so that we can share them
			std::unordered_map<std::string, std::shared_ptr<Bindable>> binds;

			// Note: Codex itself will hold 1 ref count to the bindable. Thus, you would expect to see (number of expected references + 1)
			// TODO: if you want to clean up bindables, it would need to write a garbage collector that loops through the bindable with ref == 1 and delete them
	};
}
