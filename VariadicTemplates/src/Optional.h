#pragma once

#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

namespace VT
{
	// has_value
	// in-place construction

	class NullOptionalType
	{
	private:
		NullOptionalType() = default;

	public:
		NullOptionalType(const NullOptionalType& other) = delete;
		NullOptionalType(NullOptionalType&& other) = delete;
		NullOptionalType operator=(const NullOptionalType& other) = delete;
		NullOptionalType operator=(NullOptionalType&& other) = delete;
		~NullOptionalType() = default;

		static NullOptionalType& getNullOptionalType()
		{
			static NullOptionalType type_;
			return type_;
		}
	};

	template <typename T>
	class Optional
	{
	private:
		alignas(T) std::byte storage_[sizeof(T)];
		bool exists_ = false;

		template <typename>
		friend class Optional;

		template <typename U>
		struct IsOptional : std::false_type
		{};

		template <typename U>
		struct IsOptional<Optional<U>> : std::true_type
		{};

	public:
		Optional() noexcept = default;
		explicit Optional(const NullOptionalType& type) noexcept;

		Optional(const Optional& other);
		Optional(Optional&& other);

		template <typename U, typename
			= std::enable_if_t<std::is_constructible_v<T, const U&>>>
		Optional(const Optional<U>& other);

		template <typename U, typename
			= std::enable_if_t<std::is_constructible_v<T, U&&>>>
		Optional(Optional<U>&& other);

		template <
			typename... Args,
			typename = std::enable_if_t<std::is_constructible_v<T, Args&&...>>
		>
		Optional(Args&&... args);

		Optional& operator=(const NullOptionalType& type);
		Optional& operator=(const Optional& other);
		Optional& operator=(Optional&& other);

		~Optional();
	};

	template <typename T>
	inline Optional<T>::Optional(const NullOptionalType&) noexcept
		: exists_(false)
	{}

	template <typename T>
	inline Optional<T>::Optional(const Optional<T>& other)
	{
		if (other.exists_)
		{
			new(&storage_[0]) T(*reinterpret_cast<const T*>(&other.storage_[0]));
			exists_ = true;
		}
	}

	template <typename T>
	inline Optional<T>::Optional(Optional&& other)
	{
		if (other.exists_)
		{
			new(&storage_[0]) T(std::move(*reinterpret_cast<T*>(&other.storage_[0])));
			exists_ = true;
		}
	}

	template <typename T>
	template <typename U, typename>
	inline Optional<T>::Optional(const Optional<U>& other)
	{
		if (other.exists_)
		{
			new(&storage_[0]) T(*reinterpret_cast<const U*>(&other.storage_[0]));
			exists_ = true;
		}
	}

	template <typename T>
	template <typename U, typename>
	inline Optional<T>::Optional(Optional<U>&& other)
	{
		if (other.exists_)
		{
			new(&storage_[0]) T(std::move(*reinterpret_cast<U*>(&other.storage_[0])));
			exists_ = true;
		}
	}

	template<typename T>
	template<typename ...Args, typename>
	inline Optional<T>::Optional(Args && ...args)
	{
		new(&storage_[0]) T(std::forward<Args>(args)...);
		exists_ = true;
	}

	template <typename T>
	inline Optional<T>& Optional<T>::operator=(const NullOptionalType&)
	{
		if (exists_)
		{
			reinterpret_cast<T*>(&storage_[0])->~T();
			exists_ = false;
		}
		return *this;
	}

	template <typename T>
	inline Optional<T>& Optional<T>::operator=(const Optional& other)
	{
		if (this == &other)
		{
			return *this;
		}

		if (exists_ && other.exists_)
		{
			*reinterpret_cast<T*>(&storage_[0]) =
				*reinterpret_cast<const T*>(&other.storage_[0]);
		}
		else if (exists_ && !other.exists_)
		{
			reinterpret_cast<T*>(&storage_[0])->~T();
			exists_ = false;
		}
		else if (!exists_ && other.exists_)
		{
			new(&storage_[0]) T(*reinterpret_cast<const T*>(&other.storage_[0]));
			exists_ = true;
		}

		return *this;
	}

	template <typename T>
	inline Optional<T>& Optional<T>::operator=(Optional&& other)
	{
		if (this == &other)
		{
			return *this;
		}

		if (exists_ && other.exists_)
		{
			*reinterpret_cast<T*>(&storage_[0]) =
				std::move(*reinterpret_cast<T*>(&other.storage_[0]));
		}
		else if (exists_ && !other.exists_)
		{
			reinterpret_cast<T*>(&storage_[0])->~T();
			exists_ = false;
		}
		else if (!exists_ && other.exists_)
		{
			new(&storage_[0]) T(std::move(*reinterpret_cast<T*>(&other.storage_[0])));
			exists_ = true;
		}

		return *this;
	}

	template <typename T>
	inline Optional<T>::~Optional()
	{
		if (exists_)
		{
			reinterpret_cast<T*>(&storage_[0])->~T();
			exists_ = false;
		}
	}
}