#pragma once

namespace SPTR
{
	template <typename T>
	class IPtr
	{
	public:
		virtual ~IPtr() = 0;

		virtual T& operator*() const = 0;
		virtual T* operator->() const = 0;
		
		//virtual T& operator[](const std::size_t index) const = 0;
		
		virtual T* get() const = 0;

		virtual void reset() = 0;
	};

	template <typename T> IPtr<T>::~IPtr() = default;

	template <typename T> void operator++(IPtr<T>& ptr);
	template <typename T> void operator+(IPtr<T>& ptr, const int shift);
	template <typename T> void operator--(IPtr<T>& ptr);
	template <typename T> void operator-(IPtr<T>& ptr, const int shift);

} // end of SPTR
