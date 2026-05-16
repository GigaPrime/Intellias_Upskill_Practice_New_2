#pragma once

#include <type_traits>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace VT 
{
	// Compile time deducing the type at given index
	template <std::size_t Index, typename... Types>
	struct CPTypeAtIndex;

	// Compile time deducing the index of a given type 
	template <typename T, std::size_t Index, typename... Types>
	struct CPIndexAtType;

	// Run time deeducing value at index
	template <std::size_t Index, typename... Types>
	struct RTValueAtIndex;

	// Run time deducing type at index for copy/move constructors and assignment operators
	template <std::size_t Index, typename... Types>
	struct RTTypeAtIndex;

	// A helper used for holdsAlternative function allowing for duplicated types deduction
	template <typename T, std::size_t Index, typename... Types>
	struct CheckTypeAtIndex;

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

		template<std::size_t Index, typename... Types>
		friend constexpr typename CPTypeAtIndex<Index, Types...>::type& get(Variant<Types...>& variant);

		template<std::size_t Index, typename... Types>
		friend constexpr const typename CPTypeAtIndex<Index, Types...>::type& get(const Variant<Types...>& variant);

		template<std::size_t Index, typename... Types>
		friend constexpr typename CPTypeAtIndex<Index, Types...>::type&& get(Variant<Types...>&& variant);

		template<std::size_t Index, typename... Types>
		friend constexpr const typename CPTypeAtIndex<Index, Types...>::type&& get(const Variant<Types...>&& variant);

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
		std::size_t index() const;

		template<typename Visitor, 
			typename ReturnType = decltype(std::declval<Visitor>()(std::declval<const typename CPTypeAtIndex<0, Types...>::type&>()))>
		ReturnType visit(Visitor&& visitor) const;

		bool operator==(const Variant& other) const;
		bool operator!=(const Variant& other) const;
		bool operator<(const Variant& other) const;
		bool operator>(const Variant& other) const;
		bool operator<=(const Variant& other) const;
		bool operator>=(const Variant& other) const;
	};

	// Non-member functions
	template <typename Head, typename... Types>
	struct CPTypeAtIndex<0, Head, Types...>
	{
		using type = Head;
	};
	
	template <std::size_t Index, typename Head, typename... Types>
	struct CPTypeAtIndex<Index, Head, Types...>
	{
		using type = typename CPTypeAtIndex<Index - 1, Types...>::type;
	};

	template <typename T, std::size_t Index, typename Head, typename... Tail>
	struct CPIndexAtType<T, Index, Head, Tail...>
	{
		static constexpr std::size_t getIndex()
		{
			if constexpr (std::is_same<T, Head>::value)
			{
				return Index;
			}
			else
			{
				return CPIndexAtType<T, Index + 1, Tail...>::getIndex();
			}
		}
	};

	template <typename T, std::size_t Index>
	struct CPIndexAtType<T, Index>
	{
		static constexpr std::size_t getIndex()
		{
			// Force compile error
			static_assert(sizeof(T) == 0, "Type not found in Variant type list");
			static constexpr std::size_t index = 0; // Type not found
			return index;
		}	
	};

	// A kinda wrapper over the return type of the Visitor in case it is void or lambda with no return type specified
	template <typename T>
	struct TypeTag
	{
		using type = T;
	};

	template <std::size_t Index, typename... Types>
	struct RTTypeAtIndex
	{
		static constexpr void recurseCopy(const std::size_t otherIindex, std::byte* thisStorage, const std::byte* otherStorage)
		{
			throw std::runtime_error("No valid type found in Variant type list");
		}

		static constexpr void recurseMove(const std::size_t otherIindex, std::byte* thisStorage, std::byte* otherStorage)
		{
			throw std::runtime_error("No valid type found in Variant type list");
		}

		static constexpr void recurseDestruct(const std::size_t otherIindex, std::byte* thisStorage)
		{
			throw std::runtime_error("No valid type found in Variant type list");
		}

		static constexpr bool recurseEqual(const std::size_t otherIindex, const std::byte* thisStorage, const std::byte* otherStorage)
		{
			throw std::runtime_error("No valid type found in Variant type list");
		}

		static constexpr bool recurseLess(const std::size_t otherIindex, const std::byte* thisStorage, const std::byte* otherStorage)
		{
			throw std::runtime_error("No valid type found in Variant type list");
		}

		template <typename Visitor, typename ReturnType>
		static constexpr ReturnType recurseVisit(const std::size_t index, const std::byte* storage, Visitor&& visitor)
		{
			throw std::runtime_error("No valid type found in Variant type list");
		}
	};

	// alignas can't be used here since arrays should be passed here not the storage itself.
	// That's why the storage is passed as a pointer.
	template <std::size_t Index, typename Head, typename... Types>
	struct RTTypeAtIndex<Index, Head, Types...>
	{
		static constexpr void recurseCopy (const std::size_t otherIindex, std::byte* thisStorage, const std::byte* otherStorage)
		{
			if (Index != otherIindex)
			{
				RTTypeAtIndex<Index + 1, Types...>::recurseCopy(otherIindex, thisStorage, otherStorage);
			}
			else
			{
				new(thisStorage) Head(*reinterpret_cast<const Head*>(otherStorage));
			}
		}

		static constexpr void recurseMove(const std::size_t otherIindex, std::byte* thisStorage, std::byte* otherStorage)
		{
			if (Index != otherIindex)
			{
				RTTypeAtIndex<Index + 1, Types...>::recurseMove(otherIindex, thisStorage, otherStorage);
			}
			else
			{
				new(thisStorage) Head(std::move(*reinterpret_cast<Head*>(otherStorage)));
			}
		}

		static constexpr void recurseDestruct(const std::size_t otherIindex, std::byte* thisStorage)
		{
			if (Index != otherIindex)
			{
				RTTypeAtIndex<Index + 1, Types...>::recurseDestruct(otherIindex, thisStorage);
			}
			else
			{
				reinterpret_cast<Head*>(thisStorage)->~Head();
			}
		}

		static constexpr bool recurseEqual(const std::size_t otherIindex, const std::byte* thisStorage, const std::byte* otherStorage)
		{
			if (Index != otherIindex)
			{
				return RTTypeAtIndex<Index + 1, Types...>::recurseEqual(otherIindex, thisStorage, otherStorage);
			}
			return *reinterpret_cast<const Head*>(thisStorage) == *reinterpret_cast<const Head*>(otherStorage);
		}

		static constexpr bool recurseLess(const std::size_t otherIindex, const std::byte* thisStorage, const std::byte* otherStorage)
		{
			if (Index != otherIindex)
			{
				return RTTypeAtIndex<Index + 1, Types...>::recurseLess(otherIindex, thisStorage, otherStorage);
			}
			return *reinterpret_cast<const Head*>(thisStorage) < *reinterpret_cast<const Head*>(otherStorage);
		}

		template <typename Visitor, typename ReturnType>
		static constexpr ReturnType recurseVisit(const std::size_t index, const std::byte* storage, Visitor&& visitor)
		{
			if (Index == index)
			{
				return visitor(*reinterpret_cast<const Head*>(storage));
			}
			else
			{
				return RTTypeAtIndex<Index + 1, Types...>::template recurseVisit<Visitor, ReturnType>(index, storage, std::forward<Visitor>(visitor));
			}
		}
	};

	template<std::size_t Index, typename... Types>
	struct RTValueAtIndex
	{
		static_assert(Index != Index, "Index out of range in Variant type list");
	};

	template<typename Head, typename... Types>
	struct RTValueAtIndex<0, Head, Types...>
	{
		static constexpr Head* getValue(std::byte* storage)
		{
			return reinterpret_cast<Head*>(storage);
		}

		static constexpr const Head* getValue(const std::byte* storage)
		{
			return reinterpret_cast<const Head*>(storage);
		}
	};

	template<std::size_t Index, typename Head, typename... Types>
	struct RTValueAtIndex<Index, Head, Types...>
	{
		static_assert(Index <= sizeof...(Types), "Index out of range in Variant type list");

		static constexpr Head* getValue(std::byte* storage)
		{
			return RTValueAtIndex<Index - 1, Types...>::getValue(storage);
		}

		static constexpr const Head* getValue(const std::byte* storage)
		{
			return RTValueAtIndex<Index - 1, Types...>::getValue(storage);
		}
	};

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


	// Base case specialization - empty Types
	template<typename T, std::size_t Index>
	struct CheckTypeAtIndex<T, Index>
	{
		static constexpr bool check(std::size_t currentIdx)
		{
			return false;
		}
	};

	// Recursive case
	template<typename T, std::size_t Index, typename Head, typename... Tail>
	struct CheckTypeAtIndex<T, Index, Head, Tail...>
	{
		static constexpr bool check(std::size_t currentIdx)
		{
			if (currentIdx == Index)
			{
				return std::is_same<T, Head>::value;
			}
			else
			{
				return CheckTypeAtIndex<T, Index + 1, Tail...>::check(currentIdx);
			}
		}
	};

	template<typename T, typename... Types >
	bool holdsAlternative(const Variant<Types...>& variant)
	{
		return CheckTypeAtIndex<T, 0, Types...>::check(variant.index());
	}

	template<std::size_t Index, typename... Types>
	constexpr typename CPTypeAtIndex<Index, Types...>::type& get(Variant<Types...>& variant)
	{
		if (variant.index() != Index)
		{
			throw std::runtime_error("Variant does not hold the requested type");
		}

		using Type = typename CPTypeAtIndex<Index, Types...>::type;
		return *reinterpret_cast<Type*>(variant.storage_);
	}

	template<std::size_t Index, typename... Types>
	constexpr const typename CPTypeAtIndex<Index, Types...>::type& get(const Variant<Types...>& variant)
	{
		if (variant.index() != Index)
		{
			throw std::runtime_error("Variant does not hold the requested type");
		}

		using Type = typename CPTypeAtIndex<Index, Types...>::type;
		return *reinterpret_cast<const Type*>(variant.storage_);
	}

	template<std::size_t Index, typename... Types>
	constexpr typename CPTypeAtIndex<Index, Types...>::type&& get(Variant<Types...>&& variant)
	{
		if (variant.index() != Index)
		{
			throw std::runtime_error("Variant does not hold the requested type");
		}

		using Type = typename CPTypeAtIndex<Index, Types...>::type;
		return std::move(*reinterpret_cast<Type*>(variant.storage_));
	}

	template<std::size_t Index, typename... Types>
	constexpr const typename CPTypeAtIndex<Index, Types...>::type&& get(const Variant<Types...>&& variant)
	{
		if (variant.index() != Index)
		{
			throw std::runtime_error("Variant does not hold the requested type");
		}

		using Type = typename CPTypeAtIndex<Index, Types...>::type;
		return std::move(*reinterpret_cast<const Type*>(variant.storage_));
	}

	// Variant member functions implementations

	template<typename ...Types>
	inline void Variant<Types...>::destroy()
	{
		RTTypeAtIndex<0, Types...>::recurseDestruct(typeIndex_, storage_);
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

	template <typename ...Types>
	template<typename Visitor, typename ReturnType>
	inline ReturnType Variant<Types...>::visit(Visitor&& visitor) const
	{
		return RTTypeAtIndex<0, Types...>::template recurseVisit<Visitor, ReturnType>(typeIndex_, storage_, std::forward<Visitor>(visitor));
	}

	template<typename ...Types>
	inline Variant<Types...>::Variant(const Variant& other) : typeIndex_(other.typeIndex_) 
	{
		RTTypeAtIndex<0, Types...>::recurseCopy(other.typeIndex_, storage_, other.storage_);
	}

	template<typename ...Types>
	inline Variant<Types...>::Variant(Variant&& other) : typeIndex_(other.typeIndex_)
	{
		RTTypeAtIndex<0, Types...>::recurseMove(other.typeIndex_, storage_, other.storage_);
		other.destroy();
		other.typeIndex_ = 0;
		new(&other.storage_[0]) typename CPTypeAtIndex<0, Types...>::type();
	}

	template<typename ...Types>
	inline Variant<Types...>& Variant<Types...>::operator=(const Variant& other)
	{
		if (this != &other)
		{
			destroy();
			typeIndex_ = other.typeIndex_;
			RTTypeAtIndex<0, Types...>::recurseCopy(other.typeIndex_, storage_, other.storage_);
		}
		return *this;
	}

	template<typename ...Types>
	inline Variant<Types...>& Variant<Types...>::operator=(Variant&& other)
	{
		if (this != &other)
		{
			destroy();
			typeIndex_ = other.typeIndex_;
			RTTypeAtIndex<0, Types...>::recurseMove(other.typeIndex_, storage_, other.storage_);
		}
		return *this;
	}

	template<typename ...Types>
	inline Variant<Types...>::~Variant()
	{
		destroy();
	}

	template<typename ...Types>
	inline void Variant<Types...>::swap(Variant& other)
	{
		Variant temp(std::move(*this));
		*this = std::move(other);
		other = std::move(temp);
	}

	template<typename ...Types>
	inline std::size_t Variant<Types...>::index() const
	{
		 return typeIndex_;
	}

	template<typename ...Types>
	inline bool Variant<Types...>::operator==(const Variant& other) const
	{
		if (typeIndex_ != other.typeIndex_)
		{
			return false;
		}
		return RTTypeAtIndex<0, Types...>::recurseEqual(typeIndex_, const_cast<std::byte*>(storage_), const_cast<std::byte*>(other.storage_));
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
		return RTTypeAtIndex<0, Types...>::recurseLess(typeIndex_, const_cast<std::byte*>(storage_), const_cast<std::byte*>(other.storage_));
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
