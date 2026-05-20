#pragma once

#include "Helper.h"
#include "SharedPtr.h"

namespace SPTR
{
    template <typename T>
    class WeakPtr final
    {
    private:
        ControlBlockBase* ctrl_ = nullptr;

        template<typename>
        friend class WeakPtr;

    public:
        WeakPtr() noexcept = default;
        explicit WeakPtr(std::nullptr_t) noexcept;
        explicit WeakPtr(const WeakPtr<T>& other) noexcept;

        template <typename U>
        explicit WeakPtr(const WeakPtr<U>& other) noexcept;

        explicit WeakPtr(const SharedPtr<T>& otherShared) noexcept;

        template <typename U>
        explicit WeakPtr(const SharedPtr<U>& otherShared) noexcept;

        explicit WeakPtr(WeakPtr&& other) noexcept;

        template <typename U>
        explicit WeakPtr(WeakPtr<U>&& other) noexcept;

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

        SharedPtr<T> lock() noexcept;
        bool expired() noexcept;
        std::size_t useCount() const noexcept;
        void reset() noexcept;
    };

    template<typename T>
    inline WeakPtr<T>::WeakPtr(std::nullptr_t) noexcept : ctrl_(nullptr) {}

    template<typename T>
    inline WeakPtr<T>::WeakPtr(const WeakPtr<T>& other) noexcept : ctrl_(other.ctrl_)
    {
        if (ctrl_)
        {
            ++ctrl_->weakCount_;
        }
    }

    template<typename T>
    template<typename U>
    inline WeakPtr<T>::WeakPtr(const WeakPtr<U>& other) noexcept
    {
        if constexpr (std::is_convertible_v<U*, T*>)
        {
            ctrl_ = other.ctrl_;
            if (ctrl_)
            {
                ++ctrl_->weakCount_;
            }
        }
    }

    template<typename T>
    inline WeakPtr<T>::WeakPtr(const SharedPtr<T>& otherShared) noexcept : ctrl_(otherShared.ctrl_)
    {
        if (ctrl_)
        {
            ++ctrl_->weakCount_;
        }
    }

    template<typename T>
    template<typename U>
    inline WeakPtr<T>::WeakPtr(const SharedPtr<U>& otherShared) noexcept
    {
        if constexpr (std::is_convertible_v<U*, T*>)
        {
            ctrl_ = otherShared.ctrl_;
            if (ctrl_)
            {
                ++ctrl_->weakCount_;
            }
        }
    }

    template<typename T>
    inline WeakPtr<T>::WeakPtr(WeakPtr<T>&& other) noexcept : ctrl_(other.ctrl_)
    {
        other.ctrl_ = nullptr;
    }

    template<typename T>
    template<typename U>
    inline WeakPtr<T>::WeakPtr(WeakPtr<U>&& other) noexcept
    {
        if constexpr (std::is_convertible_v<U*, T*>)
        {
            ctrl_ = other.ctrl_;
            other.ctrl_ = nullptr;
        }
    }

    template<typename T>
    inline WeakPtr<T>& WeakPtr<T>::operator=(const SharedPtr<T>& otherShared) noexcept
    {
        if (ctrl_ != otherShared.ctrl_)
        {
            reset();
            ctrl_ = otherShared.ctrl_;
            if (ctrl_)
            {
                ++ctrl_->weakCount_;
            }
        }
        return *this;
    }

    template<typename T>
    template<typename U>
    inline WeakPtr<T>& WeakPtr<T>::operator=(const SharedPtr<U>& otherShared) noexcept
    {
        if constexpr (std::is_convertible_v<U*, T*>)
        {
            if (ctrl_ != otherShared.ctrl_)
            {
                reset();
                ctrl_ = otherShared.ctrl_;
                if (ctrl_)
                {
                    ++ctrl_->weakCount_;
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
            ctrl_ = other.ctrl_;
            if (ctrl_)
            {
                ++ctrl_->weakCount_;
            }
        }
        return *this;
    }

    template<typename T>
    template<typename U>
    inline WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr<U>& other) noexcept
    {
        if constexpr (std::is_convertible_v<U*, T*>)
        {
            if (ctrl_ != other.ctrl_)
            {
                reset();
                ctrl_ = other.ctrl_;
                if (ctrl_)
                {
                    ++ctrl_->weakCount_;
                }
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
            ctrl_ = other.ctrl_;
            other.ctrl_ = nullptr;
        }
        return *this;
    }

    template<typename T>
    template<typename U>
    inline WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr<U>&& other) noexcept
    {
        if constexpr (std::is_convertible_v<U*, T*>)
        {
            if (ctrl_ != other.ctrl_)
            {
                reset();
                ctrl_ = other.ctrl_;
                other.ctrl_ = nullptr;
            }
        }
        return *this;
    }

    template<typename T>
    inline WeakPtr<T>::~WeakPtr()
    {
        reset();
    }

    template<typename T>
    inline SharedPtr<T> WeakPtr<T>::lock() noexcept
    {
        SharedPtr<T> result;

        if (!ctrl_)
        {
            auto* cb = new ControlBlock<T>();
            cb->ptr_ = nullptr;
            cb->refCount_ = 1;
            cb->weakCount_ = 0;
            result.ctrl_ = cb;
            return result;
        }

        if (ctrl_->refCount_ == 0)
        {
            return result;
        }

        result.ctrl_ = ctrl_;
        ++ctrl_->refCount_;
        return result;
    }

    template<typename T>
    inline bool WeakPtr<T>::expired() noexcept
    {
        return !ctrl_ || ctrl_->refCount_ == 0;
    }

    template<typename T>
    inline std::size_t WeakPtr<T>::useCount() const noexcept
    {
        return ctrl_ ? ctrl_->refCount_ : 0;
    }

    template<typename T>
    inline void WeakPtr<T>::reset() noexcept
    {
        if (!ctrl_)
        {
            return;
        }

        auto* current = ctrl_;
        ctrl_ = nullptr;

        if (current->weakCount_ > 0)
        {
            --current->weakCount_;
        }

        if (current->weakCount_ == 0 && current->refCount_ == 0)
        {
            delete current;
        }
    }
} // namespace SPTR
