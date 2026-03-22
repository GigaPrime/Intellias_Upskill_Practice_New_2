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

	// Non-template base for control blocks. This allows storing a single
	// pointer type in smart pointers and avoids invalid conversions between
	// different ControlBlock<T> specializations.
	struct ControlBlockBase
	{
		std::uint64_t refCount_ = 0;
		std::uint64_t weakCount_ = 0;

		virtual ~ControlBlockBase() = default;
	};

    template <typename T>
	struct ControlBlock : public ControlBlockBase
	{
		T* ptr_ = nullptr;

		~ControlBlock() override = default;
	};

	template <typename T>
	struct IndexableControlBlock final : public ControlBlock<T>
	{
		std::uint64_t size_ = 0;

		~IndexableControlBlock() = default;
	};
} // end of SPTR