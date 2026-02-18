#pragma once

#include <cstddef>
#include <concepts>
#include <functional>
#include <type_traits>

namespace SPTR
{
	namespace detail
	{
		template <typename T>
		concept isIndexable = requires(T& t, std::size_t i) 
		{
			t[i];
		};
	} // end of detail

	template <typename T>
	// Potentially C++20 concepts could be used 
	// to constrain T types avoiding nullptr and void types
	class UniquePtr final
	{
	private:
		T* ptr_ = nullptr;
		bool isAllocated_ = false;

		bool isTypeValid() const;
		void allocateMemory();

	public:
		UniquePtr() noexcept = default;
		explicit UniquePtr(std::nullptr_t) noexcept;
		explicit UniquePtr(T*& ptr) noexcept;
		explicit UniquePtr(T& obj) noexcept;
		explicit UniquePtr(T&& value) noexcept;
		
		UniquePtr(const UniquePtr& other) = delete;
		UniquePtr(UniquePtr&& other) noexcept;
		
		UniquePtr& operator=(const UniquePtr& other) = delete;
		UniquePtr& operator=(UniquePtr&& other) noexcept;

		~UniquePtr() ;

		T& operator*() const ;
		T* operator->() const ;
		decltype(auto) operator[](const std::size_t index) const;
		T* get() const ;
		void reset() ;
	};

	// Non-member functions
	template<typename R, typename ... P>
	UniquePtr<R> makeUnique(P&& ...args);

	template<typename T>
	UniquePtr<T[]> makeUnique(std::initializer_list<T> init);

	template<typename T>
	bool UniquePtr<T>::isTypeValid() const
	{
		// Deducing the type of T at compile time for void
		if constexpr (std::is_void_v<T>)
		{
			std::cerr << "UniquePtr<void> is not supported."
				<< "The pointer will be set to nullptr." << std::endl;
			return false;
		}
		// Deducing the type of T at compile time for nullptr
		else if constexpr (std::is_null_pointer_v<T>)
		{
			std::cout << "Warning: nullptr assigned" << std::endl;
			return false;
		}
		return true;
	}

	template<typename T>
	void UniquePtr<T>::allocateMemory()
	{
		ptr_ = static_cast<T*>(std::malloc(sizeof(T)));
		if (!ptr_)
		{
			std::cerr << "Memory allocation failed" << std::endl;
		}
		isAllocated_ = true;
	}

	template<typename T>
	inline UniquePtr<T>::UniquePtr(std::nullptr_t) noexcept
	{
		if (!isTypeValid())
		{
			ptr_ = nullptr;
		}
	}

	template<typename T>
	inline UniquePtr<T>::UniquePtr(T*& otherPtr) noexcept
	{
		if(!isTypeValid())
		{
			ptr_ = nullptr;
		}
		else
		{
			ptr_ = otherPtr;
			// Avoiding double ownership
			otherPtr = nullptr;
			isAllocated_ = true;
		}
	}

	template<typename T>
	inline UniquePtr<T>::UniquePtr(T& obj) noexcept
	{
		if (!isTypeValid())
		{
			ptr_ = nullptr;
		}
		else
		{
			ptr_ = &obj;
		}
	}

	template<typename T>
	inline UniquePtr<T>::UniquePtr(T&& value) noexcept
	{
		if (!isTypeValid())
		{
			ptr_ = nullptr;
		}
		else
		{
			allocateMemory();
			std::construct_at(ptr_, value); // or ptr_->T();
		}
	}

	template<typename T>
	inline UniquePtr<T>::~UniquePtr()
	{
		reset();
	}

	template<typename T>
	inline UniquePtr<T>::UniquePtr(UniquePtr&& other) noexcept
	{
		if (this != &other)
		{
			reset();
			ptr_ = other.ptr_;
			isAllocated_ = other.isAllocated_;
			other.ptr_ = nullptr;
			other.isAllocated_ = false;
		}
	}

	template<typename T>
	inline UniquePtr<T>& UniquePtr<T>::operator=(UniquePtr&& other) noexcept
	{
		if (this != &other)
		{
			reset();
			ptr_ = other.ptr_;
			isAllocated_ = other.isAllocated_;
			other.ptr_ = nullptr;
			other.isAllocated_ = false;
		}
		return *this;
	}

	template<typename T>
	inline T& UniquePtr<T>::operator*() const
	{
		if (!ptr_)
		{
			throw std::runtime_error("Dereferencing a nullptr\n");
		}
		return *ptr_;
	}

	template<typename T>
	inline T* SPTR::UniquePtr<T>::operator->() const
	{
		return ptr_;
	}

	template<typename T>
	decltype(auto) UniquePtr<T>::operator[](std::size_t index) const
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

	template<typename T>
	inline T* UniquePtr<T>::get() const
	{
		return ptr_;
	}

	template<typename T>
	inline void UniquePtr<T>::reset()
	{
		if (ptr_)
		{
			std::destroy_at(ptr_); // or ptr_->~T();
			if (isAllocated_)
			{
				std::free(ptr_);
			}
		}
		ptr_ = nullptr;
	}

	// Non-member functions

	template<typename R, typename ... P>
	UniquePtr<R> makeUnique(P&& ...args)
	{
		R* ptr = static_cast<R*>(std::malloc(sizeof(R)));
		if (!ptr)
		{
			throw std::bad_alloc();
		}

		try 
		{
			// std::construct_at is noexcept, but the called c-tor may throw
			std::construct_at(ptr, std::forward<P>(args)...);
		}
		catch (...)
		{
			// nothing was created, no std::destroy_at / d-tor needed
			std::free(ptr);
			// re-throwing further
			throw;
		}
		
		return SPTR::UniquePtr<R> (ptr);
	}

	// operator < / >
	// operator ==
	// ...


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
