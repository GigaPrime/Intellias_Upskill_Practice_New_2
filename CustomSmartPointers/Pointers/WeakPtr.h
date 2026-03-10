#pragma once
#pragma once

#include "Helper.h"
#include "SharedPtr.h"

namespace SPTR
{
	template <typename T>
	class WeakPtr final
	{
	private:
		T* ptr_ = nullptr;
		std::uint64_t* refCount_ = nullptr;
		std::uint64_t* weakCount_ = nullptr;

	public:
		WeakPtr() noexcept = default;
		explicit WeakPtr(std::nullptr_t) noexcept;
		explicit WeakPtr(const WeakPtr<T>& other) noexcept;

		template <typename U>
		explicit WeakPtr(const WeakPtr<U>& other) noexcept;

		template <typename U>
		WeakPtr(const SharedPtr<U>& otherShared) noexcept;

		WeakPtr(WeakPtr&& other) noexcept;

		template <typename U>
		WeakPtr(WeakPtr<U>&& other) noexcept;

		WeakPtr& operator=(const SharedPtr<T>& otherShared) noexcept;

		template <typename U>
		WeakPtr& operator=(const SharedPtr<U>& otherShared) noexcept;

		WeakPtr& operator=(const WeakPtr<T>& other) noexcept;

		template <typename U>
		WeakPtr& operator=(const WeakPtr<U>& other) noexcept;

		WeakPtr& operator=(WeakPtr<T>&& other) noexcept;

		template <typename U>
		WeakPtr& operator=(WeakPtr<U>&& other) noexcept;

		~WeakPtr();

		SharedPtr<T> lock() const noexcept;
		bool expired() const noexcept;
		std::size_t useCount() const noexcept;
		void reset() noexcept;
	};

	template<typename T>
	inline WeakPtr<T>::WeakPtr(std::nullptr_t) noexcept
		: ptr_(nullptr), refCount_(new std::uint64_t(0)), weakCount_(new std::uint64_t(0)) {
	}

	template<typename T>
	inline WeakPtr<T>::WeakPtr(const WeakPtr<T>& other) noexcept
	{
		if (this != &other)
		{
			ptr_ = other.ptr_;
			refCount_ = other.refCount_;
			weakCount_ = other.weakCount_;
			if (ptr_)
			{
				++*weakCount_;
			}
		}
	}

	template<typename T>
	template<typename U>
	inline WeakPtr<T>::WeakPtr(const SharedPtr<U>& otherShared) noexcept
	{
		if constexpr (std::is_convertible_v<U, T>)
		{
			if (ptr_ != otherShared.get())
			{
				ptr_ = otherShared.get();
				refCount_ = otherShared.refCountPtr();
				weakCount_ = new std::uint64_t(0);
				if (ptr_)
				{
					++*weakCount_;
				}
			}
		}
	}

	template<typename T>
	inline WeakPtr<T>::WeakPtr(WeakPtr<T>&& other) noexcept
	{
		if (this != &other)
		{
			ptr_ = other.ptr_;
			refCount_ = other.refCount_;
			weakCount_ = other.weakCount_;
			other.ptr_ = nullptr;
			other.refCount_ = nullptr;
			other.weakCount_ = nullptr;
		}
	}

	template<typename T>
	inline WeakPtr<T>& WeakPtr<T>::operator=(const SharedPtr<T>& otherShared) noexcept
	{
		if (ptr_ != otherShared.get())
		{
			reset();
			ptr_ = otherShared.get();
			refCount_ = otherShared.refCountPtr();
			weakCount_ = new std::uint64_t(1);
			if (ptr_)
			{
				++*weakCount_;
			}
		}
		return *this;
	}

	template<typename T>
	template<typename U>
	inline WeakPtr<T>& WeakPtr<T>::operator=(const SharedPtr<U>& otherShared) noexcept
	{
		if constexpr (std::is_convertible_v<U, T>)
		{
			if (ptr_ != otherShared.get())
			{
				reset();
				ptr_ = otherShared.get();
				refCount_ = otherShared.refCountPtr();
				weakCount_ = new std::uint64_t(1);
				if (ptr_)
				{
					++*weakCount_;
				}
			}
		}
		return *this;
	}

	template<typename T>
	inline WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr<T>& other) noexcept
	{
		if (this != &other)
		{
			reset();
			ptr_ = other.ptr_;
			refCount_ = other.refCount_;
			weakCount_ = other.weakCount_;
			if (ptr_)
			{
				++*weakCount_;
			}
		}
		return *this;
	}

	template<typename T>
	inline WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr<T>&& other) noexcept
	{
		if (this != &other)
		{
			reset();
			ptr_ = other.ptr_;
			refCount_ = other.refCount_;
			weakCount_ = other.weakCount_;
			other.ptr_ = nullptr;
			other.refCount_ = nullptr;
			other.weakCount_ = nullptr;
		}
		return *this;
	}

	template<typename T>
	inline WeakPtr<T>::~WeakPtr()
	{
		if (--*refCount_ == 0)
		{
			delete refCount_;
			refCount_ = nullptr;
		}
	}

	template<typename T>
	inline SharedPtr<T> SPTR::WeakPtr<T>::lock() const noexcept
	{
		if (!expired())
		{
			return SharedPtr<T>(ptr_);
		}
	}

	template<typename T>
	inline bool SPTR::WeakPtr<T>::expired() const noexcept
	{
		return *refCount_ == 0;
	}

	template<typename T>
	inline std::size_t SPTR::WeakPtr<T>::useCount() const noexcept
	{
		return *refCount_;
	}

	template<typename T>
	inline void SPTR::WeakPtr<T>::reset() noexcept
	{
		if (--*weakCount_ == 0)
		{
			delete weakCount_;
			weakCount_ = nullptr;
		}
		refCount_ = nullptr;
		ptr_ = nullptr;
	}

} // end of WPTR

// Countrol block
// reset()
