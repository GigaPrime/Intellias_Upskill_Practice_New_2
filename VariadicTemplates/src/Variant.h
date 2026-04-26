#pragma once

#include <type_traits>
#include <cstring>
#include <stdexcept>

namespace VT 
{
	// Compile time deducing the type at given index
	template <std::size_t Index, typename... Types>
	struct CPTypeAtIndex;

	// Compile time deducing the index of a given type 
	template <typename T, std::size_t Index, typename... Types>
	struct CPIndexAtType;

	// Run time deducing type at index for copy/move constructors and assignment operators
	template <std::size_t Index, typename... Types>
	struct RTTypeAtIndex;

	template<typename... Types>
	constexpr std::size_t GetLargestSize();

	template<typename... Types>
	constexpr std::size_t GetLargestSize();

	template<typename... Types>
	constexpr std::size_t GetLargestAlignment();

	template <typename... Types>
	class Variant
	{
	private:
		// Compile-time determination of the largest size and alignment among the types
		alignas(GetLargestAlignment<Types...>()) std::byte storage_[GetLargestSize<Types...>()];
		std::size_t typeIndex_ = 0;

		template <typename... Types>
		friend class Variant;

		void destroy();

	public:
		Variant();
		Variant(const Variant& other);
		Variant(Variant&& other);
		Variant& operator=(const Variant& other);
		Variant& operator=(Variant&& other);

		template<std::size_t I, class... Args>
		constexpr explicit Variant(std::in_place_index_t<I>, Args&&... args);

		~Variant();

		void swap(Variant& other);

		bool operator==(const Variant& other) const;
		bool operator!=(const Variant& other) const;
		bool operator<(const Variant& other) const;
		bool operator>(const Variant& other) const;
		bool operator<=(const Variant& other) const;
		bool operator>=(const Variant& other) const;
	};

	// Non-member functions
	template<>
	struct CPTypeAtIndex<-1>
	{
		using type = void;
	};

	template <typename Head, typename... Types>
	struct CPTypeAtIndex<0, Head, Types...>
	{
		using type = Head;
	};
	
	template <int Index, typename Head, typename... Types>
	struct CPTypeAtIndex<Index, Head, Types...>
	{
		using type = typename CPTypeAtIndex<Index - 1, Types...>::type;
	};

	template <typename T, std::size_t Index, typename Head, typename... Tail>
	struct CPIndexAtType<T, Index, Head, Tail...>
	{
		static constexpr std::size_t value = 
			std::is_same<T, Head>::value ? Index : CPIndexAtType<T, Index + 1, Tail...>::value;
	};

	template <typename T, std::size_t Index>
	struct CPIndexAtType<T, Index>
	{
		// Force compile error
		static_assert(sizeof(T) == 0, "Type not found in Variant type list");
		static constexpr std::size_t value = 0; // Type not found
	};

	// This is agly. Second template param is not needed at all. 
	// Compile error happens without it since it would be incorrect partial template specialization
	// The param is not used
	// To fix this a separate (not a partial specialization for CPIndexAtType) struct should be introduced
	template <typename T, std::size_t Index, typename... Types>
	struct CPIndexAtType
	{
		static constexpr std::size_t value = CPIndexAtType<T, 0, Types...>::value;
	};

	template <std::size_t Index>
	struct RTTypeAtIndex<Index>
	{
		static constexpr void recurseCopy(const std::size_t otherIindex,
			alignas(GetLargestAlignment<Types...>()) std::byte thisStorage[GetLargestSize<Types...>()],
			alignas(GetLargestAlignment<Types...>()) std::byte otherStorage[GetLargestSize<Types...>()])
		{
			throw std::runtime_error("No valid type found in Variant type list");
		}

		static constexpr bool recurseEqual(const std::size_t otherIindex,
			alignas(GetLargestAlignment<Types...>()) std::byte thisStorage[GetLargestSize<Types...>()],
			alignas(GetLargestAlignment<Types...>()) std::byte otherStorage[GetLargestSize<Types...>()])
		{
			throw std::runtime_error("No valid type found in Variant type list");
		}

		static constexpr bool recurseLess(const std::size_t otherIindex,
			alignas(GetLargestAlignment<Types...>()) std::byte thisStorage[GetLargestSize<Types...>()],
			alignas(GetLargestAlignment<Types...>()) std::byte otherStorage[GetLargestSize<Types...>()])
		{
			throw std::runtime_error("No valid type found in Variant type list");
		}
	};

	template <std::size_t Index, typename Head, typename... Tail>
	struct RTTypeAtIndex<Index, Head, Tail...>
	{
		static constexpr void recurseCopy (const std::size_t otherIindex,
			alignas(GetLargestAlignment<Types...>()) std::byte thisStorage[GetLargestSize<Types...>()],
			alignas(GetLargestAlignment<Types...>()) std::byte otherStorage[GetLargestSize<Types...>()])
		{
			if (Index != otherIindex)
			{
				RTTypeAtIndex<Index + 1, Tail...>::recurseCopy(otherIindex, thisStorage, otherStorage);
			}
			new(&thisStorage[0]) T(*reinterpret_cast<T*>(&other.otherStorage[0]));
		}

		static constexpr void recurseMove(const std::size_t otherIindex,
			alignas(GetLargestAlignment<Types...>()) std::byte thisStorage[GetLargestSize<Types...>()],
			alignas(GetLargestAlignment<Types...>()) std::byte otherStorage[GetLargestSize<Types...>()])
		{
			if (Index != otherIindex)
			{
				RTTypeAtIndex<Index + 1, Tail...>::recurseMove(otherIindex, thisStorage, otherStorage);
			}
			new(&thisStorage[0]) T(*reinterpret_cast<T*>(&other.otherStorage[0]));
		}

		static constexpr void recurseDestruct(const std::size_t otherIindex,
			alignas(GetLargestAlignment<Types...>()) std::byte thisStorage[GetLargestSize<Types...>()])
		{
			if (Index != otherIindex)
			{
				RTTypeAtIndex<Index + 1, Tail...>::recurseDestruct(otherIindex, thisStorage);
			}
			reinterpret_cast<T*>(&thisStorage[0])->~T();
		}

		static constexpr bool recurseEqual(const std::size_t otherIindex,
			alignas(GetLargestAlignment<Types...>()) std::byte thisStorage[GetLargestSize<Types...>()],
			alignas(GetLargestAlignment<Types...>()) std::byte otherStorage[GetLargestSize<Types...>()])
		{
			if (Index != otherIindex)
			{
				return RTTypeAtIndex<Index + 1, Tail...>::recurseEqual(otherIindex, thisStorage, otherStorage);
			}
			return *reinterpret_cast<Head*>(&thisStorage[0]) == *reinterpret_cast<Head*>(&otherStorage[0]);
		}

		static constexpr bool recurseLess(const std::size_t otherIindex,
			alignas(GetLargestAlignment<Types...>()) std::byte thisStorage[GetLargestSize<Types...>()],
			alignas(GetLargestAlignment<Types...>()) std::byte otherStorage[GetLargestSize<Types...>()])
		{
			if (Index != otherIindex)
			{
				return RTTypeAtIndex<Index + 1, Tail...>::recurseLess(otherIindex, thisStorage, otherStorage);
			}
			return *reinterpret_cast<Head*>(&thisStorage[0]) < *reinterpret_cast<Head*>(&otherStorage[0]);
		}
	};

	template <std::size_t Index, typename... Types>
	struct RTTypeAtIndex{};

	/// Get the largest size among all types
	template<typename... Types>
	constexpr std::size_t GetLargestSize()
	{
		return std::max({sizeof(Types)...});
	}

	/// Get the largest alignment among all types
	template<typename... Types>
	constexpr std::size_t GetLargestAlignment()
	{
		return std::max({alignof(Types)...});
	}

	template<typename ...Types>
	inline void Variant<Types...>::destroy()
	{
		RTTypeAtIndex::recurseDestruct(typeIndex_, storage_);
	}

	// To simulate behaviour of std::variant, the default constructor 
	// should create an instance of the first type in the Types list.
	template<typename ...Types>
	inline Variant<Types...>::Variant() : typeIndex_(0)
	{
		new(&storage_[0]) typename CPTypeAtIndex<0, Types...>::type();
	}

	template<typename ...Types>
	template<std::size_t I, class... Args>
	inline constexpr Variant<Types...>::Variant(std::in_place_index_t<I>, Args&&... args) : typeIndex_(I)
	{
		using Type = typename CPTypeAtIndex<I, Types...>::type;
		new(&storage_[0]) Type(std::forward<Args>(args)...);
	}

	template<typename ...Types>
	inline Variant<Types...>::Variant(const Variant& other) : typeIndex_(other.typeIndex_) 
	{
		RTTypeAtIndex::recurseCopy(other.typeIndex_, storage_, other.storage_);
	}

	template<typename ...Types>
	inline Variant<Types...>::Variant(Variant&& other)
	{
		RTTypeAtIndex::recurseMove(other.typeIndex_, storage_, other.storage_);
	}

	template<typename ...Types>
	inline Variant<Types...>& Variant<Types...>::operator=(const Variant& other)
	{
		RTTypeAtIndex::recurseCopy(other.typeIndex_, storage_, other.storage_);
		return *this;
	}

	template<typename ...Types>
	inline Variant<Types...>& Variant<Types...>::operator=(Variant&& other)
	{
		RTTypeAtIndex::recurseMove(other.typeIndex_, storage_, other.storage_);
		return *this;
	}

	template<typename ...Types>
	inline Variant<Types...>::~Variant()
	{
		destroy();
	}

	template<typename ...Types>
	inline bool Variant<Types...>::operator==(const Variant& other) const
	{
		if (typeIndex_ != other.typeIndex_)
		{
			return false;
		}
		return RTTypeAtIndex<0, Types...>::recurseEqual(typeIndex_, storage_, other.storage_);
	}

	template<typename ...Types>
	inline bool Variant<Types...>::operator!=(const Variant& other) const
	{
		return !(*this == other);
	}

	template<typename ...Types>
	inline bool Variant<Types...>::operator<(const Variant& other) const
	{
		if (typeIndex_ != other.typeIndex_)
		{
			return typeIndex_ < other.typeIndex_;
		}
		return RTTypeAtIndex<0, Types...>::recurseLess(typeIndex_, storage_, other.storage_);
	}

	template<typename ...Types>
	inline bool Variant<Types...>::operator>(const Variant& other) const
	{
		return other < *this;
	}

	template<typename ...Types>
	inline bool Variant<Types...>::operator<=(const Variant& other) const
	{
		return !(other < *this);
	}

	template<typename ...Types>
	inline bool Variant<Types...>::operator>=(const Variant& other) const
	{
		return !(*this < other);
	}

} // namespace
