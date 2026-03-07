#pragma once

#include <cstddef>
#include <concepts>
#include <functional>
#include <memory>
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

	template <typename T>
	struct Deleter;

	template <typename T, typename Deleter = std::default_delete<T>>
	class UniquePtr final
	{
	private:
		T* ptr_ = nullptr;

	public:
		UniquePtr() noexcept = default;
		explicit UniquePtr(std::nullptr_t) noexcept;
		explicit UniquePtr(T* ptr) noexcept;

		UniquePtr(const UniquePtr& other) = delete;
		UniquePtr(UniquePtr&& other) noexcept;

		UniquePtr& operator=(const UniquePtr& other) = delete;
		UniquePtr& operator=(UniquePtr&& other) noexcept;

		~UniquePtr();

		T& operator*();
		T* operator->();
		decltype(auto) operator[](const std::size_t index) const;
		T* get() const;
		void reset();
	};

	template<typename T, typename Deleter>
	inline SPTR::UniquePtr<T, Deleter>::UniquePtr(std::nullptr_t) noexcept : ptr_(nullptr) {}

	template<typename T, typename Deleter>
	inline UniquePtr<T, Deleter>::UniquePtr(T* otherPtr) noexcept : ptr_(otherPtr) {}

	template<typename T, typename Deleter>
	inline UniquePtr<T, Deleter>::~UniquePtr()
	{
		reset();
	}

	template<typename T, typename Deleter>
	inline UniquePtr<T, Deleter>::UniquePtr(UniquePtr&& other) noexcept
	{
		if (this != &other)
		{
			reset();
			ptr_ = other.ptr_;
			other.ptr_ = nullptr;
		}
	}

	template<typename T, typename Deleter>
	inline UniquePtr<T, Deleter>& UniquePtr<T, Deleter>::operator=(UniquePtr&& other) noexcept
	{
		if (this != &other)
		{
			reset();
			ptr_ = other.ptr_;
			other.ptr_ = nullptr;
		}
		return *this;
	}

	template<typename T, typename Deleter>
	inline T& UniquePtr<T, Deleter>::operator*()
	{
		if (!ptr_)
		{
			throw std::runtime_error("Dereferencing a nullptr\n");
		}
		return *ptr_;
	}

	template<typename T, typename Deleter>
	inline T* SPTR::UniquePtr<T, Deleter>::operator->()
	{
		if (!ptr_)
		{
			throw std::runtime_error("Dereferencing a nullptr\n");
		}
		return ptr_;
	}

	template<typename T, typename Deleter>
	decltype(auto) UniquePtr<T, Deleter>::operator[](std::size_t index) const
	{
		if constexpr (!detail::isIndexable<T>)
		{
			throw std::runtime_error("Type provided is not iterable\n");
		}
		else 
		{
			if(!ptr_)
			{
				throw std::runtime_error("Dereferencing a nullptr\n");
			}
			return (*ptr_)[index];
		} 
	}

	template<typename T, typename Deleter>
	inline T* UniquePtr<T, Deleter>::get() const
	{
		return ptr_;
	}

	template<typename T, typename Deleter>
	inline void UniquePtr<T, Deleter>::reset()
	{
		Deleter deleter;
		deleter(ptr_);
		ptr_ = nullptr;
	}

	// Non-member functions

	template<typename R, typename ... P>
	UniquePtr<R> makeUnique(P&& ...args)
	{
		R* ptr;

		try 
		{
			ptr = new R(std::forward<P>(args)...);
		}
		catch (...)
		{
			// re-throwing further
			throw;
		}
		
		return SPTR::UniquePtr<R> (ptr);
	}

	template<typename T>
	bool operator==(const UniquePtr<T>& lhs, const UniquePtr<T>& rhs)
	{
		return lhs.get() == rhs.get();
	}

	template<typename T>
	bool operator!=(const UniquePtr<T>& lhs, const UniquePtr<T>& rhs)
	{
		return lhs.get() != rhs.get();
	}

	template<typename T>
	bool operator>(const UniquePtr<T>& lhs, const UniquePtr<T>& rhs)
	{
		return lhs.get() > rhs.get();
	}

	template<typename T>
	bool operator>=(const UniquePtr<T>& lhs, const UniquePtr<T>& rhs)
	{
		return lhs.get() >= rhs.get();
	}

	template<typename T>
	bool operator<(const UniquePtr<T>& lhs, const UniquePtr<T>& rhs)
	{
		return lhs.get() < rhs.get();
	}

	template<typename T>
	bool operator<=(const UniquePtr<T>& lhs, const UniquePtr<T>& rhs)
	{
		return lhs.get() <= rhs.get();
	}

	// Partial template specialization for void*
	template <>
	class UniquePtr<void>
	{
	private:
		void* ptr_ = nullptr;
	
	public:
		UniquePtr() = delete;
		explicit UniquePtr(void*& ptr) noexcept : ptr_(ptr) {};
	
		UniquePtr(const UniquePtr& other) = delete;
		UniquePtr(UniquePtr&& other) = delete;
	
		UniquePtr& operator=(const UniquePtr& other) = delete;
		UniquePtr& operator=(UniquePtr&& other) = delete;
	
		~UniquePtr() {};
	
		void* get() const { return ptr_; };
		void* operator->() const = delete;
		decltype(auto) operator[](const std::size_t index) const = delete;
		void operator*() const = delete;
	};

	template <>
	UniquePtr<void> makeUnique()
	{
		void* ptr = nullptr;
		return UniquePtr<void>(ptr);
	}

	// Partial template specialization for C-style arrays
	template <typename T>
	class UniquePtr<T[]>
	{
	private:
		T* ptr_ = nullptr;
		std::size_t size_ = 0;

	public:
		UniquePtr<T[]>(T* ptr, const std::size_t size) noexcept;
		~UniquePtr<T[]>();

		std::size_t size() const noexcept;
		T* get() const noexcept;

		T& operator[](const std::size_t index);
	};

	template<typename T>
	inline SPTR::UniquePtr<T[]>::UniquePtr(T* ptr, const std::size_t size) noexcept : ptr_(ptr), size_(size)
	{
		if (!ptr_)
		{
			throw std::bad_alloc();
		}
	}

	template<typename T>
	UniquePtr<T[]>::~UniquePtr<T[]>()
	{
		if (ptr_)
		{
			for (std::size_t i = 0; i < size_; ++i)
			{
				std::destroy_at(ptr_ + i);
			}
			std::free(ptr_);
		}
	}

	template<typename T>
	std::size_t UniquePtr<T[]>::size() const noexcept
	{ 
		return size_; 
	}

	template<typename T>
	T* UniquePtr<T[]>::get() const noexcept 
	{ 
		return ptr_; 
	}

	template<typename T>
	inline T& SPTR::UniquePtr<T[]>::operator[](const std::size_t index)
	{
		if (!ptr_)
		{
			throw std::runtime_error("Dereferencing a nullptr\n");
		}
		{
			if (index >= size_) throw std::out_of_range("index");
		}
		return ptr_[index];
	}

	template<typename T>
	UniquePtr<T[]> makeUnique(std::initializer_list<T> init)
	{
		const std::size_t elementsNumber = init.size();
		T* ptr = static_cast<T*>(std::malloc(sizeof(T) * elementsNumber));

		if (!ptr)
		{
			throw std::bad_alloc();
		}

		std::size_t i = 0;
		try 
		{
			for (const T& value : init)
			{
				std::construct_at(ptr + i, value);
				++i;
			}
		}
		catch (...)
		{
			for (std::size_t j = 0; j < i; ++j)
			{
				std::destroy_at(ptr + j);
			}
			std::free(ptr);
			throw;
		}

		return UniquePtr<T[]>(ptr, elementsNumber);
	}
} // end of SPTR
