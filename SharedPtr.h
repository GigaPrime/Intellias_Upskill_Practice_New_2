#pragma once

#include <memory>
#include <type_traits>

namespace SPTR
{
	template <typename T>
	struct Deleter;

	template <typename T, typename Deleter = std::default_delete<T>>
	class SharedPtr final
	{
	private:
		T* ptr_ = nullptr;
		std::uint64_t refCount_ = 0;

		void copy(SharedPtr otherPtr);
		void move(SharedPtr&& otherPtr);

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
	};

	template<typename T, typename Deleter>
	inline void SPTR::SharedPtr<T, Deleter>::copy(SharedPtr other)
	{
		if (this != other)
		{
			ptr_ = other.ptr_;
			refCount_ = other.refCount_;
			if (ptr_)
			{
				++refCount_;
			}
		}
	}

	template<typename T, typename Deleter>
	inline void SharedPtr<T, Deleter>::move(SharedPtr&& other) noexcept
	{
		if (this != other)
		{
			ptr_ = other.ptr_;
			refCount_ = other.refCount_;
			other.ptr_ = nullptr;
			other.refCount_ = 0;
		}
	}

	template<typename T, typename Deleter>
	inline SharedPtr<T, Deleter>::SharedPtr(std::nullptr_t) noexcept : ptr_(nullptr), refCount_(0) {}

	template<typename T, typename Deleter>
	inline SharedPtr<T, Deleter>::SharedPtr(T* ptr) noexcept : ptr_(ptr), refCount_(ptr ? 1 : 0) {}

	template<typename T, typename Deleter>
	inline SharedPtr<T, Deleter>::SharedPtr(const SharedPtr& other) noexcept
	{
		copy(other);
	}

	template<typename T, typename Deleter>
	inline SharedPtr<T, Deleter>::SharedPtr(SharedPtr&& other) noexcept
	{
		move(other);
	}

	template<typename T, typename Deleter>
	inline SharedPtr<T, Deleter>& SharedPtr<T, Deleter>::operator=(const SharedPtr& other) noexcept
	{
		copy(other);
		return *this;
	}

	template<typename T, typename Deleter>
	inline SharedPtr<T, Deleter>& SharedPtr<T, Deleter>::operator=(SharedPtr<T, Deleter>&& other) noexcept
	{
		move(other);
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
		if constexpr (!std::is_array(T))
		{
			throw std::runtime_error("Type provided is not indexable\n");
		}
		else
		{
			if (!ptr_)
			{
				throw std::runtime_error("Dereferencing a nullptr\n");
			}
		}
		return (*ptr_)[index];
	}

	template<typename T, typename Deleter>
	inline T* SharedPtr<T, Deleter>::get() const
	{
		return ptr_;
	}

	template<typename T, typename Deleter>
	inline void SharedPtr<T, Deleter>::reset()
	{
		if (ptr_ && --refCount_ == 0)
		{
			Deleter{}(ptr_);
			ptr_ = nullptr;
		}
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
		catch (...)
		{
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



} // End of SPTR