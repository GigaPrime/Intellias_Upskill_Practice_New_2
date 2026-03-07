#pragma once

#include <memory>
#include <type_traits>
#include <vector>

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
	class SharedPtr final
	{
	private:
		T* ptr_ = nullptr;
		std::uint64_t* refCount_ = new std::uint64_t(0);

	public:
		SharedPtr() noexcept = default;
		explicit SharedPtr(std::nullptr_t) noexcept;
		explicit SharedPtr(T* ptr) noexcept;

		SharedPtr(const SharedPtr& other) noexcept;
		SharedPtr(SharedPtr&& other) noexcept;

		SharedPtr& operator= (const SharedPtr& other) noexcept;
		SharedPtr& operator= (SharedPtr&& other) noexcept;

		~SharedPtr();

		T& operator*();
		T* operator->();
		T& operator[](const std::size_t index);

		T* get() const;
		void reset();
		void reset(T* ptr);
		std::size_t refCount() const noexcept;
	};

	template<typename T, typename Deleter>
	inline SharedPtr<T, Deleter>::SharedPtr(std::nullptr_t) noexcept : ptr_(nullptr), refCount_(new std::uint64_t(0)) {}

	template<typename T, typename Deleter>
	inline SharedPtr<T, Deleter>::SharedPtr(T* ptr) noexcept : ptr_(ptr) 
	{
		if (ptr)
		{
			ptr_ = ptr;
			refCount_ = new std::uint64_t(1);
		}
	}

	template<typename T, typename Deleter>
	inline SharedPtr<T, Deleter>::SharedPtr(const SharedPtr& other) noexcept
	{
		if (this != &other)
		{
			ptr_ = other.ptr_;
			refCount_ = other.refCount_;
			++*refCount_;
		}
	}

	template<typename T, typename Deleter>
	inline SharedPtr<T, Deleter>::SharedPtr(SharedPtr&& other) noexcept
	{
		if (this != &other)
		{
			ptr_ = other.ptr_;
			refCount_ = other.refCount_;
			other.ptr_ = nullptr;
			other.refCount_ = nullptr;
		}
	}

	template<typename T, typename Deleter>
	inline SharedPtr<T, Deleter>& SharedPtr<T, Deleter>::operator=(const SharedPtr& other) noexcept
	{
		if (this != &other)
		{
			reset();
			ptr_ = other.ptr_;
			refCount_ = other.refCount_;
			++*refCount_;
		}
		return *this;
	}

	template<typename T, typename Deleter>
	inline SharedPtr<T, Deleter>& SharedPtr<T, Deleter>::operator=(SharedPtr<T, Deleter>&& other) noexcept
	{
		if (this != &other)
		{
			reset();
			ptr_ = other.ptr_;
			refCount_ = other.refCount_;
			other.ptr_ = nullptr;
			other.refCount_ = nullptr;
		}		
		return *this;
	}

	template<typename T, typename Deleter>
	inline SharedPtr<T, Deleter>::~SharedPtr()
	{
		reset();
	}

	template<typename T, typename Deleter>
	inline T& SharedPtr<T, Deleter>::operator*()
	{
		if (!ptr_)
		{
			throw std::runtime_error("Dereferencing a nullptr\n");
		}
		return *ptr_;
	}

	template<typename T, typename Deleter>
	inline T* SharedPtr<T, Deleter>::operator->()
	{
		if (!ptr_)
		{
			throw std::runtime_error("Dereferencing a nullptr\n");
		}
		return ptr_;
	}

	template<typename T, typename Deleter>
	inline T& SharedPtr<T, Deleter>::operator[](const std::size_t index)
	{
		if constexpr (!detail::isIndexable<T>)
		{
			throw std::runtime_error("Type provided is not iterable\n");
		}
		else
		{
			if (!ptr_)
			{
				throw std::runtime_error("Dereferencing a nullptr\n");
			}
			return (*ptr_)[index];
		}
	}

	template<typename T, typename Deleter>
	inline T* SharedPtr<T, Deleter>::get() const
	{
		return ptr_;
	}

	template<typename T, typename Deleter>
	inline void SharedPtr<T, Deleter>::reset()
	{
		if (!refCount_)
		{
			ptr_ = nullptr;
			return;
		}

		if (--*refCount_ == 0)
		{
			Deleter{}(ptr_);
		}

		ptr_ = nullptr;
	}

	template<typename T, typename Deleter>
	inline void SharedPtr<T, Deleter>::reset(T* ptr)
	{
		reset();
		if (ptr)
		{
			ptr_ = ptr;
			refCount_ = new std::uint64_t(1);
		}
	}

	template<typename T, typename Deleter>
	inline std::uint64_t SharedPtr<T, Deleter>::refCount() const noexcept
	{
		return *refCount_;
	}

	// Non-member functions

	template<typename R, typename ... P>
	SharedPtr<R> makeShared(P&& ...args)
	{
		R* ptr;

		try 
		{
			ptr = new R(std::forward<P>(args)...);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Exception in makeShared: " << e.what() << std::endl;
			throw;
		}
		catch (...)
		{
			std::cerr << "Unknown exception in makeShared" << std::endl;
			throw;
		}

		return SharedPtr<R>(ptr);
	}

	template<typename T>
	bool operator==(const SharedPtr<T>& lhs, const SharedPtr<T>& rhs)
	{
		return lhs.get() == rhs.get();
	}

	template<typename T>
	bool operator!=(const SharedPtr<T>& lhs, const SharedPtr<T>& rhs)
	{
		return !(lhs == rhs);
	}

	template<typename T>
	bool operator>(const SharedPtr<T>& lhs, const SharedPtr<T>& rhs)
	{
		return lhs.get() > rhs.get();
	}

	template<typename T>
	bool operator>=(const SharedPtr<T>& lhs, const SharedPtr<T>& rhs)
	{
		return lhs.get() >= rhs.get();
	}

	template<typename T>
	bool operator<(const SharedPtr<T>& lhs, const SharedPtr<T>& rhs)
	{
		return lhs.get() < rhs.get();
	}

	template<typename T>
	bool operator<=(const SharedPtr<T>& lhs, const SharedPtr<T>& rhs)
	{
		return lhs.get() <= rhs.get();
	}

	// Partial template specialization for C-style arrays
	template <typename T>
	class SharedPtr<T[]>
	{
	private:
		T* ptr_ = nullptr;
		std::uint64_t* refCount_ = new std::uint64_t(0);
		std::size_t size_ = 0; // if 0 -> unknown size (allocated by new[])

	public:
		SharedPtr<T[]>(T* ptr, const std::size_t size) noexcept;
		explicit SharedPtr<T[]>(T* ptr) noexcept;
		~SharedPtr<T[]>();

		std::size_t size() const noexcept;
		T* get() const noexcept;
		void reset();
		void reset(T* ptr, const std::size_t size);
		void reset(T* ptr);

		T& operator[](const std::size_t index);
	};

	template<typename T>
	inline SharedPtr<T[]>::SharedPtr(T* ptr, const std::size_t size) noexcept 
		: ptr_(ptr), size_(size), refCount_(new std::uint64_t(1))
	{
		if (!ptr_)
		{
			throw std::bad_alloc();
		}
	}

	template<typename T>
	inline SharedPtr<T[]>::SharedPtr(T* ptr) noexcept
		: ptr_(ptr), size_(0), refCount_(new std::uint64_t(1)) {}

	template<typename T>
	SharedPtr<T[]>::~SharedPtr<T[]>()
	{
		reset();
	}

	template<typename T>
	inline std::size_t SharedPtr<T[]>::size() const noexcept
	{
		return size_;
	}

	template<typename T>
	inline T* SharedPtr<T[]>::get() const noexcept
	{
		return ptr_;
	}

	template<typename T>
	inline T& SharedPtr<T[]>::operator[](const std::size_t index)
	{
		if (!ptr_)
		{
			throw std::runtime_error("Dereferencing a nullptr\n");
		}
		if (size_ != 0 && index >= size_)
		{
			throw std::out_of_range("index");
		}
		return ptr_[index];
	}

	template<typename T>
	inline void SharedPtr<T[]>::reset()
	{
		if (!refCount_)
		{
			ptr_ = nullptr;
			size_ = 0;
			return;
		}

		if (--*refCount_ == 0)
		{
			if (size_ == 0)
			{
				delete[] ptr_;
			}
			else
			{
				for (std::size_t i = 0; i < size_; ++i)
				{
					std::destroy_at(ptr_ + i);
				}
				std::free(ptr_);
			}
		}

		ptr_ = nullptr;
		size_ = 0;
	}

	template<typename T>
	inline void SPTR::SharedPtr<T[]>::reset(T* ptr, const std::size_t size)
	{
		reset();
		if (ptr)
		{
			ptr_ = ptr;
			refCount_ = new std::uint64_t(1);
			size_ = size;
		}
	}

	template<typename T>
	inline void SPTR::SharedPtr<T[]>::reset(T* ptr)
	{
		reset();
		if (ptr)
		{
			ptr_ = ptr;
			refCount_ = new std::uint64_t(1);
			size_ = 0; // unknown size
		}
	}

	template<typename T>
	SharedPtr<T[]> makeShared(std::initializer_list<T> init)
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

		return SharedPtr<T[]>(ptr, elementsNumber);
	}
} // End of SPTR
