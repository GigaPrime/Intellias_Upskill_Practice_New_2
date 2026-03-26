#pragma once

#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

namespace VT
{
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
			static NullOptionalType type;
			return type;
		}
	};

	template <typename T>
	class Optional
	{
	private:
		alignas(T) std::byte storage[sizeof(T)];
		bool exists = false;

		template <typename>
		friend class Optional;

	private:
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

		//Optional<T>(Args&& ... args);

		Optional& operator=(const NullOptionalType& type);
		Optional& operator=(const Optional& other);
		Optional& operator=(Optional&& other);

		~Optional();
	};

	template <typename T>
	inline Optional<T>::Optional(const NullOptionalType&) noexcept
		: exists(false)
	{}

	template <typename T>
	inline Optional<T>::Optional(const Optional<T>& other)
	{
		if (other.exists)
		{
			new(&storage[0]) T(*reinterpret_cast<const T*>(&other.storage[0]));
			exists = true;
		}
	}

	template <typename T>
	inline Optional<T>::Optional(Optional&& other)
	{
		if (other.exists)
		{
			new(&storage[0]) T(std::move(*reinterpret_cast<T*>(&other.storage[0])));
			exists = true;
		}
	}

	template <typename T>
	template <typename U, typename>
	inline Optional<T>::Optional(const Optional<U>& other)
	{
		if (other.exists)
		{
			new(&storage[0]) T(*reinterpret_cast<const U*>(&other.storage[0]));
			exists = true;
		}
	}

	template <typename T>
	template <typename U, typename>
	inline Optional<T>::Optional(Optional<U>&& other)
	{
		if (other.exists)
		{
			new(&storage[0]) T(std::move(*reinterpret_cast<U*>(&other.storage[0])));
			exists = true;
		}
	}

	template <typename T>
	inline Optional<T>& Optional<T>::operator=(const NullOptionalType&)
	{
		if (exists)
		{
			reinterpret_cast<T*>(&storage[0])->~T();
			exists = false;
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

		if (exists && other.exists)
		{
			*reinterpret_cast<T*>(&storage[0]) =
				*reinterpret_cast<const T*>(&other.storage[0]);
		}
		else if (exists && !other.exists)
		{
			reinterpret_cast<T*>(&storage[0])->~T();
			exists = false;
		}
		else if (!exists && other.exists)
		{
			new(&storage[0]) T(*reinterpret_cast<const T*>(&other.storage[0]));
			exists = true;
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

		if (exists && other.exists)
		{
			*reinterpret_cast<T*>(&storage[0]) =
				std::move(*reinterpret_cast<T*>(&other.storage[0]));
		}
		else if (exists && !other.exists)
		{
			reinterpret_cast<T*>(&storage[0])->~T();
			exists = false;
		}
		else if (!exists && other.exists)
		{
			new(&storage[0]) T(std::move(*reinterpret_cast<T*>(&other.storage[0])));
			exists = true;
		}

		return *this;
	}

	template <typename T>
	inline Optional<T>::~Optional()
	{
		if (exists)
		{
			reinterpret_cast<T*>(&storage[0])->~T();
			exists = false;
		}
	}
}