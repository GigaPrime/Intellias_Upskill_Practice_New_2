#pragma once

#include <type_traits>

namespace VT 
{
	template <size_t Index, typename... Types>
	struct TypeAtIndex;

	template<typename... Types>
	constexpr size_t GetLargestSize();

	template<typename... Types>
	constexpr size_t GetLargestAlignment();
	
	template <typename T, typename... Types>
	class Variant
	{
	private:
		alignas(T) std::byte storage_[sizeof(T)];

		template <typename... Types>
		friend class Variant;

		void destroy();

	public:
		Variant();
		Variant(const Types& type) noexcept;
		Variant(const Variant& other);
		Variant(Variant&& other);
		Variant& operator=(const Variant& other);
		Variant& operator=(Variant&& other);
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
	template <typename Head, typename... Types>
	struct TypeAtIndex<0, Head, Types...>
	{
		using type = Head;
	};
	
	template <size_t Index, typename Head, typename... Types>
	struct TypeAtIndex<Index, Head, Types...>
	{
		using type = typename TypeAtIndex<Index - 1, Types...>::type;
	};

	/// Get the largest size among all types
	template<typename... Types>
	constexpr size_t GetLargestSize()
	{
		return std::max({sizeof(Types)...});
	}

	/// Get the largest alignment among all types
	template<typename... Types>
	constexpr size_t GetLargestAlignment()
	{
		return std::max({alignof(Types)...});
	}

	
	template<typename T, typename ...Types>
	inline Variant<T, Types...>::Variant()
	{
		
	}

	template<typename T, typename ...Types>
	inline Variant<T, Types...>::Variant(const Variant& other) : Variant<Types...>(other), storage_(other.storage_) {}

	template<typename T, typename ...Types>
	inline Variant<T, Types...>::Variant(Variant&& other) : Variant<Types...>(std::move(other)), storage_(std::move(other.storage_)) {}

	





} // namespace VT