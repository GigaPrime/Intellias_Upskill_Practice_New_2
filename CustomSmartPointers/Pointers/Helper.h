#pragma once

#include <type_traits>

namespace SPTR
{
	namespace detail
	{
		template <typename T>
		concept isIndexable = requires(T & t, std::size_t i)
		{
			t[i];
		};
	} // end of detail

	template <typename T, typename Deleter>	
	struct ControlBlock
	{
		std::uint64_t refCount = 0;
		std::uint64_t weakCount = 0;
		T* ptr = nullptr;
		// Destructor for the managed object with Deleter;
	};
} // end of SPTR