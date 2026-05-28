#include <cerrno>
#include <memory>
#include <typeinfo>
#include <type_traits>
#include <utility>

namespace VT 
{
	struct BaseHolder 
	{
		virtual std::unique_ptr<BaseHolder> clone() const = 0;
		virtual ~BaseHolder() = default;

		virtual const std::type_info& type() const = 0;
	};

	template <typename T>
	struct ExactHolder : public BaseHolder
	{
		T value_;

		ExactHolder(const T& value) : value_(value) {}
		ExactHolder(T&& value) : value_(std::move(value)) {}
		
		template <typename... Args>
		ExactHolder(Args&&... args) : value_(std::forward<Args>(args)...) {}
		
		std::unique_ptr<BaseHolder> clone() const override;

		const std::type_info& type() const override;
	};

	// Despite being a traits component, the Any class is designed to be a type-erased container that can hold any copy-constructible type.
	// To preserve non-templated implementation, the type-erasure is achieved with the help of a polymorphic base class (BaseHolder)
	class Any
	{
	private:
		std::unique_ptr<BaseHolder> holder_ = nullptr;

		template <typename T>
		friend T anyCast(const Any& operand);

		template <typename T>
		friend T anyCast(Any& operand);

		template <typename T>
		friend T anyCast(Any&& operand);

		template <typename T, typename>
		friend const T* anyCast(const Any* operand) noexcept;

		template <typename T, typename>
		friend T* anyCast(Any* operand) noexcept;

	public:
		Any() = default;
		Any(const Any& other);
		Any(Any&& other) noexcept;

		~Any();

		template<typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Any>>>
		inline Any(T&& value);

		template <typename T, typename... Args, 
			typename = std::enable_if_t<std::is_constructible_v<T, Args...>&& std::is_copy_constructible_v<T>>>
		explicit Any(std::in_place_type_t<T>, Args&&... args);

		Any& operator=(const Any& other);
		Any& operator=(Any&& other) noexcept;

		bool hasValue() const noexcept;
		const std::type_info& type() const;

		void reset() noexcept;
		void swap(Any& other) noexcept;

		template<typename T, typename... Args>
		std::decay_t<T>emplace(Args&&... args);
	};

	Any::Any(const Any& other)
	{
		holder_ = other.holder_ ? other.holder_->clone() : nullptr;
	}

	inline Any::Any(Any&& other) noexcept
	{
		holder_ = std::move(other.holder_);
	}

	inline Any::~Any()
	{
		reset();
	}

	inline Any& Any::operator=(const Any& other)
	{
		if (&other != this)
		{
			std::unique_ptr<BaseHolder> tmpHolder = other.holder_ ? other.holder_->clone() : nullptr;
			holder_ = std::move(tmpHolder);
		}
		return *this;
	}

	inline Any& Any::operator=(Any&& other) noexcept
	{
		if (&other != this)
		{
			holder_ = std::move(other.holder_);
		}
		return *this;
	}

	// This constructor is taken directly from std::any contract
	//
	// Decay the type to remove references and cv-qualifiers 
	// since the T&& is a forwarding reference and can be instantiated with 
	// both lvalue and rvalue references.
	// In other case it could lead to occasional creation of the reference wrapper 
	// which is not the intended behaviour of this constructor
	//
	// The std::enable_if_t is used to prevent this constructor from being instantiated when T is Any itself,
	// where
	// std::enable_if_t is responsible for excluding the constructor from overload resolution when T is Any (SFINAE implementation)
	// std::is_same_v is used to check if the decayed type of T is the same as Any, and if it is, the constructor will not be enabled.
	// std::decay removes constness and reference qualifiaers
	template<typename T, typename>
	inline Any::Any(T&& value)
	{
		using storedType = std::decay_t<T>;

		// Constructor should fail at compile time if the type is not copy constructible
		static_assert(std::is_copy_constructible<storedType>::value, 
			"Type is not copy constructible, cannot be stored in Any");
		holder_ = std::make_unique<ExactHolder<storedType>>(std::forward<T>(value));
	}

	template<typename T, typename ...Args, typename>
	inline Any::Any(std::in_place_type_t<T>, Args && ...args)
	{
		holder_ = std::make_unique<ExactHolder<T>>(std::forward<Args>(args)...);
	}

	template<typename T, typename ...Args>
	inline std::decay_t<T> Any::emplace(Args&& ...args)
	{
		reset();

		using storedType = std::decay_t<T>;
		static_assert(std::is_copy_constructible<storedType>::value,
			"Type is not copy constructible, cannot be stored in Any");
		holder_ = std::make_unique<ExactHolder<storedType>>(std::forward<Args>(args)...);
		return static_cast<ExactHolder<storedType>*>(holder_.get())->value_;
	}

	inline bool Any::hasValue() const noexcept
	{
		return holder_ != nullptr;
	}

	inline const std::type_info& Any::type() const
	{
		// For an empty Any
		if (!holder_)
		{
			return typeid(void);
		}
		return holder_->type();
	}

	inline void Any::reset() noexcept
	{
		holder_.reset();
	}

	inline void Any::swap(Any& other) noexcept
	{
		std::swap(holder_, other.holder_);
	}
	
	template<typename T>
	inline std::unique_ptr<BaseHolder> ExactHolder<T>::clone() const
	{		
		static_assert(std::is_copy_constructible<T>::value,
			"Type is not copy constructible, cannot be cloned in ExactHolder");

		// std::make_unique throws std::bad_alloc if memory allocation fails
		return std::make_unique<ExactHolder<T>>(value_);
	}

	// The type function is used to retrieve the type information of the stored value
	// std::type_info has deleted c-tor/copy c-tor so acts like a default type descriptor owned by the STL itself
	// that's why no object could be created and only reference to it could be returned
	// Since it does not "belong" to the ExactHolder, the const reference should be returned
	template<typename T>
	inline const std::type_info& ExactHolder<T>::type() const
	{
		{
			return typeid(T);
		}
	}

	// The anyCast functions are used to retrieve the type of the stored value and return it to the caller. 
	// If the type does not match, a std::bad_any_cast exception is thrown.
	// In fact it does not cast anything, but rather checks the type and returns the value if the type matches.
	template<typename T>
	T anyCast(const Any& operand)
	{
		// removing any potential reference and cv-qualifiers from T to get the actual type
		// Happens at compile time
		using returnType = std::decay_t<T>;

		// Check if the type of the stored value matches the requested type
		// Happens at runtime (it is impossible to use std::is_same_v since it is a compile time operation)
		if(operand.type() == typeid(returnType))
		{
			// Casting the holder with dereferencing it to the ExactHolder of the requested type
			return static_cast<const ExactHolder<returnType>*>(operand.holder_.get())->value_;
		}
		// If the type does not match, throw an exception
		throw std::bad_any_cast();
	}

	template<typename T>
	T anyCast(Any& operand)
	{
		// I wonder i it is worth calling the const version of anyCast to avoid code duplication, 
		// but it would require some const_cast which is not the best practice
		using returnType = std::decay_t<T>;
		if (operand.type() == typeid(returnType))
		{
			return static_cast<ExactHolder<returnType>*>(operand.holder_.get())->value_;
		}
		throw std::bad_any_cast();
	}

	template<typename T>
	T anyCast(Any&& operand)
	{
		using returnType = std::decay_t<T>;
		if (operand.type() == typeid(returnType))
		{
			// move the stored value out of the ExactHolder
			ExactHolder<returnType>* h = static_cast<ExactHolder<returnType>*>(operand.holder_.get());
			return std::move(h->value_);
		}	
		throw std::bad_any_cast();
	}

	// Pointer to reference is not allowed
	// std::decay removes constness if any
	template<typename T, typename = std::enable_if_t<!std::is_reference_v<T>>>
	const T* anyCast(const Any* operand) noexcept
	{
		using returnType = std::decay_t<T>;
		if (operand && operand->type() == typeid(returnType))
		{
			return &static_cast<const ExactHolder<returnType>*>(operand->holder_.get())->value_;
		}
		return nullptr;
	}

	template<typename T, typename = std::enable_if_t<!std::is_reference_v<T>>>
	T* anyCast(Any* operand) noexcept
	{
		using returnType = std::decay_t<T>;
		if (operand && operand->type() == typeid(returnType))
		{
			return &static_cast<ExactHolder<returnType>*>(operand->holder_.get())->value_;
		}
		return nullptr;
	}
} // namespace VT
