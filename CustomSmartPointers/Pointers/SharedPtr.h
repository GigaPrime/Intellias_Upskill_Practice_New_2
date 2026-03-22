#pragma once

#include "Helper.h"

namespace SPTR
{
    template <typename T>
    struct Deleter;

    template <typename T, typename Deleter = std::default_delete<T>>
    class SharedPtr final
    {
    private:
        ControlBlockBase* ctrl_ = nullptr;

        template <typename>
        friend class WeakPtr;

    public:
        SharedPtr() noexcept = default;
        explicit SharedPtr(std::nullptr_t) noexcept;
        explicit SharedPtr(T* ptr) noexcept;

        SharedPtr(const SharedPtr& other) noexcept;
        SharedPtr(SharedPtr&& other) noexcept;

        SharedPtr& operator=(const SharedPtr& other) noexcept;
        SharedPtr& operator=(SharedPtr&& other) noexcept;

        ~SharedPtr();

        T& operator*();
        T* operator->();
        T& operator[](std::size_t index);

        T* get() const;
        void reset();
        void reset(T* ptr);
        std::uint64_t refCount() const noexcept;
    };

    template<typename T, typename Deleter>
    inline SharedPtr<T, Deleter>::SharedPtr(std::nullptr_t) noexcept : ctrl_(nullptr) {}

    template<typename T, typename Deleter>
    inline SharedPtr<T, Deleter>::SharedPtr(T* ptr) noexcept
    {
        if (ptr)
        {
            auto* cb = new ControlBlock<T>();
            cb->ptr_ = ptr;
            cb->refCount_ = 1;
            cb->weakCount_ = 0;
            ctrl_ = cb;
        }
    }

    template<typename T, typename Deleter>
    inline SharedPtr<T, Deleter>::SharedPtr(const SharedPtr& other) noexcept : ctrl_(other.ctrl_)
    {
        if (ctrl_)
        {
            ++ctrl_->refCount_;
        }
    }

    template<typename T, typename Deleter>
    inline SharedPtr<T, Deleter>::SharedPtr(SharedPtr&& other) noexcept : ctrl_(other.ctrl_)
    {
        other.ctrl_ = nullptr;
    }

    template<typename T, typename Deleter>
    inline SharedPtr<T, Deleter>& SharedPtr<T, Deleter>::operator=(const SharedPtr& other) noexcept
    {
        if (this != &other)
        {
            reset();
            ctrl_ = other.ctrl_;
            if (ctrl_)
            {
                ++ctrl_->refCount_;
            }
        }
        return *this;
    }

    template<typename T, typename Deleter>
    inline SharedPtr<T, Deleter>& SharedPtr<T, Deleter>::operator=(SharedPtr&& other) noexcept
    {
        if (this != &other)
        {
            reset();
            ctrl_ = other.ctrl_;
            other.ctrl_ = nullptr;
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
        if (!ctrl_ || !static_cast<ControlBlock<T>*>(ctrl_)->ptr_)
        {
            throw std::runtime_error("Dereferencing a nullptr\n");
        }
        return *static_cast<ControlBlock<T>*>(ctrl_)->ptr_;
    }

    template<typename T, typename Deleter>
    inline T* SharedPtr<T, Deleter>::operator->()
    {
        if (!ctrl_ || !static_cast<ControlBlock<T>*>(ctrl_)->ptr_)
        {
            throw std::runtime_error("Dereferencing a nullptr\n");
        }
        return static_cast<ControlBlock<T>*>(ctrl_)->ptr_;
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
            if (!ctrl_ || !static_cast<ControlBlock<T>*>(ctrl_)->ptr_)
            {
                throw std::runtime_error("Dereferencing a nullptr\n");
            }
            return (*static_cast<ControlBlock<T>*>(ctrl_)->ptr_)[index];
        }
    }

    template<typename T, typename Deleter>
    inline T* SharedPtr<T, Deleter>::get() const
    {
        return ctrl_ ? static_cast<ControlBlock<T>*>(ctrl_)->ptr_ : nullptr;
    }

    template<typename T, typename Deleter>
    inline void SharedPtr<T, Deleter>::reset()
    {
        if (!ctrl_)
        {
            return;
        }

        auto* current = ctrl_;
        ctrl_ = nullptr;

        if (current->refCount_ > 0)
        {
            --current->refCount_;
        }

        if (current->refCount_ == 0)
        {
            auto* derived = static_cast<ControlBlock<T>*>(current);
            if (derived->ptr_)
            {
                Deleter{}(derived->ptr_);
                derived->ptr_ = nullptr;
            }

            if (current->weakCount_ == 0)
            {
                delete current;
            }
        }
    }

    template<typename T, typename Deleter>
    inline void SharedPtr<T, Deleter>::reset(T* ptr)
    {
        reset();
        if (ptr)
        {
            auto* cb = new ControlBlock<T>();
            cb->ptr_ = ptr;
            cb->refCount_ = 1;
            cb->weakCount_ = 0;
            ctrl_ = cb;
        }
    }

    template<typename T, typename Deleter>
    inline std::uint64_t SharedPtr<T, Deleter>::refCount() const noexcept
    {
        return ctrl_ ? ctrl_->refCount_ : 0;
    }

    template<typename R, typename... P>
    SharedPtr<R> makeShared(P&&... args)
    {
        return SharedPtr<R>(new R(std::forward<P>(args)...));
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

    template <typename T>
    class SharedPtr<T[]>
    {
    private:
        IndexableControlBlock<T>* iCtrl_ = nullptr;

    public:
        SharedPtr(T* ptr, std::size_t size) noexcept;
        explicit SharedPtr(T* ptr) noexcept;
        ~SharedPtr();

        std::size_t size() const noexcept;
        T* get() const noexcept;
        void reset();
        void reset(T* ptr, std::size_t size);
        void reset(T* ptr);

        T& operator[](std::size_t index);
    };

    template<typename T>
    inline SharedPtr<T[]>::SharedPtr(T* ptr, const std::size_t size) noexcept
    {
        if (ptr)
        {
            iCtrl_ = new IndexableControlBlock<T>();
            iCtrl_->ptr_ = ptr;
            iCtrl_->refCount_ = 1;
            iCtrl_->weakCount_ = 0;
            iCtrl_->size_ = size;
        }
    }

    template<typename T>
    inline SharedPtr<T[]>::SharedPtr(T* ptr) noexcept
    {
        if (ptr)
        {
            iCtrl_ = new IndexableControlBlock<T>();
            iCtrl_->ptr_ = ptr;
            iCtrl_->refCount_ = 1;
            iCtrl_->weakCount_ = 0;
            iCtrl_->size_ = 0;
        }
    }

    template<typename T>
    inline SharedPtr<T[]>::~SharedPtr()
    {
        reset();
    }

    template<typename T>
    inline std::size_t SharedPtr<T[]>::size() const noexcept
    {
        return iCtrl_ ? iCtrl_->size_ : 0;
    }

    template<typename T>
    inline T* SharedPtr<T[]>::get() const noexcept
    {
        return iCtrl_ ? iCtrl_->ptr_ : nullptr;
    }

    template<typename T>
    inline T& SharedPtr<T[]>::operator[](const std::size_t index)
    {
        if (!iCtrl_ || !iCtrl_->ptr_)
        {
            throw std::runtime_error("Dereferencing a nullptr\n");
        }
        if (iCtrl_->size_ != 0 && index >= iCtrl_->size_)
        {
            throw std::out_of_range("index");
        }
        return iCtrl_->ptr_[index];
    }

    template<typename T>
    inline void SharedPtr<T[]>::reset()
    {
        if (!iCtrl_)
        {
            return;
        }

        auto* current = iCtrl_;
        iCtrl_ = nullptr;

        if (current->refCount_ > 0)
        {
            --current->refCount_;
        }

        if (current->refCount_ == 0)
        {
            if (current->ptr_)
            {
                if (current->size_ == 0)
                {
                    delete[] current->ptr_;
                }
                else
                {
                    for (std::size_t i = 0; i < current->size_; ++i)
                    {
                        std::destroy_at(current->ptr_ + i);
                    }
                    std::free(current->ptr_);
                }
                current->ptr_ = nullptr;
            }

            if (current->weakCount_ == 0)
            {
                delete current;
            }
        }
    }

    template<typename T>
    inline void SharedPtr<T[]>::reset(T* ptr, const std::size_t size)
    {
        reset();
        if (ptr)
        {
            iCtrl_ = new IndexableControlBlock<T>();
            iCtrl_->ptr_ = ptr;
            iCtrl_->refCount_ = 1;
            iCtrl_->weakCount_ = 0;
            iCtrl_->size_ = size;
        }
    }

    template<typename T>
    inline void SharedPtr<T[]>::reset(T* ptr)
    {
        reset();
        if (ptr)
        {
            iCtrl_ = new IndexableControlBlock<T>();
            iCtrl_->ptr_ = ptr;
            iCtrl_->refCount_ = 1;
            iCtrl_->weakCount_ = 0;
            iCtrl_->size_ = 0;
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
} // namespace SPTR
