#pragma once

#include <type_traits>
#include <utility>

// The tuple is implemented as a recursve variadic template class. 
// Howeve, recursion happens not because of forvard declaration but because of the template specislization
// Tuple<int, chat, bool>
// is a child of Tuple<chat, bool>, which is a child of Tuple<bool>, which is a child of Tuple<>. 
// It's also woth mentioning that there's no penalty in terms of CPU time because teverythinf happens at compile time.
// It soe not have penalty in terms of memory because the empty base class optimization will kick in and Tuple<> will not take any space.
// Conseptually it could be implemented as an composition of two classes:
// Tuple<Head, Tail...> and Tuple<Tail...> where the last one could receive the empty parameter/type
namespace VT 
{
	template <typename... Tail>
	class Tuple;

	template <>
	class Tuple<>{};
	
	template <typename Head, typename... Tail>
	class Tuple<Head, Tail...> : public Tuple<Tail...>
	{
	private:
		template <typename... Tail>
		friend class Tuple;

		Head head_;
	public:
		Tuple() = default;
		Tuple(const Head& head, const Tail&... tail) noexcept;
		Tuple(const Tuple& other);
		Tuple(Tuple&& other) ;
		Tuple& operator=(const Tuple& other) ;
		Tuple& operator=(Tuple&& other) ;

		void swap(Tuple& other);
		// Not a member function, but a free function that creates a tuple from given arguments
		// Tuple<Head, Tail...> makeTuple(const Head& head, const Tail&... tail) noexcept;

		bool operator== (const Tuple& other) const;
		bool operator!= (const Tuple& other) const;
		bool operator< (const Tuple& other) const;
		bool operator> (const Tuple& other) const;
		bool operator<= (const Tuple& other) const;
		bool operator>= (const Tuple& other) const;
	};

	// Initialiser list sequence doesn't really matter but Tail before the head depicts how compiler 
	// actually initializes the base class before the members of the derived
	// Base class should be initialized in the initializer list, not in the body
	template<typename Head, typename ...Tail>																						
	inline Tuple<Head, Tail...>::Tuple(const Head& head, const Tail & ...tail) noexcept : Tuple<Tail...>(tail...), head_(head) {}

	template<typename Head, typename ...Tail>
	inline Tuple<Head, Tail...>::Tuple(const Tuple& other)  : Tuple<Tail...>(other), head_(other.head_) {}

	template<typename Head, typename ...Tail>
	inline Tuple<Head, Tail...>::Tuple(Tuple&& other)  : Tuple<Tail...>(std::move(other)), head_(std::move(other.head_)) {}

	template<typename Head, typename ...Tail>
	inline Tuple<Head, Tail...>& Tuple<Head, Tail...>::operator=(const Tuple& other)
	{
		if (this != &other)
		{
			Tuple<Tail...>::operator=(other);
			head_ = other.head_;
		}
		return *this;
	}

	template<typename Head, typename ...Tail>
	inline Tuple<Head, Tail...>& Tuple<Head, Tail...>::operator=(Tuple&& other)
	{
		if (this != &other)
		{
			Tuple<Tail...>::operator=(std::move(other));
			head_ = std::move(other.head_);
		}
		return *this;
	}

	template<typename Head, typename ...Tail>
	inline void VT::Tuple<Head, Tail...>::swap(Tuple& other)
	{
		std::swap(static_cast<Tuple<Tail...>&>(*this), static_cast<Tuple<Tail...>&>(other));
		std::swap(head_, other.head_);
	}

	template<typename Head, typename ...Tail>
	inline bool VT::Tuple<Head, Tail...>::operator==(const Tuple& other) const
	{
		if (head_ == other.head_
			&& static_cast<const Tuple<Tail...>&>(*this)
			== static_cast<const Tuple<Tail...>&>(other)) { return true; }
		return false;
	}

	template<typename Head, typename ...Tail>
	inline bool VT::Tuple<Head, Tail...>::operator!= (const Tuple& other) const
	{
		return !(*this == other);
	}

	template<typename Head, typename ...Tail>
	inline bool VT::Tuple<Head, Tail...>::operator> (const Tuple& other) const
	{
		if (head_ > other.head_) 
		{ 
			return true; 
		}
		else if (head_ < other.head_) 
		{
			return false;
		}
		else
		{ 
			return static_cast<const Tuple<Tail...>&>(*this) > static_cast<const Tuple<Tail...>&>(other);
		}
	}

	template<typename Head, typename ...Tail>
	inline bool VT::Tuple<Head, Tail...>::operator< (const Tuple& other) const
	{
		if (head_ < other.head_)
		{
			return true;
		}
		else if(head_ > other.head_)
		{
			return false;
		}
		else
		{
			return static_cast<const Tuple<Tail...>&>(*this) < static_cast<const Tuple<Tail...>&>(other);
		}
		
		return false;
	}

	template<typename Head, typename ...Tail>
	inline bool VT::Tuple<Head, Tail...>::operator<=(const Tuple& other) const
	{
		if (head_ < other.head_)
		{
			return true;
		}
		else if (head_ > other.head_)
		{
			return false;
		}
		else
		{
			return static_cast<const Tuple<Tail...>&>(*this) <= static_cast<const Tuple<Tail...>&>(other);
		}
	}

	template<typename Head, typename ...Tail>
	inline bool VT::Tuple<Head, Tail...>::operator>=(const Tuple& other) const
	{
		if (head_ > other.head_)
		{
			return true;
		}
		else if (head_ < other.head_)
		{
			return false;
		}
		else
		{
			return static_cast<const Tuple<Tail...>&>(*this) >= static_cast<const Tuple<Tail...>&>(other);
		}
	}

	// Non-member functions

	template <std::size_t Index, typename... Types>
	class TypeAtIndex 
	{
	public:
		static typeAtIndex()
	};




	template <std::size_t index, typename... Types>
	typename type_at_index<index, Types...>::type& get(Tuple<Types...>& tuple);

} // namespace VT